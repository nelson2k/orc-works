# Frontend

React 19 + TypeScript at [tauri-orc-src/src/](../../tauri-orc-src/src/). Entry: [main.tsx](../../tauri-orc-src/src/main.tsx) mounts `<App />` into `#root` and imports `./styles.css`.

| File | Role |
| --- | --- |
| [main.tsx](../../tauri-orc-src/src/main.tsx) | React entrypoint; imports the global stylesheet. |
| [App.tsx](../../tauri-orc-src/src/App.tsx) | Top-level form: PDF picker, page range, OpenAI model dropdown (8 options from `gpt-4o-mini` to `gpt-5`), `no_llm` + `full_vram` checkboxes, submit/stop buttons. On mount calls `startMarker()` + `waitForReady()` and shows a `.server-status` pill; on unmount calls `stopMarker()`. Submit routes through `convertPdf()` against the spawned sidecar. Renders the file name + the returned `markdown`. |
| [PdfFilePicker.tsx](../../tauri-orc-src/src/PdfFilePicker.tsx) | Hidden `<input type=file>` triggered by a styled button; renders a first-page thumbnail via pdfjs-dist into a `<canvas>`. Cancels in-flight render tasks on file change. |
| [PdfViewer.tsx](../../tauri-orc-src/src/PdfViewer.tsx) | Larger pdfjs preview with prev/next page controls, ctrl-+/-/0 zoom (0.4–3×), and space-to-pan drag on the canvas stage. Auto-fits page width to viewport, capped at 820px or 1.5× scale. |
| [markerClient.ts](../../tauri-orc-src/src/markerClient.ts) | Thin wrapper over Tauri `invoke()` for `marker_start` / `marker_stop` / `marker_status` / `marker_server_url`, plus `waitForReady()` (polls `/health`) and `convertPdf(file, options)` (POSTs file + `page_range` / `model` / `no_llm` / `full_vram` to `/convert`, with `AbortSignal` support). |
| [styles.css](../../tauri-orc-src/src/styles.css) | Dark-theme stylesheet (`#080c14` bg, `#5787ff` accents). Imported by `main.tsx`. |
| [App.css](../../tauri-orc-src/src/App.css) | Leftover template styles (Vite/React logo hovers); not imported anywhere. |
| [vite-env.d.ts](../../tauri-orc-src/src/vite-env.d.ts) | Vite client types. |
| [assets/react.svg](../../tauri-orc-src/src/assets/react.svg) | Unused template asset. |

## Convert flow

`App.tsx` keeps a single `AbortController` so the user's "Stop" button cancels the in-flight `fetch()`. The submit handler is gated on `serverStatus === "ready"`, which is set by the `useEffect` that boots the sidecar. Errors raised by `convertPdf()` are surfaced inline via the `.error` block.

`PdfFilePicker.tsx` and `PdfViewer.tsx` each own their pdfjs `RenderTask` ref and cancel it on cleanup or when the source file changes.

## Server-status pill

`App.tsx` exposes `serverStatus: "starting" | "ready" | "error"` via the `.server-status` element in the toolbar. The three CSS classes in [styles.css](../../tauri-orc-src/src/styles.css) (`-starting`, `-ready`, `-error`) give the pill its blue / green / red look.

## Stylesheet routing

Only [styles.css](../../tauri-orc-src/src/styles.css) is live. [App.css](../../tauri-orc-src/src/App.css) is template debris and is not imported anywhere.
