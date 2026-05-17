# Overview

## What it is

Tauri builds small native binaries that host an OS WebView as the UI and a Rust binary as the backend. The two halves talk over an IPC channel; the frontend is any HTML/CSS/JS framework that can compile to static assets.

- **Windowing**: [`tao`](https://docs.rs/tao) — a fork of `winit` extended for menus and tray.
- **WebView**: [`WRY`](https://github.com/tauri-apps/wry) — wraps WKWebView (macOS/iOS), WebView2 (Windows), WebKitGTK (Linux), Android System WebView.
- **No bundled runtime**: the final binary is a Rust executable. There is no Node.js or Electron-style Chromium ship.

## What it is NOT (from `ARCHITECTURE.md`)

- Not a lightweight kernel wrapper — it calls WRY/TAO directly.
- Not a VM or virtualized environment — it is an app toolkit, not a sandbox layer.

## Supported platforms

From the upstream README:

| Platform   | Versions                                                 |
| ---------- | -------------------------------------------------------- |
| Windows    | 7+                                                       |
| macOS      | 10.15+                                                   |
| Linux      | webkit2gtk 4.1 for v2 (e.g. Ubuntu 22.04)                |
| iOS/iPadOS | 9+                                                       |
| Android    | 8+                                                       |

Cross-compilation is not supported out of the box; the official [`tauri-action`](https://github.com/tauri-apps/tauri-action) GitHub workflow is the cross-platform escape hatch.

## Headline features

- App bundler producing `.app`, `.dmg`, `.deb`, `.rpm`, `.AppImage`, NSIS `.exe`, WiX `.msi`.
- Built-in self-updater (desktop only) driven by a server URL in config.
- System tray icons, native notifications, native menus.
- **Native WebView Protocol** — no local HTTP server is spun up to serve the UI in production.
- VS Code extension and a `tauri-action` GitHub workflow.

## License

Apache-2.0 OR MIT. Logo is CC-BY-NC-ND.
