# Known gaps and rough edges

Live punch-list of mismatches and rough edges in [tauri-orc-src/](../../tauri-orc-src/), kept current as fixes land.

## Server-side

- **No artifact URLs.** [server.py](../../tauri-orc-src/marker-code/server.py) returns `{ markdown }` only — no persisted `markdownUrl` / `metadataUrl` for download, just inline text. The pre-Tauri `backend/` had these; the sidecar does not.
- **`local.env` is not loaded.** [server.py](../../tauri-orc-src/marker-code/server.py) does not import dotenv or read it, so `OPENAI_API_KEY` and `TORCH_DEVICE` have no effect today. Either `os.environ` them before launching uvicorn, or `load_dotenv("local.env")` at module top.
- **First `/convert` has no progress feedback.** `get_artifact_dict()` lazy-loads ~3 GB of surya weights on the first conversion, not at uvicorn boot. The UI just shows "Converting..." through the whole download + model-load wait. Either eager-load at boot (so `serverStatus` covers it) or stream a status event back through Tauri.

## Rust shell

- **Child stdout/stderr pipes can deadlock.** [lib.rs](../../tauri-orc-src/src-tauri/src/lib.rs) spawns marker with `Stdio::piped()` on both streams but nothing drains them. Once the OS pipe buffer (~64 KB on Windows) fills with marker progress logs, the child blocks on `write` and the server hangs. Use `Stdio::inherit()` or spawn reader threads.
- **Windows-only paths.** [lib.rs](../../tauri-orc-src/src-tauri/src/lib.rs) hardcodes `venv/Scripts/python.exe`; no macOS/Linux branch. Build the path with `if cfg!(windows)` or check both `Scripts/` and `bin/`.

## Naming / cosmetic

- **Product naming is the Tauri template default.** [Cargo.toml](../../tauri-orc-src/src-tauri/Cargo.toml) crate name is `src` (lib `src_lib`); [tauri.conf.json](../../tauri-orc-src/src-tauri/tauri.conf.json) `productName` and window title are both `"src"`; [package.json](../../tauri-orc-src/package.json) name is `"src"`. The bundle id is `com.nelson.orc-works`, so the user-facing identity is mixed.
- **Leftover template files.** [App.css](../../tauri-orc-src/src/App.css), [public/tauri.svg](../../tauri-orc-src/public/tauri.svg), [public/vite.svg](../../tauri-orc-src/public/vite.svg), and [src/assets/react.svg](../../tauri-orc-src/src/assets/react.svg) are not referenced by the app.
