# Architecture

Wails apps are a single Go process that owns:

1. An OS window with a native WebView.
2. An in-process asset server (no listening port — by default).
3. A binding bridge so JS in the WebView can call Go methods.
4. An event bus that flows both directions (Go → JS, JS → Go, OS → app).

## Source layout (v3)

```
v3/
├── cmd/wails3/          The wails3 CLI (init / dev / build / package / doctor / task)
├── pkg/
│   ├── application/     The Options struct, App, Window, Event, Service abstractions
│   │   ├── application.go           — application.New(), Run(), global App
│   │   ├── application_options.go   — top-level Options
│   │   ├── application_windows.go   — Windows backend
│   │   ├── application_darwin.go    — macOS backend
│   │   ├── application_linux*.go    — Linux (GTK3) backend
│   │   ├── application_ios*.go      — iOS backend
│   │   └── application_android*.go  — Android backend
│   ├── events/          Strongly-typed Go event identifiers (Common.*, Mac.*, etc.)
│   ├── services/        Plug-in services (sqlite, kvstore, fileserver, notifications, dock, log)
│   ├── icons/           Default icons
│   ├── mac/             macOS helpers (NSWorkspace, accessibility, etc.)
│   └── w32/             Win32 helpers (system tray, COM, etc.)
├── internal/            Asset server, codegen for bindings, build commands, etc.
└── examples/            60+ runnable demos (binding, events, dialogs, drag-n-drop, …)
```

## Process model

A v3 app's `main()` reads roughly:

```go
app := application.New(application.Options{
    Name: "My App",
    Services: []application.Service{
        application.NewService(&GreetService{}),
    },
    Assets: application.AssetOptions{
        Handler: application.BundledAssetFileServer(assets),
    },
})

app.Window.NewWithOptions(application.WebviewWindowOptions{ URL: "/" })

app.Run()
```

`application.New` returns a singleton `*App`. `app.Run()` blocks until the OS tells the app to exit (last window closed, OS signal, etc.). All OS interaction (windows, menus, dialogs, tray) happens on the main OS thread, marshalled via the application package.

## Asset serving

`AssetOptions.Handler` is anything implementing `http.Handler` — usually `BundledAssetFileServer(embed.FS)`. The WebView gets a fake host (Windows: `wails.localhost`, macOS: `wails`, Linux: `wails`) that maps to in-memory dispatching. **No TCP port is opened.** This matters for the mobile backends (iOS/Android both avoid listening sockets entirely for battery and sandboxing reasons).

You can also point it at a real `http.Handler` to embed Gin / chi / standard `net/http` — see the `gin-example` and `gin-routing` examples in [`repos-folder/wails/v3/examples/`](../../repos-folder/wails/v3/examples/).

## Lifecycle

The app has well-defined events you can hook (`events.Common.ApplicationStarted`, `events.Common.ThemeChanged`, `events.Common.WindowFocus`, etc.) — see [events.md](events.md). Hooks can cancel events (e.g. a `WindowClosing` hook can prevent the window from closing); listeners fire after the action.

`OnShutdown` in `Options` blocks the shutdown process so cleanup can run.
