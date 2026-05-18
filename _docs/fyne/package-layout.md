# Package layout

Contents of `repos-folder/fyne/`:

```
fyne/                       core package — interfaces only, no rendering
├── fyne.go                 package doc comment + import path declaration
├── app.go                  App interface, AppMetadata, Lifecycle, NewWithID glue
├── window.go               Window interface
├── canvas.go               Canvas interface (root-level Canvas object container)
├── canvasobject.go         CanvasObject + behavior interfaces (Tappable, Focusable, ...)
├── container.go            *fyne.Container (deprecated direct constructors)
├── widget.go               Widget + WidgetRenderer interfaces
├── layout.go               Layout interface
├── driver.go               Driver interface
├── device.go               Device interface
├── theme.go                Theme interface + theme name enums
├── settings.go             Settings interface (app-wide theme/scale)
├── preferences.go          Preferences interface
├── storage.go              Storage interface
├── cloud.go                CloudProvider interface (2.3+)
├── clipboard.go            Clipboard interface (2.6+)
├── menu.go                 Menu, MainMenu types
├── shortcut.go             Shortcut and built-in shortcut types
├── key.go                  KeyEvent, Modifier, KeyName constants
├── key_*.go                Per-platform key tables
├── notification.go         Notification + ScheduledNotification types
├── overlay_stack.go        OverlayStack interface for canvas overlays
├── accessibility.go        Accessible interface + roles
├── animation.go            Animation type + curves
├── event.go                Common event types: PointEvent, ScrollEvent, DragEvent, ...
├── geometry.go             Position, Size, NewPos, NewSize, NewSquareOffsetPos
├── math.go                 Helper math
├── resource.go             Resource interface, StaticResource
├── scroll.go               ScrollDirection enum
├── serialise.go            Resource serialization helpers
├── text.go                 TextStyle, TextAlign, TextWrap
├── thread.go               Do / DoAndWait — UI-thread routing helpers
├── tools/                  Developer tools
├── uri.go                  URI / ListableURI / URIReadCloser / URIWriteCloser interfaces
├── validation.go           StringValidator interface
├── log.go                  LogError / LogWarn / LogDebug
├── cache.go                Cache interface (2.8+)
│
├── app/                    Default fyne.App implementation
│   ├── app.go              fyneApp struct, New() / NewWithID()
│   ├── app_darwin.{go,m}   macOS desktop bits (cgo/objc)
│   ├── app_desktop_darwin.* macOS-specific
│   ├── app_mobile_*.{go,c,m} iOS / Android
│   ├── app_windows.go      Win32 desktop
│   ├── app_xdg.go          Linux/BSD (XDG)
│   ├── app_wasm.go         Browser (WASM)
│   ├── app_noos.go         No-OS / embedded
│   ├── app_other.go        Fallbacks
│   ├── app_gl.go           OpenGL setup
│   ├── app_software.go     Software-renderer app
│   ├── settings*.go        Per-platform settings persistence (theme/scale)
│   ├── preferences*.go     Per-platform preferences persistence
│   ├── cache*.go           Per-platform cache
│   ├── storage.go          App-scoped storage
│   ├── meta*.go            FyneApp.toml parsing
│   ├── cloud.go            Cloud provider wiring
│   ├── icon_cache_file.go  Icon caching
│   └── ...
│
├── canvas/                 Canvas primitives (Text, Image, Rectangle, Circle, Line,
│                              Raster, Gradient, Arc, BezierCurve, Polygon, ...)
│
├── container/              Layout-flavored constructors (NewVBox, NewHBox, NewBorder,
│                              NewGridWith*, NewStack/Max, NewScroll, NewPadded, NewCenter)
│                           plus widget-flavored composite containers (AppTabs, DocTabs,
│                              Split, Scroll, InnerWindow, MultipleWindows, Navigation)
│
├── data/
│   ├── binding/            Reactive bindings (Bool, Int, Float, String, lists, maps,
│   │                       trees, preference-backed, sprintf, converters)
│   └── validation/         All / Regexp / Time validators
│
├── dialog/                 Stock dialogs: Information, Error, Confirm, Form, Entry,
│                              Custom, Progress (+ infinite), ColorPicker,
│                              FileOpen/FileSave/FolderOpen (with per-platform impls)
│
├── driver/                 Public driver interfaces and helpers
│   ├── desktop/            Cursors, modifier keys, mouse, shortcut, extended interfaces
│   ├── mobile/             Touch, virtual keyboard, device orientation
│   ├── embedded/           Embedding hooks
│   ├── software/           Software-renderer driver
│   └── native.go           Native window handle accessors
│
├── img/                    README screenshots
│
├── internal/               Implementation details — NOT part of the public API
│   ├── animation/          Animation engine
│   ├── async/              Goroutine helpers
│   ├── build/              Build flags
│   ├── cache/              Renderer / texture cache
│   ├── color/              Color helpers (premultiplied, gamma)
│   ├── driver/             The actual driver implementations (glfw, mobile, software, web)
│   ├── painter/            Software + GL painters
│   ├── repository/         File / HTTP / in-memory storage repositories
│   ├── scale/              DPI scaling helpers
│   ├── scheduler/          Notification scheduling
│   ├── svg/                SVG parsing
│   ├── test/               Internal test plumbing
│   ├── theme/              Internal theme constants
│   ├── widget/             Internal widget building blocks (BaseRenderer, ...)
│   └── ...
│
├── lang/                   i18n: AddTranslations*, X(), N(), SystemLocale()
│   └── translations/       TOML files per locale (~30 languages)
│
├── layout/                 Box / Grid / GridWrap / Border / Form / Center / Stack
│                              / Padded / CustomPadded / RowWrap / Spacer
│
├── storage/                URI helpers, Reader/Writer/Copy/Move/Delete, ListerForURI
│   └── repository/         Repository interface; built-in scheme handlers
│
├── theme/                  Default theme, color/size/icon/font name enums, JSON theme
│   ├── bundled-fonts.go    Bundled Inter / Source Code Pro
│   ├── bundled-icons.go    Bundled Material-style icons
│   ├── bundled-emoji.go    Bundled emoji font (or unbundled variant)
│   ├── icons/              SVG sources for the bundled icons
│   └── font/               Bundled TTF binaries
│
├── test/                   Public test harness: NewApp, NewWindow, Tap, Type,
│                              AssertRendersToImage, AssertRendersToMarkup
│
├── tools/                  Developer tools (probably moved to fyne.io/tools upstream)
│
├── widget/                 Stock widgets — see canvas-objects.md for the catalog
│
└── cmd/
    ├── fyne/               The fyne CLI (package, install, release, serve, ...)
    ├── fyne_demo/          Widget showcase app (also serves as integration test)
    ├── fyne_settings/      Global Fyne settings GUI
    └── hello/              Minimal example app
```

