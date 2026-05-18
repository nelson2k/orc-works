# fyne — notes

Source: `repos-folder/fyne` (fyne-io). Go module `fyne.io/fyne/v2`. Go 1.19 minimum (declared in `go.mod`). Upstream: <https://github.com/fyne-io/fyne>.

What it is: a cross-platform GUI toolkit and application API written in Go. One codebase compiles to native binaries for Windows, macOS, Linux, BSD, iOS, Android, and web (via WebAssembly). It does not embed a browser — rendering goes through OpenGL (`go-gl/gl` + GLFW on desktop) or platform-native canvases on mobile/web.

The pitch in one sentence: write a Go program that calls `app.New()`, `NewWindow(...)`, and `SetContent(...)` with widgets / containers, and it runs everywhere.

Shape:

```
fyne/                       core interfaces (App, Window, Canvas, CanvasObject, Driver, ...)
├── app/                    default App implementation per platform (uses driver/glfw on desktop)
├── canvas/                 primitive graphical objects: Text, Image, Rectangle, Circle, ...
├── container/              high-level containers (Tabs, Split, Scroll, InnerWindow, ...)
├── data/binding/           reactive data bindings for widgets
├── data/validation/        common form validators (regex, time)
├── dialog/                 stock dialogs (file, folder, color picker, confirm, form, ...)
├── driver/                 driver interfaces for desktop / mobile / software / embedded
├── layout/                 box / grid / form / border / stack layouts
├── lang/                   i18n / localization
├── storage/                URI-based file abstraction with pluggable repositories
├── theme/                  bundled themes, colors, sizes, fonts, icons
├── widget/                 stock widgets: Button, Entry, List, Tree, Table, Form, RichText, ...
├── test/                   in-memory test helpers
├── tools/                  developer utilities
├── cmd/fyne                the `fyne` CLI (package, install, release, serve)
├── cmd/fyne_demo           the showcase app shipped with the toolkit
├── cmd/fyne_settings       GUI for global Fyne settings
└── cmd/hello               minimal example
```

Three idiomatic application-level concepts:

- **`fyne.App`** — process-singleton; owns the driver, settings, preferences, storage, clipboard, cache, lifecycle, cloud provider. Created via `app.New()` or `app.NewWithID(id)`.
- **`fyne.Window`** — a top-level OS window. Apps have one or more; closing the "master" exits the app by default.
- **`fyne.CanvasObject`** — anything that can be drawn. Implemented by every widget, container, and canvas primitive. The minimum contract: `MinSize`, `Move`/`Position`, `Resize`/`Size`, `Show`/`Hide`/`Visible`, `Refresh`.

Plus a set of small **behavior interfaces** that any `CanvasObject` may opt into: `Tappable`, `SecondaryTappable`, `DoubleTappable`, `Draggable`, `Focusable`, `Scrollable`, `Shortcutable`, `Disableable`, `Tabbable` — fyne checks for these at runtime to dispatch events.

A separate `Widget` interface extends `CanvasObject` with `CreateRenderer() WidgetRenderer` so widgets can encapsulate child objects and re-render declaratively.

License: BSD-3-Clause (`LICENSE`). Fyne is part of the FyshOS organization; sister projects include FyneDesk (a Linux desktop environment) and `fyne.io/cloud` (cloud-sync providers).
