# Gio

[`gioui.org`](https://gioui.org) — an immediate-mode GUI toolkit for Go. Supports Android, iOS, macOS, Linux, FreeBSD, OpenBSD, Windows, and (experimentally) WebAssembly.

Immediate-mode means the UI is re-issued every frame: the program builds an [op.Ops](../../repos-folder/gio/op/op.go) instruction list and hands it to the window via `FrameEvent.Frame(ops)`. There is no retained scene graph — state lives in the program.

## Module identity

```
module gioui.org
go 1.24.0
```

Major deps (from [go.mod](../../repos-folder/gio/go.mod)):

| Module | Purpose |
|---|---|
| `gioui.org/shader` | Precompiled shader bytecode |
| `github.com/go-text/typesetting` | Text shaping |
| `eliasnaur.com/font` | Default font set |
| `golang.org/x/image`, `golang.org/x/text`, `golang.org/x/sys` | Standard extensions |
| `golang.org/x/exp/shiny` | Material-design icons |

## Top-level layout

```
gio/
├── app/                 Platform integration: windows, event loop, GL/Metal/Vulkan/D3D11/Wayland/X11/JS bridges
├── widget/              Stateful UI primitives (Button, Editor, List, Slider, Icon, Image, Label, ...)
│   └── material/        A Material-design theme wrapping widget primitives
├── layout/              Constraint-based layout: Inset, Flex, Stack, List, Spacer, Direction
├── op/                  The operation list (Ops) and macros
│   ├── clip/            Clip ops + shape builders (Path, RRect, Outline)
│   └── paint/           Color, image, linear-gradient fills + PaintOp
├── text/                The Shaper, family parser, gotext bridge
├── font/                Default + opentype font support
│   ├── gofont/          Go family (Go, Go Mono) embedded
│   └── opentype/        OpenType loader
├── io/                  Input and OS-side IO
│   ├── pointer/         Pointer events (mouse + touch)
│   ├── key/             Keyboard events + modifier maps (per OS)
│   ├── event/           Generic event interface
│   ├── input/           Event router (private to the event-dispatch path)
│   ├── clipboard/       Clipboard read/write events
│   ├── transfer/        Drag-and-drop transfer events
│   ├── semantic/        Accessibility metadata
│   ├── system/          Window lifecycle events
│   └── permission/      Imported tag packages for OS permissions
├── gesture/             Higher-level gesture recognizers (click, drag, hover, scroll)
├── gpu/                 GPU command construction (compiled shaders, drawcalls)
│   ├── headless/        Off-screen rendering target
│   └── internal/        Backend-specific GPU bits
├── f32/                 Float32 2D math: Point, Rectangle, Affine2D
├── unit/                Density-independent units: Dp (display pixels), Sp (scaled pixels)
└── internal/            Implementation details — not part of the public API
    ├── byteslice, cocoainit, d3d11, debug, egl, f32, f32color, fling,
    │   gl, ops, scene, stroke, vk
```

## The three core layers

1. **Operations** ([op/](../../repos-folder/gio/op/)) — `op.Ops` is an append-only command buffer. Drawing calls (`paint.ColorOp`, `paint.PaintOp`), clip pushes (`clip.RRect{}.Push(ops)`), transforms (`op.Offset(...)`), and input registrations all go in here.
2. **Layout** ([layout/](../../repos-folder/gio/layout/)) — composable widgets that take a `layout.Context` (carrying constraints and `*op.Ops`) and return `layout.Dimensions`. The standard layouts (`Flex`, `Stack`, `List`, `Inset`, `Direction`, `Spacer`) compose into arbitrary trees.
3. **Widgets** ([widget/](../../repos-folder/gio/widget/)) — stateful structs (`widget.Button`, `widget.Editor`, `widget.List`, `widget.Bool`) that record events. They have no drawing methods of their own; a *theme* (e.g. [widget/material/](../../repos-folder/gio/widget/material/)) turns a `widget.X` plus a `Theme` into a `layout.Widget` function that renders it.

## The window lifecycle

```go
func main() {
    go func() {
        w := new(app.Window)
        var ops op.Ops
        for {
            switch e := w.Event().(type) {
            case app.FrameEvent:
                gtx := app.NewContext(&ops, e)
                // ... build widget tree against gtx ...
                e.Frame(gtx.Ops)
            case app.DestroyEvent:
                return
            }
        }
    }()
    app.Main()  // hands control of the main thread to the OS
}
```

The pattern is: spawn a goroutine that owns the window, call `app.Main()` from the actual `main` function to satisfy macOS/iOS/Android main-thread requirements.

