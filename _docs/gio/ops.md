# Operations (op, op/clip, op/paint)

Package: [gioui.org/op](../../repos-folder/gio/op/), [op/clip](../../repos-folder/gio/op/clip/), [op/paint](../../repos-folder/gio/op/paint/)

Gio's drawing model is "build a list of ops, then submit". Op lists are pure data — no garbage, no allocation when reset between frames.

## `op.Ops`

```go
type Ops struct{ Internal ops.Ops }   // append-only buffer of serialized ops

ops := new(op.Ops)
ops.Reset()                            // wipe and re-use across frames
```

`Reset` invalidates any recorded macros.

## State model

Per-Ops state:

- **Color / image / gradient** — set directly by `paint.ColorOp{}.Add(ops)`, `paint.ImageOp{}.Add(ops)`, etc.
- **Transformation stack** — `op.Offset(...)` / `op.Affine(...)`, push with `.Push(ops)`, undo with `.Pop()`.
- **Clip stack** — `clip.RRect{}.Push(ops)`, undo with `.Pop()`.
- **Macros** — `op.Record(ops)` / `m.Stop()` produces a `CallOp` that can be replayed later via `call.Add(ops)`.
- **Defer** — `op.Defer(ops, call)` runs a recording after all queued ops finish (FIFO, opposite of Go's `defer`).
- **InvalidateCmd** — `op.InvalidateCmd{At: t}.Add(ops)` schedules a redraw.

## Transformations

```go
stack := op.Offset(image.Pt(10, 10)).Push(ops)
// ... ops drawn here are translated by (10, 10) ...
stack.Pop()
```

`op.Affine(f32.AffineId().Scale(...).Rotate(...))` gives the full affine pipeline (in `gioui.org/f32`).

## Macros

```go
ops := new(op.Ops)
macro := op.Record(ops)
paint.ColorOp{Color: red}.Add(ops)
paint.PaintOp{}.Add(ops)
call := macro.Stop()

// elsewhere:
call.Add(ops)   // replay
```

## `op/paint`

Fills the current clip with a color, an image, or a gradient.

```go
paint.ColorOp{Color: color.NRGBA{R: 255, A: 255}}.Add(ops)
paint.PaintOp{}.Add(ops)                  // actually fills

// Helper: combine clip + fill
paint.FillShape(ops, color.NRGBA{...}, clip.RRect{Rect: r, SE: 4, SW: 4, NW: 4, NE: 4}.Op(ops))

// Helper: fill the whole current clip
paint.Fill(ops, color.NRGBA{...})

// Image
img := paint.NewImageOp(myImage)
img.Filter = paint.FilterLinear
img.Add(ops)
paint.PaintOp{}.Add(ops)

// Linear gradient
paint.LinearGradientOp{
    Stop1: f32.Pt(0,0), Color1: color.NRGBA{...},
    Stop2: f32.Pt(0,100), Color2: color.NRGBA{...},
}.Add(ops)

// Opacity over a sub-tree
op := paint.PushOpacity(ops, 0.5)
// ... children ...
op.Pop()
```

`ImageFilter` constants: `FilterLinear` (default), `FilterNearest`.

## `op/clip`

Defines the current clip region. Push a shape, draw, pop.

```go
defer clip.RRect{Rect: image.Rectangle{Max: size}, SE: 8, SW: 8, NW: 8, NE: 8}.Push(ops).Pop()
paint.Fill(ops, color.NRGBA{R: 255, A: 255})
```

Built-in shapes:

- `clip.Rect(image.Rectangle{...})` — `.Op()`, `.Push(ops)`, `.Path()`
- `clip.RRect{Rect, SE, SW, NW, NE}` — rounded rectangle with per-corner radii
- `clip.Ellipse(image.Rectangle{...})` — ellipse inscribed in rect
- `clip.UniformRRect(rect, radius)` — convenience for equal-radius round rects
- `clip.Outline{Path: spec, Width: 2}` — outline (stroke) of a path
- `clip.Stroke{Path: spec, Width: 2}` — stroke variant

### Building paths

```go
var p clip.Path
p.Begin(ops)
p.MoveTo(f32.Pt(0, 0))
p.LineTo(f32.Pt(100, 0))
p.QuadTo(f32.Pt(150, 50), f32.Pt(100, 100))
p.CubeTo(f32.Pt(80, 120), f32.Pt(20, 120), f32.Pt(0, 100))
p.ArcTo(f1, f2, angle)
p.Close()
spec := p.End()

clip.Outline{Path: spec, Width: 2}.Op().Push(ops).Pop()
```

`PathSpec` is reusable across frames. `Op` (the unified clip op) is what gets pushed onto the clip stack.

## Stacks and lifetimes

`Push(ops)` returns a stack handle; you must call `.Pop()` (or `defer stack.Pop()`) before the surrounding op list is submitted. Imbalanced stacks panic at submit time.

The state at the top of the stack is what affects subsequent ops. Transforms and clips compose as you'd expect.

## The frame pipeline

```
              ┌──────────────┐    ┌──────────┐    ┌────────────┐
   app code ─►│ build Ops    │───►│ Frame()  │───►│ gpu render │
              └──────────────┘    └──────────┘    └────────────┘
```

Once `e.Frame(ops)` is called, the GPU backend ([gpu/](../../repos-folder/gio/gpu/), with per-API backends under `gpu/internal/`) consumes the serialized stream, rasterizes paths, schedules drawcalls and (where applicable) submits a command buffer.
