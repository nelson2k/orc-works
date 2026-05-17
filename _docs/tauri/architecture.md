# Architecture — workspace crates

The repo is a Cargo workspace (`Cargo.toml` lists all members) plus two pnpm packages. Roles below pulled from `ARCHITECTURE.md` and crate sources.

## Tauri Core (Rust)

| Crate | Path | Role |
| --- | --- | --- |
| `tauri` | `crates/tauri` | The headline crate. Pulls runtime, macros, utils, API together. Reads `tauri.conf.json` at compile time, handles script injection, hosts the systems API, manages updating. |
| `tauri-build` | `crates/tauri-build` | Applies build-time macros so `cargo build` can wire in Tauri features. |
| `tauri-codegen` | `crates/tauri-codegen` | Embed/hash/compress assets + tray icons at compile time. Parses `tauri.conf.json` into the `Config` struct. |
| `tauri-macros` | `crates/tauri-macros` | Macros for context, handlers, commands; leverages `tauri-codegen`. |
| `tauri-runtime` | `crates/tauri-runtime` | Glue trait layer between `tauri` and the lower-level webview library. |
| `tauri-runtime-wry` | `crates/tauri-runtime-wry` | `tauri-runtime` implementation backed by WRY — printing, monitor detection, window plumbing. |
| `tauri-utils` | `crates/tauri-utils` | Shared parsing for configs, platform triples, CSP injection, asset helpers. |
| `tauri-plugin` | `crates/tauri-plugin` | Build-script + runtime definitions for authoring plugins. Feature-gated `build` / `runtime`. |

## Tooling

| Crate / Package | Path | Role |
| --- | --- | --- |
| `tauri-cli` | `crates/tauri-cli` | Rust executable behind `cargo tauri` / `tauri` — `init`, `dev`, `build`, `bundle`, `info`, `migrate`, `add`/`remove`, `icon`, `signer`, mobile (`android`/`ios`), ACL (`permission`/`capability`), `plugin`, `inspect`. |
| `tauri-bundler` | `crates/tauri-bundler` | Library that emits per-platform installers from a build artifact. Usable outside Tauri projects. |
| `tauri-driver` | `crates/tauri-driver` | WebDriver-style driver used for end-to-end tests. |
| `tauri-macos-sign` | `crates/tauri-macos-sign` | macOS code-signing helpers used by the bundler. |
| `tauri-schema-generator` | `crates/tauri-schema-generator` | Generates the JSON Schema for `tauri.conf.json` (shipped alongside the CLI). |
| `tauri-schema-worker` | `crates/tauri-schema-worker` | Worker variant of the schema generator. |
| `@tauri-apps/api` | `packages/api` | TS library that compiles to CJS + ESM; the frontend-side IPC client and event/menu/tray/window APIs. |
| `@tauri-apps/cli` | `packages/cli` | napi-rs wrapper around `tauri-cli`, distributed as platform-specific npm packages. |

## Upstream (own repos, not in this workspace)

- [`tao`](https://github.com/tauri-apps/tao) — window creation, fork of `winit`.
- [`wry`](https://github.com/tauri-apps/wry) — webview abstraction.
- [`create-tauri-app`](https://github.com/tauri-apps/create-tauri-app) — scaffolder behind `npm create tauri-app`.

## Module map inside `tauri` crate

From `crates/tauri/src/lib.rs`:

```
app/            event/          image/      ipc/        manager/
menu/           path            pattern.rs  plugin/     process.rs
protocol/       resources       scope/      state.rs    test/
tray/           vibrancy        webview/    window/
async_runtime.rs                ios.rs (target_os = "ios")
```

`menu` and `tray` are `desktop`-only. `ios` is `target_os = "ios"`-only.

## Cargo features (selected)

From the crate-level doc comment in `crates/tauri/src/lib.rs`:

- `wry` (default) — use WRY as runtime; disable for a custom runtime.
- `common-controls-v6` (default, Windows) — Common Controls v6 for the `about` menu item.
- `x11` / `dbus` (default, Linux) — toggle off for Wayland-only / no theme support.
- `tracing` — emit `tracing` spans for startup, plugins, eval, events, IPC, updater, custom protocols.
- `isolation` — Isolation pattern (auto-enabled when `app > security > pattern > use = "isolation"`).
- `custom-protocol` — CLI-managed flag; when on, Tauri assumes production mode.
- `devtools` — Web inspector; on by default in debug, gated behind private APIs on macOS so App Store apps must keep it off.
- `tray-icon`, `macos-private-api`, `compression`, `config-json5`, `config-toml`, `image-ico`, `image-png`, `specta`, `dynamic-acl` (default).
- TLS: `native-tls`, `native-tls-vendored`, `rustls-tls`.
