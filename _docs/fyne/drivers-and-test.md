# Drivers and testing

## `fyne.Driver` ([driver.go](../../repos-folder/fyne/driver.go))

The render backend abstraction. Picked at compile time based on build tags and OS; rarely accessed directly.

```go
type Driver interface {
    CreateWindow(string) Window
    AllWindows() []Window

    RenderedTextSize(text string, fontSize float32, style TextStyle, source Resource) (Size, baseline float32)
    CanvasForObject(CanvasObject) Canvas
    AbsolutePositionForObject(CanvasObject) Position

    Device() Device
    Run()
    Quit()

    StartAnimation(*Animation)
    StopAnimation(*Animation)

    DoubleTapDelay() time.Duration
    SetDisableScreenBlanking(bool)

    DoFromGoroutine(fn func(), wait bool)
}
```

`DoFromGoroutine` is what `fyne.Do(...)` and `fyne.DoAndWait(...)` call internally — every non-UI goroutine that needs to touch widgets must funnel through it.

## Driver implementations

| Package | Used for |
|---|---|
| `internal/driver/glfw/` | Desktop (Windows, macOS, Linux, BSD). GLFW + OpenGL. The default. |
| `internal/driver/mobile/` | iOS and Android. Wraps `gomobile` and the native view stack. |
| `internal/driver/software/` | Headless software rasterizer. Used with the `ci` build tag for tests. |
| `internal/driver/embedded/` (via `driver/embedded`) | Embedding Fyne into another GL context (e.g. game engines). |
| `internal/driver/web/` | WASM browser builds. Renders through WebGL via `fyne-io/gl-js`. |

The exported sub-packages under [driver/](../../repos-folder/fyne/driver/) give widgets a way to access driver-specific behavior without depending on the internal packages:

| Package | Purpose |
|---|---|
| `driver/desktop` | Desktop-only extensions: mouse cursors, modifier keys, secondary tap interpretation, extended shortcuts |
| `driver/mobile` | Mobile-only: device orientation, touch gestures, virtual keyboard control |
| `driver/software` | Software-rendered apps used for headless rendering (golden-image tests) |
| `driver/embedded` | Hooks for embedding into another GL context |

For widgets that want to be platform-aware, the idiom is type assertion:

```go
if desktopDriver, ok := fyne.CurrentApp().Driver().(desktop.Driver); ok {
    // desktop-only behavior
}
```

## `Device` ([device.go](../../repos-folder/fyne/device.go))

```go
type Device interface {
    Orientation() DeviceOrientation
    IsMobile() bool
    IsBrowser() bool
    HasKeyboard() bool
    SystemScaleForWindow(Window) float32
    Locale() Locale
}
```

`a.Driver().Device()` is the single source of truth for "am I on mobile?", "is there a hardware keyboard?", etc. The mobile driver returns true for `IsMobile`; the WASM driver returns true for `IsBrowser`.

## Testing ([test/](../../repos-folder/fyne/test/))

Fyne ships a headless testing harness so widget tests don't need an actual graphics context.

```go
import "fyne.io/fyne/v2/test"

func TestMyWidget(t *testing.T) {
    a := test.NewApp()
    defer a.Quit()

    w := test.NewWindow(widget.NewLabel("hi"))
    defer w.Close()

    test.AssertRendersToImage(t, "hi.png", w.Canvas())
}
```

Key helpers:

| Function | Purpose |
|---|---|
| `test.NewApp()` | An in-memory `fyne.App` |
| `test.NewWindow(content)` | A test window with the given content |
| `test.NewWindowWithPainter(...)` | Test window with a chosen painter |
| `test.Tap(obj)` / `test.DoubleTap(obj)` / `test.Drag(...)` | Synthesize input events |
| `test.Type(obj, "text")` | Synthesize typing |
| `test.AssertRendersToImage(t, "fixture.png", canvas)` | Golden-image comparison |
| `test.AssertObjectRendersToImage(...)` | Same for a single object |
| `test.AssertRendersToMarkup(...)` | Snapshot the canvas as XML-like markup |
| `test.Theme()` / `test.WithTestTheme()` | A stable theme for golden tests |
| `test.WidgetRenderer(w)` | Reach a widget's renderer |
| `test.MoveMouse(canvas, pos)` | Simulate cursor movement |

The `ci` build tag (`go test -tags ci ./...`) makes `app.New()` itself return a test app, so apps that don't import the test package can still run under headless CI.

## Test fixture data

Many widget tests have a `testdata/` directory next to them holding the expected golden PNGs and markup snapshots. `test.AssertRendersToImage` writes the actual output next to the expected file on mismatch, so failures produce a side-by-side image diff.

## Internal cache and threading

[cache.go](../../repos-folder/fyne/cache.go) and [thread.go](../../repos-folder/fyne/thread.go) live in the root package but are mostly internal — they back the widget renderer cache and the goroutine routing layer. App authors normally don't touch them; widget authors might use `cache.Renderer(widget)` from `internal/cache` to look up a renderer instance.

`fyne.NewTickerForWidget(w, period, fn)` / `fyne.Do` / `fyne.DoAndWait` are the supported scheduling primitives — see [thread.go](../../repos-folder/fyne/thread.go) for the implementation.
