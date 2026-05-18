# App and Window

Package: [gioui.org/app](../../repos-folder/gio/app/)

The `app` package is the OS-specific bridge. It owns the `Window`, the main loop, GL/Metal/Vulkan/D3D11 contexts, the OS event pumps, and platform-conditional shims.

## `app.Main()`

```go
func Main()
```

Must be called from the program's `main()`. On macOS, iOS, Android and Wayland, the OS requires that GUI work happen on the main OS thread. `app.Main` blocks for the rest of the program's life and pumps the platform's run-loop.

Pattern:

```go
func main() {
    go func() {
        w := new(app.Window)
        for { /* event loop */ }
    }()
    app.Main()
}
```

## `app.Window`

```go
type Window struct { ... }   // zero value is useful
```

The first call to `(*Window).Event()` actually creates and shows the OS window. On iOS / Android / WASM only one Window is supported; on desktop you can create multiple, each in its own goroutine.

### Methods

| Method | Purpose |
|---|---|
| `Event() event.Event` | Block for the next event for this window. Iterates lifetime events: `app.FrameEvent`, `app.ConfigEvent`, `app.DestroyEvent`, `key.Event`, etc. |
| `Option(opts ...Option)` | Configure the window (size, title, mode, decorated, ...) |
| `Invalidate()` | Schedule a redraw — produces a fresh `FrameEvent` |
| `Run(f func())` | Run a function on the window's GUI goroutine |
| `Perform(actions system.Action)` | Trigger a platform action (close, minimize, raise, ...) |

### Options ([app/os.go](../../repos-folder/gio/app/os.go))

`app.Option` is a function `func(unit.Metric, *Config)`. Built-in options:

- `app.Title(string)` — title bar text
- `app.Size(w, h unit.Dp)` / `app.MinSize(w, h)` / `app.MaxSize(w, h)`
- `app.Decorated(bool)` — chrome on/off
- `app.CustomRenderer(bool)` — let the app draw via GPU rather than the built-in painter
- `app.StatusColor(color.NRGBA)`, `app.NavigationColor(color.NRGBA)` (Android / mobile)
- `app.TopMost(bool)`
- `Windowed`, `Fullscreen`, `Minimized`, `Maximized` (constants of `app.WindowMode`) → `.Option()`
- `AnyOrientation`, `LandscapeOrientation`, `PortraitOrientation` (Android / JS) → `.Option()`

Apply them either at construction (`w.Option(app.Title("Hello"), app.Size(800, 600))`) or any time later.

### Config and ConfigEvent

```go
type Config struct {
    Size, MaxSize, MinSize image.Point
    Title           string
    Mode            WindowMode
    StatusColor     color.NRGBA
    NavigationColor color.NRGBA
    Orientation     Orientation
    CustomRenderer  bool
    Decorated       bool
    TopMost         bool
    Focused         bool
}
```

`app.ConfigEvent` is sent whenever any of those change. Useful for adapting layout to the current mode (e.g. immersive vs windowed).

## FrameEvent

```go
type FrameEvent struct {
    Now    time.Time
    Metric unit.Metric    // px-per-dp / px-per-sp
    Size   image.Point
    Insets Insets         // safe-area insets
    Frame  func(frame *op.Ops)
    Queue  event.Queue
    ...
}
```

`Frame(ops)` is the call that submits a complete redraw to the OS-side renderer. Build the ops list, then call `e.Frame(&ops)`.

`app.NewContext(&ops, e)` constructs a fresh `layout.Context` from a `FrameEvent`. It carries the constraints (window size), the ops list, and the queue for input event delivery during this frame.

## DestroyEvent

```go
type DestroyEvent struct{ Err error }
```

Sent when the OS closes the window. The program must stop reading events when this arrives.

## App-level events: `app.Events`

```go
app.Events(func(e event.Event) bool {
    switch e := e.(type) {
    case app.URLEvent:
        // deep link opened
    }
    return true
})
```

App-level events (URL handlers, theme changes coming from the OS) aren't bound to a particular window. The iterator yields them; returning `false` stops iteration.

## DataDir

```go
dir, err := app.DataDir()
```

Returns an OS-appropriate per-app data directory (`%APPDATA%/<app>` on Windows, `~/Library/Application Support/<app>` on macOS, `$XDG_DATA_HOME` on Linux, sandboxed paths on mobile).

## Per-OS implementation files

Inside [app/](../../repos-folder/gio/app/) you'll find platform-specific files compiled by build tags:

| OS | Bridge files |
|---|---|
| Linux Wayland | `os_wayland.go` + `.c`, `wayland_*.c` |
| Linux X11 | `os_x11.go`, `egl_x11.go`, `vulkan_x11.go` |
| Windows | `os_windows.go`, `egl_windows.go`, `d3d11_windows.go`, `log_windows.go`, `vulkan*.go` |
| macOS | `os_macos.go` + `.m`, `gl_macos.go` + `.m`, `metal_macos.go`, `egl_*` |
| iOS | `os_ios.go` + `.m`, `gl_ios.go` + `.m`, `metal_ios.go`, `framework_ios.h`, `log_ios.go` |
| Android | `os_android.go`, `egl_android.go`, `vulkan_android.go`, `Gio.java`, `GioActivity.java`, `GioView.java`, `log_android.go` |
| WASM/JS | `os_js.go`, `gl_js.go` |

These are normally invisible to app code.

## Permissions

[app/permission/](../../repos-folder/gio/app/permission/) holds tag packages — importing one of them advertises that the program needs that permission (mostly Android). They are blank imports that the build system reads.
