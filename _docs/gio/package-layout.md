# Package layout

Contents of `repos-folder/gio/`:

```
gio/
├── go.mod                     module gioui.org, go 1.24
├── go.sum
├── README.md / LICENSE
├── flake.nix / flake.lock     Nix dev shell
│
├── app/                       OS bridge: Window, event loop, FrameEvent, ConfigEvent, Options
│   ├── app.go                 NewContext, Main, Events, DataDir
│   ├── window.go              *Window, methods, options
│   ├── os.go                  Config, WindowMode, Orientation
│   ├── system.go              app-wide event glue
│   ├── doc.go                 package overview
│   ├── runmain.go             Main() implementation
│   ├── datadir.go             DataDir() per-OS helper
│   ├── ime.go / ime_test.go   IME state & glue
│   ├── d3d11_windows.go       D3D11 backend on Windows
│   ├── egl_*.go               EGL setup on Wayland/X11/Android/Windows
│   ├── gl_*.go / *.m          OpenGL setup on macOS/iOS/JS
│   ├── metal_*.go             Metal backend on macOS/iOS
│   ├── vulkan*.go             Vulkan backend on Android/Wayland/X11
│   ├── os_*.go / *.m / *.c    Per-platform window driver
│   ├── log_*.go               Per-platform logging glue
│   ├── wayland_*.{c,h}        Wayland protocol stubs
│   ├── permission/            Tag packages for OS permissions
│   ├── Gio.java / GioActivity.java / GioView.java   Android Java bridge
│   └── framework_ios.h        iOS framework header
│
├── widget/                    UI state and event handling (no rendering)
│   ├── button.go              widget.Clickable
│   ├── editor.go              widget.Editor (text input, IME-aware)
│   ├── selectable.go          read-only selectable text
│   ├── list.go                widget.List + widget.Scrollbar
│   ├── bool.go                widget.Bool
│   ├── enum.go                widget.Enum (radio groups)
│   ├── float.go               widget.Float (slider state)
│   ├── icon.go                widget.Icon, NewIcon
│   ├── image.go               widget.Image
│   ├── label.go               widget.Label (drawing-free label state)
│   ├── text.go                shared text rendering helpers
│   ├── border.go              widget.Border
│   ├── decorations.go         widget.Decorations
│   ├── dnd.go                 drag-and-drop state
│   ├── buffer.go              rope buffer for the editor
│   ├── index.go               index for the editor
│   └── material/              Material-design theme
│       ├── theme.go           NewTheme, Theme, Palette
│       ├── button.go          Button, ButtonLayout, IconButton, Clickable
│       ├── checkbox.go        CheckBox
│       ├── radiobutton.go     RadioButton
│       ├── switch.go          Switch
│       ├── slider.go          Slider
│       ├── progressbar.go     ProgressBar
│       ├── progresscircle.go  ProgressCircle
│       ├── loader.go          Loader
│       ├── editor.go          Editor
│       ├── label.go           H1..H6, Body1/2, Subtitle1/2, Caption, Overline, Label
│       ├── list.go            List, Scrollbar
│       ├── decorations.go     Decorations
│       └── checkable.go       checkbox/radio shared bits
│
├── layout/                    Constraint-based layout
│   ├── layout.go              Constraints, Dimensions, Axis, Alignment, Direction, Inset, Spacer
│   ├── context.go             Context, Dp, Sp, Disabled
│   ├── flex.go                Flex, FlexChild, Rigid, Flexed, Spacing
│   ├── stack.go               Stack, StackChild, Stacked, Expanded, Background
│   ├── list.go                List, ListElement, Position, ScrollBy, ScrollTo
│   └── fit.go                 image fit helpers (Contain, Cover, ...)
│
├── op/                        Operations
│   ├── op.go                  Ops, MacroOp, CallOp, TransformOp, TransformStack, Offset, Affine, Defer
│   ├── clip/
│   │   ├── clip.go            Op, Stack, Path, PathSpec
│   │   ├── shapes.go          Rect, RRect, Ellipse, UniformRRect, Outline, Stroke
│   │   └── doc.go             package overview
│   └── paint/
│       ├── paint.go           ColorOp, ImageOp, LinearGradientOp, PaintOp, FillShape, Fill, PushOpacity, ImageFilter
│       └── doc.go             package overview
│
├── text/                      Text shaping
│   ├── shaper.go              Shaper, ShaperOption, NewShaper, NoSystemFonts, WithCollection
│   ├── text.go                Alignment, WrapPolicy
│   ├── gotext.go              go-text/typesetting integration
│   ├── family_parser.go       CSS-style font-family string parsing
│   ├── lru.go                 shaped-run cache
│   └── testdata/              golden test fixtures
│
├── font/                      Font types and faces
│   ├── font.go                Font, Typeface, Style, Weight
│   ├── gofont/                Embedded Go family + Go Mono
│   └── opentype/              OpenType / TrueType loader
│
├── io/                        Input + OS-side IO
│   ├── event/                 Event, Tag, Filter, Op
│   ├── pointer/               Pointer events, Cursor, Filter, GrabCmd, PassOp
│   ├── key/                   Key events, Modifiers, Filter, FocusCmd, SoftKeyboardCmd, EditEvent
│   ├── clipboard/             ReadOp, WriteOp
│   ├── transfer/              Drag-and-drop transfer (DataEvent, SourceFilter, TargetFilter)
│   ├── input/                 Internal event router
│   ├── semantic/              Accessibility (LabelOp, DescriptionOp, Button, ...)
│   ├── system/                Action constants, Locale
│   └── permission/            (empty tag packages — re-exported under app/permission)
│
├── gesture/                   gesture.Click, Drag, Hover, Scroll
│
├── gpu/                       GPU command construction (clip rasterizer, draw scheduler)
│   ├── api.go                 GPU interface
│   ├── caches.go              GPU resource caches
│   ├── clip.go                clip rasterization
│   ├── gpu.go                 backend selection
│   ├── pack.go                texture atlas packing
│   ├── path.go                stroke + fill path tessellation
│   ├── timer.go               frame timing
│   ├── headless/              off-screen render target
│   └── internal/              per-backend implementations (gl, vulkan, d3d11, metal)
│
├── f32/                       Float32 2D math (Point, Rectangle, Affine2D)
├── unit/                      Dp, Sp, Metric
│
└── internal/                  Implementation details — not part of the public API
    ├── byteslice/             Type-punning helpers
    ├── cocoainit/             AppKit init
    ├── d3d11/                 Direct3D 11 wrapper
    ├── debug/                 Debug overlays / opt-in tracing
    ├── egl/                   EGL wrapper
    ├── f32/                   Internal f32 helpers
    ├── f32color/              Float color math
    ├── fling/                 Fling animation
    ├── gl/                    OpenGL/GLES wrapper
    ├── ops/                   Lower-level op encoding (Ops, PC, StackID)
    ├── scene/                 Scene-graph encoding (Commands)
    ├── stroke/                Stroke flattening
    └── vk/                    Vulkan wrapper
```

## Module identity

```
module gioui.org
go 1.24.0
```

## Major dependencies

| Module | Purpose |
|---|---|
| `gioui.org/shader` | Precompiled shaders |
| `eliasnaur.com/font` | Default font bundle |
| `github.com/go-text/typesetting` | Text shaping |
| `golang.org/x/exp` | (transitive) experimental APIs |
| `golang.org/x/exp/shiny` | Material icon data |
| `golang.org/x/image` | Image utilities, `gofont` |
| `golang.org/x/sys` | OS syscalls |
| `golang.org/x/text` | Bidi, Unicode tables |
| `golang.org/x/net` | (transitive) |

## Build tags

The platform-specific files in `app/` and `internal/` rely on Go build tags (`darwin`, `windows`, `linux`, `js`, `wasm`, `ios`, `android`, ...). Building for mobile requires the `gioui.org/cmd/gogio` tool (in a separate repo).

## Internal packages

The `gio/internal/` tree is unsupported as a public API. App code should not import from it; the public seams are `op/`, `op/clip/`, `op/paint/`, `layout/`, `widget/`, `widget/material/`, `text/`, `font/`, `io/`, `gesture/`, `f32/`, `unit/`, `app/`.
