# src/worker.py

Single-process Python worker driven by the Go GUI over stdio. Reads one JSON object per `\n`-terminated line from stdin, writes one JSON object per line to stdout (flushed).

## Commands

- `render` — `{cmd, path, page, dpi=120}` → `{type: "image", page, pages, png_base64}`. Opens the PDF with `pymupdf.open`, calls `get_pixmap(dpi=...)`, returns the page as base64-encoded PNG. Also returns total page count.
- `ocr` — `{cmd, path, page}` → 0..N `{type: "progress", ...}` lines, then one final `{type: "text", page, text}` line. Each request always ends with exactly one non-progress reply.
- `quit` — exits the worker loop.

## Progress protocol

Two `kind`s emitted from `ocr`:

- **Stage events** — `{type:"progress", kind:"stage", name:"layout"}`. Sent manually from `ocr()` between major pipeline steps. Names: `loading_models` (first call only), `init_converter`, `open_pdf`, `rasterize`, `layout`, `line_detection`, `ocr_recognition`, `structure`, `processor:<ClassName>` (one per processor in `default_processors`), `render`.
- **tqdm events** — `{type:"progress", kind:"tqdm", event:"start"|"tick"|"end", desc, n, total}`. Generated automatically by a subclass of `tqdm.tqdm` that gets installed at module-import time. Marker and surya use `from tqdm import tqdm`, so once we set `tqdm.tqdm = _EventTqdm` (and `tqdm.auto.tqdm`) before importing marker, every internal progress bar in the pipeline becomes a stream of JSON events instead of stderr drawing. The actual bar is silenced by routing `file=` to `os.devnull`.

`send()` is wrapped in a `threading.Lock` so events from surya's worker threads can't interleave their `\n`-terminated lines.

## Marker model loading

`_ensure_marker()` lazy-loads the surya bundle (layout, recognition, table-rec, detection, OCR-error) via `marker.models.create_model_dict()` on the first OCR request and caches the dict in the module global `_marker_models`. So:

- Startup is fast — no model load on launch.
- First OCR click is slow — models load (and possibly download to `~/.cache/datalab`). The tqdm bars from the downloader/loader stream through as `progress` events so the GUI can show motion during this phase.
- Subsequent OCRs reuse the cached models. A new `PdfConverter` is built per call so the `page_range` is fresh, but that's cheap once models are loaded.

## Why the pipeline is hand-unrolled

`ocr()` does not call `PdfConverter.__call__` directly. It replicates what that method does — `provider_from_filepath` → `DocumentBuilder.build_document` → `layout_builder` → `line_builder` → `ocr_builder` → `StructureBuilder` → each processor → `renderer` — so it can emit a stage event before each step. This is "approach #1" we picked over forking marker: zero changes inside `repos-folder/marker`, the public API is enough.

## Errors

Every command runs inside a try/except that emits `{type: "error", message, traceback}` instead of crashing the worker. Out-of-range pages on `render` give a structured `page N out of range (doc has M pages)`; JSON parse errors give `bad json: ...`. The worker stays alive across errors.

The PyMuPDF doc is opened and closed per `render` call. Marker manages its own provider lifecycle.