## Module identity

```
module fyne.io/fyne/v2
go 1.19
```

Major direct dependencies (from `go.mod`):

| Module | Purpose |
|---|---|
| `fyne.io/systray` | System tray integration |
| `github.com/go-gl/gl` + `github.com/go-gl/glfw/v3.3/glfw` | OpenGL + windowing on desktop |
| `github.com/fyne-io/gl-js` | WebGL bridge for WASM |
| `github.com/fyne-io/glfw-js` | GLFW shim for WASM |
| `github.com/fyne-io/oksvg`, `github.com/srwiley/rasterx` | SVG rendering |
| `github.com/go-text/render` + `github.com/go-text/typesetting` | Text shaping |
| `github.com/anthonynsimon/bild` | Image processing |
| `github.com/fogleman/gg` | 2D graphics helpers |
| `github.com/nfnt/resize` | Image resizing |
| `github.com/fyne-io/image` | Image utilities |
| `github.com/fsnotify/fsnotify` | File-system events |
| `github.com/godbus/dbus/v5` | D-Bus on Linux |
| `github.com/rymdport/portal` | XDG portals (sandboxes) |
| `github.com/yuin/goldmark` | Markdown parser for widget.Markdown |
| `github.com/nicksnyder/go-i18n/v2` | Translations |
| `github.com/jeandeaual/go-locale` | OS locale detection |
| `github.com/hack-pad/go-indexeddb` | Browser storage backend |
| `github.com/BurntSushi/toml` | TOML parsing for FyneApp.toml + lang files |
| `github.com/urfave/cli/v2` | CLI framework (fyne tool) |
| `github.com/josephspurrier/goversioninfo`, `github.com/jackmordaunt/icns/v2`, `github.com/akavel/rsrc` | Windows / macOS resource bundling |
| `github.com/natefinch/atomic` | Atomic file writes |
| `golang.org/x/{image,mod,sys,text,tools}` | Standard extensions |
| `fyne.io/systray` | Tray icon |

Internal-only deps in `internal/` are not part of the public API surface; depending on them from outside the module is unsupported.
