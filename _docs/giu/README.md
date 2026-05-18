# giu

[github.com/AllenDang/giu](https://github.com/AllenDang/giu) — a declarative cross-platform GUI framework for Go on top of [Dear ImGui](https://github.com/ocornut/imgui), via the [cimgui-go](https://github.com/AllenDang/cimgui-go) cgo binding.

The library wraps the Dear ImGui immediate-mode model in a fluent builder API so you write `g.Button("Save").OnClick(...)` rather than calling raw ImGui draw commands. It also bundles GLFW for the OS window, OpenGL for the renderer, and DPI-aware font atlas rebuilding.

## Module identity

```
module github.com/AllenDang/giu
go 1.25.0
```

Main deps (from [go.mod](../../repos-folder/giu/go.mod)):

| Module | Purpose |
|---|---|
| `github.com/AllenDang/cimgui-go` | cgo Dear ImGui binding (provides imgui, implot, immarkdown, imnodes, imguizmo) |
| `github.com/AllenDang/go-findfont` | OS font discovery |
| `github.com/faiface/mainthread` | main-thread routing for OpenGL |
| `github.com/gucio321/glm-go` | glm-style float math |
| `github.com/mazznoer/csscolorparser`, `github.com/napsy/go-css` | CSS color/parsing for theme files |
| `github.com/pkg/browser` | Open URLs in default browser |
| `github.com/sahilm/fuzzy` | Fuzzy search (filter widget) |
| `golang.design/x/hotkey` | Global hotkeys |
| `golang.org/x/image` | Image utilities |

## Top-level layout

`repos-folder/giu/` is a flat package — all the Go files live in the package root (`package giu`). There's no submodule for widgets or layouts; everything is `giu.X`.

| File | Concern |
|---|---|
| `MasterWindow.go` | OS window: `NewMasterWindow`, `(*MasterWindow).Run`, flags, icon, drop callback, close callback |
| `Context.go` | `*GIUContext`, ID generation, generic `SetState[T]` / `GetState[T]` for per-frame state |
| `Backend.go` / `Texture*.go` / `Style.go` / `StyleSetter.go` | cimgui-go bridges and styling helpers |
| `Layout.go` | `Layout` (slice of widgets) + the central rendering loop |
| `Window.go` | `SingleWindow()`, `Window(title)`, `SingleWindowWithMenuBar()` |
| `Widgets.go` | The bulk of the widget set: Row, Column, Combo, ContextMenu, Menu, ProgressBar, Separator, Dummy, Spacing, Tooltip, TabBar/TabItem |
| `TextWidgets.go` | Label, InputText, InputTextMultiline, InputInt, InputFloat, Bullet, BulletText |
| `ClickableWidgets.go` | Button, ArrowButton, SmallButton, InvisibleButton, ImageButton, Checkbox, RadioButton, Selectable, TreeNode, Link |
| `SliderWidgets.go` | Slider/VSlider/DragInt/DragFloat |
| `ImageWidgets.go` | Image, ImageWithRgba, ImageWithFile, ImageWithURL |
| `TableWidgets.go` | Table, TableRow, TableColumn, TreeTable, TreeTableRow |
| `ListClipper.go` | Virtualized lists for huge datasets |
| `Plot.go` | implot wrapper — Bar/Line/Scatter/PieChart/etc. |
| `Markdown.go` | immarkdown wrapper |
| `CodeEditor.go` | ImGuiColorTextEdit wrapper |
| `MemoryEditor.go` | imgui_memory_editor wrapper |
| `Gizmo.go` | imguizmo wrapper (3D gizmos) |
| `Canvas.go` | Custom drawing (low-level draw lists) |
| `SplitLayout.go` / `Splitter` (in ExtraWidgets) | Resizable splitters |
| `StackWidget.go` | Z-stack widget |
| `Popups.go` | Popup / PopupModal |
| `Msgbox.go` | Pre-baked message-box system |
| `ContextMenu*` | Right-click menus |
| `ExtraWidgets.go` | DatePicker, ListBox, RangeBuilder, MapRangeBuilder, Custom, Condition |
| `ProgressIndicator.go` | Spinner |
| `ReflectiveBoundTexture.go` / `StatefulReflectiveBoundTexture.go` | Texture state bridges for image reflection |
| `EventHandler.go` | `Event()` — global event tap |
| `InputHandler.go` | Keyboard shortcuts |
| `Shortcut.go` (via Keycode.go etc.) | Shortcut definitions |
| `Themes.go` | DefaultTheme, LightTheme |
| `FontAtlasProsessor.go` | Incremental font atlas rebuilds |
| `Translator.go` | Lightweight i18n |
| `SurfaceLoaders.go` | Async image/surface loading |
| `Direction.go`, `Keycode.go`, `Alignment.go`, `Flags.go` | Enums used by widgets |
| `CSS.go` | CSS-style theme parsing |
| `mainthread_*.go` | OS-conditional main-thread shims |
| `build_windows.go` | Windows manifest/build flags |
| `examples/` | Demo apps |
| `cmd/` | Companion CLIs |
| `docs/` | Auxiliary docs |

## The Hello-world

```go
package main

import g "github.com/AllenDang/giu"

func loop() {
    g.SingleWindow().Layout(
        g.Label("Hello world from giu"),
        g.Row(
            g.Button("Click Me").OnClick(func(){ /* ... */ }),
            g.Button("I'm so cute"),
        ),
    )
}

func main() {
    wnd := g.NewMasterWindow("Hello world", 400, 200, g.MasterWindowFlagsNotResizable)
    wnd.Run(loop)
}
```

The `loop` function is called every frame. Widgets are values; their `Build()` method emits the ImGui calls. The `Run(loop)` call drives the GLFW main loop and only redraws on user events (≈0.5% CPU at 60 FPS idle).

## The model

- **Declarative**: each frame produces a fresh widget tree. ImGui retains nothing across frames except the IDs the library generates from labels and positions.
- **Builder API**: most widgets return `*Widget` with chainable setters (`Size(w, h)`, `Flags(...)`, `OnClick(...)`).
- **State via callbacks**: bind state as pointers (`*bool`, `*string`, `*float32`) into widgets that need writable state (Input, Slider, Checkbox).
- **Generic state**: `g.SetState[T](ctx, id, ptr)` / `g.GetState[T](ctx, id)` for non-pointer state.
- **Layout primitives**: `g.Layout{...}`, `g.Row(...)`, `g.Column(...)`, `g.SameLine()`, `g.Separator()`, `g.Dummy(w,h)`, `g.Splitter(...)`, `g.SplitLayout(...)`.

## Platforms

Built on GLFW v3.3 + OpenGL 3.2, restricted by cimgui-go's build matrix: Windows, macOS, Linux. Requires a C/C++ toolchain (cgo links the ImGui C++).

