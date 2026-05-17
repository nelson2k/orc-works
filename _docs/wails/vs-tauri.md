# Wails vs Tauri — for this project

We picked Tauri for [tauri-orc-src/](../../tauri-orc-src/). Wails is the closest alternative — same single-binary, native-WebView design — with Go instead of Rust as the backend language. Where they differ:

## Language and toolchain

| | Tauri | Wails |
| --- | --- | --- |
| Backend lang | Rust | Go |
| CLI | `cargo tauri` (cargo subcommand) | `wails3` (standalone) |
| Build artifact | Rust binary + WebView | Go binary + WebView |
| Compile time | Slow (Rust + huge dep tree) | Fast (Go) |
| Cross-compile | Cross-rs / docker / GitHub Actions | `wails3 build -platform <os>/<arch>` |
| Windows install pain | MSVC toolchain required (we hit this) | None — Go just works |

The compile-time difference is the biggest day-to-day cost. Rebuilding the Rust shell in [tauri-orc-src/](../../tauri-orc-src/) after a dependency tweak can take minutes; a Wails Go rebuild is seconds.

## IPC

| | Tauri | Wails v3 |
| --- | --- | --- |
| Define a command | `#[tauri::command] fn x(...)` + register in `invoke_handler!` | Method on a struct registered as a service |
| Call from JS | `invoke("x", { args })` | `import { X } from "./bindings/.../service.js"; await X(args);` |
| Type generation | Manual or via `tauri-specta` | Built-in; runs as part of `wails3 generate bindings` |
| Cancellation | `AbortSignal` on `invoke()` | `CancellablePromise.cancel()` flows through to Go `context.Context` |

Wails' bindings ergonomics are noticeably tighter — you write idiomatic Go methods and get idiomatic JS modules. Tauri's `invoke("string-name", payload)` is more flexible but less type-safe by default.

## Built-in services

Wails ships SQLite, KV store, notifications, file server, and dock services in [`v3/pkg/services/`](../../repos-folder/wails/v3/pkg/services/) — see [services.md](services.md). Tauri leaves these to plugins (`tauri-plugin-sql`, `tauri-plugin-notification`). Both work; Wails is more "batteries included."

## Mobile

| | Tauri | Wails |
| --- | --- | --- |
| iOS / Android | Yes (since Tauri 2.0) | iOS + Android (v3 alpha) |
| Maturity | Stable, smaller ecosystem | Newer, fewer tested apps |

Both target iOS / Android with the same "native WebView + native bridge" approach. Tauri is more battle-tested on mobile today.

## Why we picked Tauri anyway

- Bigger ecosystem (plugins, examples, GitHub Actions templates).
- Better packaging story on Windows (NSIS + MSI + auto-updater + signed builds via tauri-cli).
- v2 is stable; Wails v3 is alpha.
- The Rust backend is friendlier for the kind of system-level child-process management this project does (the marker sidecar, future signed-process handling).

## If we ever revisit

The two things that would push us toward Wails:

1. **Compile time becomes a daily friction.** Rust rebuilds eat real minutes; Go rebuilds don't.
2. **We want SQLite + bindings without authoring a plugin.** Wails has it; Tauri makes you add `tauri-plugin-sql`.

Nothing about Tauri's choice is locking us in — both stacks have nearly the same "single binary + native WebView + JS frontend" shape, so a port is mostly a rewrite of [src-tauri/src/lib.rs](../../tauri-orc-src/src-tauri/src/lib.rs) into Go and regenerating the JS bindings layer.
