# tauri-orc-src

Notes on the Tauri desktop app at [tauri-orc-src/](../../tauri-orc-src/), which wraps the marker OCR pipeline behind a React UI.

Three logical pieces under one folder:

- A React + TypeScript frontend (Vite, port 1420).
- A Rust Tauri shell that owns the OS window and spawns the Python sidecar.
- A Python FastAPI sidecar that wraps marker's `PdfConverter` (port 7423).

## Index

- [overview.md](overview.md) — folder tree, the three pieces, how they talk
- [frontend.md](frontend.md) — React app at `tauri-orc-src/src/`
- [tauri-shell.md](tauri-shell.md) — Rust crate at `tauri-orc-src/src-tauri/` + IPC commands
- [marker-sidecar.md](marker-sidecar.md) — Python server at `tauri-orc-src/marker-code/`
- [build-run.md](build-run.md) — dev loop and production build
- [gaps.md](gaps.md) — known mismatches and rough edges
