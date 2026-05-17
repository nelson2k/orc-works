# Wails — Notes

Local-source notes for [`repos-folder/wails`](../../repos-folder/wails). Wails is a Go framework for building desktop apps with a Go backend and a web-tech frontend (HTML/CSS/JS or React/Svelte/Vue/etc.), bundled into a single binary that renders in the OS-native WebView.

Source upstream: <https://github.com/wailsapp/wails>. Two active lines: **v2** (stable) and **v3** (alpha — adds iOS / Android support, a new app/window/service API, and a Go-typed event system).

## Index

- [overview.md](overview.md) — what Wails is, what it isn't, v2 vs v3, supported platforms
- [architecture.md](architecture.md) — Go process + native WebView, asset serving, app lifecycle
- [cli.md](cli.md) — `wails3` subcommands and project scaffolding
- [bindings.md](bindings.md) — exposing Go services to JS, auto-generated TS/JS bindings
- [events.md](events.md) — application events, window events, custom Go↔JS events
- [services.md](services.md) — built-in services (sqlite, kvstore, fileserver, notifications, dock, log)
- [mobile.md](mobile.md) — v3's iOS (WKWebView + CGO) and Android (WebView + JNI) story

## Why these notes exist

Wails is the candidate desktop-shell framework for this project: single Go binary, native WebView per platform, auto-generated typed JS bindings to Go services. These notes capture the parts that matter for OCR-works — bindings ergonomics, the mobile backends, and the built-in services (SQLite, notifications, KV store, file server) that ship in the framework rather than as third-party plugins.
