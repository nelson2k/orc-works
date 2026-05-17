# tauri-orc-src

Snapshot of what lives in the repo's top-level [src/](../../src/) folder — the Tauri desktop app that wraps the marker OCR pipeline.

The folder is a standard Tauri 2 scaffold (React + Vite + TypeScript on the frontend, Rust on the backend) plus a `marker-code/` sidecar that runs the Python marker server. Note the unusual nesting: the project root is named `src/`, and the Tauri scaffold places the React app under `src/src/` and the Rust crate under `src/src-tauri/`.

## Layout

```
src/
├── .vscode/extensions.json           VS Code recommendations
├── .gitignore                        node_modules, dist, logs, editor cruft
├── README.md                         stock Tauri+React+TS template blurb
├── index.html                        Vite entry; mounts /src/main.tsx
├── package.json                      npm scripts + deps
├── tsconfig.json                     strict TS, react-jsx, bundler resolution
├── tsconfig.node.json                node-side tsconfig
├── vite.config.ts                    Vite dev server on port 1420 (Tauri-fixed)
├── public/                           tauri.svg, vite.svg static assets
├── node_modules/                     (gitignored)
├── src/                              React frontend
├── src-tauri/                        Rust Tauri backend
└── marker-code/                      Python marker server sidecar
```

## Frontend — [src/src/](../../src/src/)

React 19 + TypeScript. Entry: [main.tsx](../../src/src/main.tsx) mounts `<App />` into `#root`.

| File | Role |
| --- | --- |
| [App.tsx](../../src/src/App.tsx) | Top-level form: PDF picker, page range, OpenAI model dropdown (8 options from `gpt-4o-mini` to `gpt-5`), `no_llm` + `full_vram` checkboxes, submit/stop buttons. On mount calls `startMarker()` + `waitForReady()` and shows a status pill; on unmount calls `stopMarker()`. Submit routes through `convertPdf()` (which POSTs to the spawned marker server on `:7423`). Renders `documentName` (the file name) + the returned `markdown`. |
| [PdfFilePicker.tsx](../../src/src/PdfFilePicker.tsx) | Hidden `<input type=file>` triggered by a styled button; renders a first-page thumbnail via pdfjs-dist into a `<canvas>`. Cancels in-flight render tasks on file change. |
| [PdfViewer.tsx](../../src/src/PdfViewer.tsx) | Larger pdfjs preview with prev/next page controls, ctrl-+/-/0 zoom (0.4–3×), and space-to-pan drag on the canvas stage. Auto-fits page width to viewport, capped at 820px or 1.5× scale. |
| [markerClient.ts](../../src/src/markerClient.ts) | Thin wrapper over Tauri `invoke()` for `marker_start` / `marker_stop` / `marker_status` / `marker_server_url`, plus `waitForReady()` (polls `/health`) and `convertPdf(file, options)` (POSTs file + `page_range` / `model` / `no_llm` / `full_vram` to `/convert`, with `AbortSignal` support). |
| [styles.css](../../src/src/styles.css) | Dark-theme stylesheet (`#080c14` bg, `#5787ff` accents). The actual styles in use. |
| [App.css](../../src/src/App.css) | Leftover template styles (Vite/React logo hovers); not imported by `App.tsx`. |
| [vite-env.d.ts](../../src/src/vite-env.d.ts) | Vite client types. |
| [assets/react.svg](../../src/src/assets/react.svg) | Unused template asset. |

## Rust backend — [src/src-tauri/](../../src/src-tauri/)

Crate name: `src` (library `src_lib`). Bundle identifier: `com.nelson.orc-works`. Window title: "src", 800×600.

| File | Role |
| --- | --- |
| [src/main.rs](../../src/src-tauri/src/main.rs) | Calls `src_lib::run()`. `windows_subsystem = "windows"` in release. |
| [src/lib.rs](../../src/src-tauri/src/lib.rs) | The actual app. See below. |
| [Cargo.toml](../../src/src-tauri/Cargo.toml) | Deps: `tauri` v2, `tauri-plugin-opener` v2, `serde`, `serde_json`. |
| [tauri.conf.json](../../src/src-tauri/tauri.conf.json) | `beforeDevCommand: npm run dev`, devUrl `http://localhost:1420`, `frontendDist: ../dist`. CSP is `null`. |
| [capabilities/default.json](../../src/src-tauri/capabilities/default.json) | Permissions: `core:default`, `opener:default` (main window only). |
| [build.rs](../../src/src-tauri/build.rs) | `tauri_build::build()`. |
| [icons/](../../src/src-tauri/icons/) | Stock Tauri icon set (png/ico/icns, plus Windows Store logos). |

