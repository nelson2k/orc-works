# Layout

Package: [gioui.org/layout](../../repos-folder/gio/layout/)

A `layout.Widget` is `func(gtx Context) Dimensions`. Anything that produces dimensions from constraints is a widget — including stateful UI controls *and* containers like `Flex` and `Stack`.

## Core types

```go
type Constraints struct{ Min, Max image.Point }

type Dimensions struct {
    Size     image.Point
    Baseline int        // distance from bottom to text baseline
}

type Context struct {
    Constraints Constraints
    Metric      unit.Metric
    Queue       event.Queue
    Now         time.Time
    Locale      system.Locale
    Source      input.Source
    Ops         *op.Ops
    ...
}
```

`Context` carries the current constraints, a pointer to the ops list, the per-frame event queue, the metric (dp/px ratio), and the locale.

Convenience:

- `Exact(size)` — Constraints with `Min == Max`.
- `c.Constrain(size)` — clamp a size into a Constraints.
- `c.AddMin(delta)` / `c.SubMax(delta)` — shrink/grow the box.
- `gtx.Dp(v)` / `gtx.Sp(v)` — convert dp/sp into pixels.
- `gtx.Disabled()` — derived context that disables input.

## Enums

```go
type Axis uint8       // Horizontal, Vertical
type Alignment uint8  // Start, End, Middle, Baseline
type Direction uint8  // NW, N, NE, E, SE, S, SW, W, Center
type Spacing uint8    // SpaceEnd, SpaceStart, SpaceSides, SpaceAround, SpaceBetween, SpaceEvenly
```

## Inset

```go
layout.Inset{Top: 8, Bottom: 8, Left: 16, Right: 16}.Layout(gtx, widget)
layout.UniformInset(8).Layout(gtx, widget)
```

`Inset` carries `Top/Bottom/Left/Right` as `unit.Dp`. The wrapped widget receives constraints reduced by the inset.

## Flex

Box layout, like CSS flexbox.

```go
layout.Flex{
    Axis:      layout.Horizontal,    // or Vertical
    Alignment: layout.Middle,         // cross-axis alignment
    Spacing:   layout.SpaceBetween,   // main-axis distribution
}.Layout(gtx,
    layout.Rigid(widgetA),                  // takes the size it needs
    layout.Flexed(1.0, widgetB),            // shares remaining space by weight
    layout.Rigid(layout.Spacer{Width: 8}.Layout),
)
```

- `Rigid(w)` — child decides its own main-axis size.
- `Flexed(weight, w)` — child shares the leftover space.
- `Spacing` controls how unused main-axis space is distributed: `SpaceStart`, `SpaceEnd`, `SpaceSides`, `SpaceAround`, `SpaceBetween`, `SpaceEvenly`.

## Stack

Z-stacked overlay.

```go
layout.Stack{Alignment: layout.Center}.Layout(gtx,
    layout.Expanded(background),  // sized to the stack's max constraints
    layout.Stacked(child1),       // sized to its own dimensions
    layout.Stacked(child2),
)
```

`Expanded` children get the full available size; `Stacked` children get their natural size. The stack's own size is `max(child sizes)`.

`layout.Background{}.Layout(gtx, background, foreground)` is sugar for "draw `background` sized to fit `foreground`'s dimensions".

## List (scrollable virtual list)

`List` is stateful — keep one per scrollable region:

```go
var list layout.List
list.Axis = layout.Vertical

list.Layout(gtx, len(items), func(gtx layout.Context, i int) layout.Dimensions {
    return renderItem(gtx, items[i])
})
```

Methods:

- `list.ScrollBy(n float32)` — programmatic scroll
- `list.ScrollTo(index int)` — jump to item
- `list.Position` (read) — current scroll position (offsets in dp, indices visible)
- `list.Dragging()` — is the user dragging the list right now

The list materializes only the visible cells (virtual scrolling). Push/pop dragging integrates with `gesture.Drag` automatically.

## Spacer and Direction

```go
layout.Spacer{Width: 16}.Layout(gtx)            // fixed empty space
layout.Center.Layout(gtx, w)                    // center in current constraints
layout.SE.Layout(gtx, w)                        // align to south-east
```

`Direction.Layout(gtx, w)` calls `w` with the current constraints and then positions the result inside the stack.

## Constraints discipline

Widgets are not required to *honor* their constraints — they can return a larger or smaller size. Parents are expected to deal with overflow (often by clipping). The `Constraints.Constrain` helper exists for widgets that want to clamp themselves.

`Rigid` children of a `Flex` get `Min = 0` on the main axis; `Flexed` children get a fixed main-axis size (`Min == Max`). Both share the full cross axis.

## Recipe: scrollable list of cards

```go
var list layout.List
list.Axis = layout.Vertical

list.Layout(gtx, len(items), func(gtx layout.Context, i int) layout.Dimensions {
    return layout.UniformInset(8).Layout(gtx, func(gtx layout.Context) layout.Dimensions {
        return layout.Background{}.Layout(gtx,
            func(gtx layout.Context) layout.Dimensions {
                rr := clip.UniformRRect(image.Rectangle{Max: gtx.Constraints.Min}, 6)
                paint.FillShape(gtx.Ops, color.NRGBA{R: 240, G: 240, B: 240, A: 255}, rr.Op(gtx.Ops))
                return layout.Dimensions{Size: gtx.Constraints.Min}
            },
            func(gtx layout.Context) layout.Dimensions {
                return material.Label(theme, unit.Sp(16), items[i]).Layout(gtx)
            },
        )
    })
})
```
