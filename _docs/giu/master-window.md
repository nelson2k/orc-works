# MasterWindow

[MasterWindow.go](../../repos-folder/giu/MasterWindow.go) wraps the GLFW OS window plus the ImGui + implot + imnodes context. Exactly one `*MasterWindow` per program.

## Construction

```go
wnd := giu.NewMasterWindow(title string, width, height int, flags MasterWindowFlags) *MasterWindow
```

Flags (bit-OR):

| Flag | Effect |
|---|---|
| `MasterWindowFlagsNotResizable` | Fixed window size |
| `MasterWindowFlagsMaximized` | Start maximized |
| `MasterWindowFlagsFloating` | Always on top |
| `MasterWindowFlagsFrameless` | No window chrome |
| `MasterWindowFlagsTransparent` | Translucent window |
| `MasterWindowFlagsHidden` | Initially hidden (for multi-window setups) |

`NewMasterWindow` also:

- creates ImGui, implot, imnodes contexts
- spawns a `glfwbackend` via `cimgui-go`
- assigns the global `giu.Context` (set as a package-level singleton)
- disables the default `imgui.ini` (`SetUserFile("")`) — restore by calling `wnd.SetUserFile("myapp.ini")` if you want persistent layout state
- attaches the default theme (`DefaultTheme()`) and a default input handler
- sets the background color black and the content scale auto

## Running

```go
wnd.Run(func() {
    // build widget tree here — called every frame
})
```

`Run` routes the GLFW main loop through `mainthread.Call`, so it's safe to start on whatever goroutine you call it from. Internally it locks the OS thread.

## Methods

| Method | Purpose |
|---|---|
| `GetSize() (w, h int)` | Current window size in *display* coordinates |
| `SetSize(w, h int)` | Resize |
| `GetPos() (x, y int)` / `SetPos(x, y int)` | Position on screen |
| `SetTitle(string)` | Title bar |
| `SetBgColor(color.Color)` | Clear color for the OpenGL framebuffer |
| `SetTargetFPS(uint)` | Frame-rate cap (default no cap; the app idles to ~0 FPS when there's no input) |
| `SetStyle(*StyleSetter)` / `GetStyle()` | Master-window theme (a `StyleSetter` is a stack of `Push*`/`Pop*` style ops) |
| `SetIcon(icons ...image.Image)` | OS-side icon; pass multiple sizes — GLFW picks the best |
| `SetSizeLimits(minW, minH, maxW, maxH int)` | Pass `-1` (`GlfwDontCare`) to leave a side unconstrained |
| `SetCloseCallback(cb func() bool)` | `cb` returns `true` to accept the close, `false` to veto |
| `SetDropCallback(cb func([]string))` | Files dropped onto the window |
| `SetSizeChangeCallback(cb func(w, h int))` | Resize hook |
| `Close()` / `SetShouldClose(bool)` | Programmatic close |
| `RegisterKeyboardShortcuts(s ...WindowShortcut)` | Global hotkeys (see Shortcut.go) |
| `SetInputHandler(InputHandler)` | Swap the per-frame input handler (handles shortcuts) |
| `SetAdditionalInputHandlerCallback(InputHandlerHandleCallback)` | Extra hook before default handling |
| `SetUserFile(path string)` | Path for the ImGui `.ini` state file; `""` disables persistence |
| `SetScale(scale float32)` | Manual DPI scale; pass `0` for auto (default) |

## Lifecycle hooks

Internal hooks set on construction:

- `beforeRender` — rebuilds the font atlas if any new glyphs were used in the previous frame, drains texture load/free queues
- `afterRender` — no-op slot
- `beforeDestroy` — releases implot and imnodes contexts

You don't override these directly — the corresponding `Context.FontAtlas`, `Context.textureLoadingQueue`, `Context.textureFreeingQueue` are the public seams.

## Multi-window programs

ImGui's "viewports" (multiple OS-level windows) aren't enabled. To show multiple top-level windows, run multiple `*MasterWindow`s — but cimgui-go currently allows only one global ImGui context, so this is *not* fully supported and is documented as "experimental". The supported pattern is one `MasterWindow` containing multiple `g.Window("Title")` ImGui sub-windows (children of the OS window).

## CSS theming

[CSS.go](../../repos-folder/giu/CSS.go) lets you define theme tags via a CSS-like syntax:

```go
sheet, _ := giu.ParseCSSStyleSheet(`
  .button { background-color: #2266cc; color: white; padding: 8px; }
`)
giu.Context.SetCSSStylesheet(sheet)
```

Tags can be attached to widgets via `.Tag("button")`. The master window automatically applies the `MainTag` stylesheet around each frame.

## Background color and DPI

- `SetBgColor` accepts any `color.Color`; it's converted to ImGui Vec4 internally.
- DPI awareness is on by default — both the font atlas and ImGui's StyleVar pixel sizes are scaled. Use `SetScale(0)` to revert to auto-detect.

## Background CPU cost

The library only repaints when there's input activity (the idle path tunes refresh rate down). At 60 FPS with no input, CPU usage hovers near 0.5% on typical hardware.
