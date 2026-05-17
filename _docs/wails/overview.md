# Overview

Wails is "Electron, but Go instead of Node and native WebView instead of bundled Chromium." A single Go binary embeds the web assets via `embed.FS` and renders them in the OS-provided WebView (WebView2 on Windows, WKWebView on macOS / iOS, WebKitGTK on Linux, Android WebView on Android).

The pitch from the README:

- Standard Go for the backend
- Any frontend tech (React, Svelte, Vue, plain HTML — your call)
- Native rendering — no embedded browser
- Auto-generated TypeScript/JS definitions for Go structs and methods
- Native dialogs, menus, dark/light mode, translucency
- Unified eventing between Go and JS

## v2 vs v3

| | v2 (stable) | v3 (alpha) |
| --- | --- | --- |
| Install | `go install github.com/wailsapp/wails/v2/cmd/wails@latest` | `go install github.com/wailsapp/wails/v3/cmd/wails3@latest` |
| API entry | `wails.Run(opts)` | `application.New(opts).Run()` — multi-window, services as a first-class concept |
| IPC | `runtime.EventsOn/Emit` + bound structs | `app.Event.On/Emit` + `application.NewService(&T{})` services |
| Mobile | No | iOS (WKWebView + CGO) and Android (WebView + JNI) — see [mobile.md](mobile.md) |
| Status | Production-ready | Alpha; expect breaking changes |

The v3 docs live in [`repos-folder/wails/v3/`](../../repos-folder/wails/v3/); the v2 source still lives in [`repos-folder/wails/v2/`](../../repos-folder/wails/v2/).

## Supported platforms

- **Windows** — WebView2 (Edge Chromium). Auto-installed on Windows 11; bootstrapped on Win 10.
- **macOS** — WKWebView (Safari engine).
- **Linux** — WebKitGTK 4.0 / 4.1 (system package, e.g. `libwebkit2gtk-4.0-dev`).
- **iOS (v3 alpha)** — WKWebView wrapped in `WailsViewController`. CGO bridges to Objective-C. See [`repos-folder/wails/v3/IOS_ARCHITECTURE.md`](../../repos-folder/wails/v3/IOS_ARCHITECTURE.md).
- **Android (v3 alpha)** — Native WebView via JNI. Go compiled as `-buildmode=c-shared` into a `.so`. See [`repos-folder/wails/v3/ANDROID_ARCHITECTURE.md`](../../repos-folder/wails/v3/ANDROID_ARCHITECTURE.md).

## What it isn't

- Not Electron — no embedded Chromium, different WebView per platform (rendering differences are real).
- Not a browser. Asset serving is in-process (no HTTP server listening on a port by default — v3 explicitly avoids that for battery on mobile).
- Not "just a JS framework". The backend is Go; the frontend talks to Go services through the auto-generated bindings, not to an HTTP API.
