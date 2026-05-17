# Tauri shell (Rust crate)

The Rust side at [tauri-orc-src/src-tauri/](../../tauri-orc-src/src-tauri/) is a thin Tauri 2 app whose only job is to host the WebView and manage the Python marker sidecar.

Crate name: `src` (library `src_lib`). Bundle identifier: `com.nelson.orc-works`. Window title: "src", 800×600. (The placeholder names are noted in [gaps.md](gaps.md).)

| File | Role |
| --- | --- |
| [src/main.rs](../../tauri-orc-src/src-tauri/src/main.rs) | Calls `src_lib::run()`. `windows_subsystem = "windows"` in release builds. |
| [src/lib.rs](../../tauri-orc-src/src-tauri/src/lib.rs) | The actual app. See IPC commands below. |
| [Cargo.toml](../../tauri-orc-src/src-tauri/Cargo.toml) | Deps: `tauri` v2, `tauri-plugin-opener` v2, `serde`, `serde_json`. |
| [tauri.conf.json](../../tauri-orc-src/src-tauri/tauri.conf.json) | `beforeDevCommand: npm run dev`, devUrl `http://localhost:1420`, `frontendDist: ../dist`. CSP is `null`. |
| [capabilities/default.json](../../tauri-orc-src/src-tauri/capabilities/default.json) | Permissions: `core:default`, `opener:default` (main window only). |
| [build.rs](../../tauri-orc-src/src-tauri/build.rs) | `tauri_build::build()`. |
| [icons/](../../tauri-orc-src/src-tauri/icons/) | Stock Tauri icon set (png/ico/icns, plus Windows Store logos). |

## IPC commands

Four `#[tauri::command]`s, registered in `invoke_handler!`:

- **`marker_server_url()`** → `"http://127.0.0.1:7423"` (port constant `MARKER_PORT = 7423`).
- **`marker_status()`** → `"stopped" | "running" | "error: ..."`. Uses `Child::try_wait` to detect crashes; clears state if the child has exited.
- **`marker_start()`** → spawns `marker-code/venv/Scripts/python.exe -m uvicorn server:app --host 127.0.0.1 --port 7423` with cwd = `marker-code/`. Validates that python.exe and `server.py` exist first. Pipes stdout/stderr. Re-entrant: if already running, returns `"already running on port 7423"`.
- **`marker_stop()`** → kills the child and waits.

## State and lifecycle

```rust
struct MarkerState { child: Mutex<Option<Child>> }
```

Managed via `app.manage(...)`. On `RunEvent::ExitRequested` and `RunEvent::Exit` the app kills the marker child so it does not leak across app shutdown.

`app_root()` resolves `CARGO_MANIFEST_DIR/..` — i.e. the `tauri-orc-src/` folder — and is the base for the python.exe and `server.py` path lookups. The python path is hardcoded to the Windows venv layout (`venv/Scripts/python.exe`); there is no macOS/Linux branch (see [gaps.md](gaps.md)).
