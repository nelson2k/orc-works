# Tauri — Notes

Local-source notes for [`repos-folder/tauri`](../../repos-folder/tauri). Tauri is a Rust framework for building tiny, self-updating desktop and mobile apps that render their UI in the OS-provided WebView and expose a Rust backend over an IPC bridge.

Source upstream: <https://github.com/tauri-apps/tauri> (v2 line). This snapshot's workspace declares `rust-version = "1.77.2"` and Apache-2.0 OR MIT.

## Index

- [overview.md](overview.md) — what Tauri is, what it isn't, supported platforms
- [architecture.md](architecture.md) — the workspace crates and how they fit together
- [cli.md](cli.md) — `tauri` / `cargo tauri` subcommands and what they do
- [config.md](config.md) — `tauri.conf.json` shape, supported formats, schema location
- [commands_ipc.md](commands_ipc.md) — `#[tauri::command]`, `invoke()`, events, channels, ACL
- [plugins.md](plugins.md) — how plugins are authored (Rust + JS + permissions)
- [bundler.md](bundler.md) — what `tauri-bundler` produces per platform
- [js_api.md](js_api.md) — `@tauri-apps/api` surface
- [examples.md](examples.md) — what each example in `examples/` demonstrates
- [build_release.md](build_release.md) — dev loop, release build, updater flow

## Why these notes exist

The OCR-works project may eventually want a desktop shell. These notes capture the parts of Tauri that look load-bearing for that decision — the IPC model, bundler outputs, and what the v2 mobile story looks like — without re-reading the whole repo each time.
