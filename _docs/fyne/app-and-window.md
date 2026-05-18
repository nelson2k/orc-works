# App and Window

The two top-level lifecycle types.

## `fyne.App`

Defined in [app.go](../../repos-folder/fyne/app.go). Implemented by `*fyneApp` in [app/app.go](../../repos-folder/fyne/app/app.go). One per process; access the active instance via `fyne.CurrentApp()`.

Constructors:

```go
app.New()                       // anonymous app; Preferences won't persist correctly
app.NewWithID("com.example.x")  // recommended — required for Preferences
```

Build tags pick the underlying driver:

| Tag | Driver | Picked when |
|---|---|---|
| (none) | GLFW desktop driver | Default desktop build |
| `mobile` | Mobile simulator driver | Local mobile testing |
| `ci` | Software in-memory driver | Headless test/CI runs |
| `android` (via `gomobile`) | Android driver | Mobile build |
| `ios` (via `gomobile`) | iOS driver | Mobile build |
| `wasm` / `js` | WASM driver with WebGL | Web build |

### Methods

| Method | Purpose |
|---|---|
| `NewWindow(title) Window` | Open a new top-level window |
| `Run()` | Start the event loop (blocks) |
| `Quit()` | Exit cleanly; no-op on iOS/Android |
| `Driver() Driver` | Underlying driver — rarely needed |
| `UniqueID() string` | Set via `NewWithID` or `FyneApp.toml` |
| `Icon() / SetIcon(Resource)` | App icon |
| `OpenURL(*url.URL)` | Open in default browser |
| `SendNotification(*Notification)` | OS notification immediately |
| `ScheduleNotification(n, deliverAt) (*ScheduledNotification, error)` | Queue for future delivery (2.8+) |
| `CancelScheduledNotification(id)` | Cancel by ID |
| `Settings() Settings` | Global theme / scaling settings |
| `Preferences() Preferences` | Key/value persistence; needs `NewWithID` |
| `Storage() Storage` | App-scoped file storage |
| `Clipboard() Clipboard` | System clipboard (2.6+) |
| `Cache() Cache` | App-scoped in-memory cache (2.8+) |
| `Lifecycle() Lifecycle` | Foreground/background/start/stop hooks (2.1+) |
| `Metadata() AppMetadata` | Build-time metadata from `FyneApp.toml` (2.2+) |
| `CloudProvider() / SetCloudProvider(p)` | Cloud sync hook (2.3+) |

### Master window and exit behavior

By default Fyne exits the process when the last window closes. To anchor exit on a specific "master" window:

```go
w.SetMaster()
```

Closing any non-master windows leaves the app running. Closing the master triggers `App.Quit()`.

To intercept close requests on the master (e.g. confirm-on-exit):

```go
w.SetCloseIntercept(func() {
    // show a confirm dialog; call w.Close() only if approved
})
```

`SetCloseIntercept` replaces the default close behavior — you must call `w.Close()` yourself to actually close.

### Lifecycle hooks (2.1+)

```go
a.Lifecycle().SetOnStarted(func() { ... })
a.Lifecycle().SetOnStopped(func() { ... })
a.Lifecycle().SetOnEnteredForeground(func() { ... })
a.Lifecycle().SetOnExitedForeground(func() { ... })
```

`Started` fires once when the main loop begins running; `Stopped` fires once when the loop exits. `EnteredForeground` / `ExitedForeground` fire whenever the app gains or loses focus (critical on mobile).

### Metadata

```go
md := a.Metadata()
// md.ID, md.Name, md.Version, md.Build, md.Icon, md.Release, md.Custom, md.Migrations
```

Comes from a `FyneApp.toml` file next to `main.go` at build time. Standard fields:

```toml
[Details]
Icon    = "Icon.png"
Name    = "My App"
ID      = "io.fyne.demo"
Version = "1.0.0"
Build   = 1

[Development]
# anything; surfaces under Custom map

[Migrations]
# feature opt-ins for 2.6+
```

`md.Release == true` only when built with `-tags release` (or via `fyne release`).

## `fyne.Window`

Defined in [window.go](../../repos-folder/fyne/window.go). Provides title / icon / fullscreen / size / focus / padding / content / canvas plus lifecycle hooks.

### Common operations

```go
w := a.NewWindow("My App")
w.Resize(fyne.NewSize(800, 600))
w.SetFixedSize(true)          // disable resize
w.CenterOnScreen()
w.SetIcon(myIconResource)
w.SetPadded(false)            // edge-to-edge content
w.SetMainMenu(menu)           // top-level menu
w.SetFullScreen(true)
w.SetMaster()
w.SetCloseIntercept(func() { ... })
w.SetOnClosed(func() { ... })
w.SetOnDropped(func(pos fyne.Position, uris []fyne.URI) { ... })  // 2.4+
w.RequestFocus()
w.SetContent(rootObject)
w.Show()                      // or w.ShowAndRun() to also start the loop
w.Hide()
w.Close()
```

### Canvas access

```go
c := w.Canvas()
c.SetOnTypedKey(func(k *fyne.KeyEvent) { ... })
c.SetOnTypedRune(func(r rune) { ... })
c.AddShortcut(shortcut, func(s fyne.Shortcut) { ... })
c.Focus(focusableWidget)
img := c.Capture()    // screenshot of current canvas
```

Each window has its own canvas — keyboard handlers, focus, and shortcuts all live on the canvas, not the window directly.

### Notes on multi-window

Mobile drivers typically render only one window (subsequent calls reuse the same surface). On desktop you can have as many windows as you like; the GLFW driver multiplexes them. The container package provides an `InnerWindow` widget for SDI-style child windows inside the main window (2.4+).

## Process model and singleton state

`fyne.SetCurrentApp(a)` is called automatically by `app.New*`. `fyne.CurrentApp()` lets widgets reach the active app without explicit plumbing — used heavily inside the standard widgets to fetch the theme, settings, and preferences. The implementation is an `atomic.Pointer[App]` so reads are lock-free.