### lib.rs — marker sidecar management

Exposes four Tauri commands (registered in `invoke_handler!`):

- `marker_server_url()` → `"http://127.0.0.1:7423"` (port constant `MARKER_PORT = 7423`).
- `marker_status()` → `"stopped" | "running" | "error: ..."` (uses `Child::try_wait` to detect crashes; clears state if the child exited).
- `marker_start()` → spawns `marker-code/venv/Scripts/python.exe -m uvicorn server:app --host 127.0.0.1 --port 7423` with cwd = `marker-code/`. Validates that python.exe and `server.py` exist first. Pipes stdout/stderr. Re-entrant: if already running, returns `"already running on port 7423"`.
- `marker_stop()` → kills the child and waits.

`MarkerState { child: Mutex<Option<Child>> }` is managed via `app.manage(...)`. On `RunEvent::ExitRequested` / `RunEvent::Exit`, the app kills the marker child so it doesn't leak across app shutdown.

The python path is hardcoded to the Windows venv layout (`venv/Scripts/python.exe`); there is no macOS/Linux branch. `app_root()` resolves `CARGO_MANIFEST_DIR/..` (= the top-level `src/` folder).

## Python sidecar — [src/marker-code/](../../src/marker-code/)

FastAPI server that wraps `marker.converters.pdf.PdfConverter`.

| File | Role |
| --- | --- |
| [server.py](../../src/marker-code/server.py) | Two routes: `GET /health` → `{"status": "ok"}`, and `POST /convert` accepting an `UploadFile`, writing it to a temp `.pdf`, running it through `PdfConverter` (lazy-initialized via `create_model_dict()`), and returning `{"markdown": <text>}`. The converter is cached in a module-global `_converter`. |
| [requirements-server.txt](../../src/marker-code/requirements-server.txt) | `fastapi>=0.110`, `uvicorn[standard]>=0.27`, `python-multipart>=0.0.9`. `marker` itself is installed separately into the venv. |
| [local.env](../../src/marker-code/local.env) | `OPENAI_API_KEY=...` and `TORCH_DEVICE=cuda`. **Contains a real-looking API key — should be gitignored.** Not currently loaded by `server.py`. |
| `venv/` | Local Python virtualenv (see [venv_layout memory](../../../../.claude/projects/c--Users-nelso-dev-017-ocr-works/memory/venv_layout.md) — lives at `marker-code/venv/`, not `.venv`). Holds marker + its deps. |

`server.py` exposes a minimal subset of marker. It does not yet honor `page_range`, `model`, `no_llm`, or `full_vram` — all of which the React form already sends. It also doesn't write `markdownUrl`/`metadataUrl` artifacts that `App.tsx` expects.

## Build + run flow

1. `npm run tauri dev` (from `src/`) triggers `beforeDevCommand: npm run dev` → Vite on port 1420 → Tauri opens a window pointed at `http://localhost:1420`.
2. On `App` mount, the frontend calls `marker_start` (via `markerClient.ts`) to spawn `uvicorn server:app` on 127.0.0.1:7423, polls `/health` until ready, and gates the submit button on `serverStatus === "ready"`.
3. Conversion: frontend POSTs the PDF to the marker server, which runs marker and returns markdown.
4. On window close, the Rust exit handler kills the python child.

Production build: `npm run build` (= `tsc && vite build` → `dist/`), then `cargo tauri build` packages with `frontendDist: ../dist`.

## Known gaps / mismatches to be aware of

- **Server ignores most form fields.** `page_range`, `model`, `no_llm`, `full_vram` are sent but not consumed by `server.py`.
- **No artifact URLs.** `server.py` returns `{ markdown }` only — no persisted `markdownUrl` / `metadataUrl` for download, just inline text.
- **`local.env` is committed and contains a real-shaped OpenAI key.** Should be rotated and added to `.gitignore`.
- **`local.env` is not loaded.** `server.py` does not import dotenv or read it; the OpenAI key has no effect today.
- **Windows-only paths.** `lib.rs` hardcodes `venv/Scripts/python.exe`; no cross-platform branch.
- **Product naming.** `productName`, crate name, and window title are all the placeholder `"src"`; the bundle id is `com.nelson.orc-works`.
- **Leftover template files.** `App.css`, `public/tauri.svg`, `public/vite.svg`, `src/assets/react.svg` are not referenced by the app.
