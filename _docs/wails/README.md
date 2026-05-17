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
- [vs-tauri.md](vs-tauri.md) — comparison against Tauri for this project's context

## Why these notes exist

The OCR-works project picked Tauri, but Wails is the closest alternative (same single-binary + native-WebView design, swap Rust for Go). These notes capture the parts of Wails that would matter if we ever revisit that decision — bindings ergonomics, mobile story, and the built-in services that don't exist as first-class concepts in Tauri.
