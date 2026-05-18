# src/worker.py

Single-process Python worker driven by the Go GUI over stdio. Reads one JSON object per `\n`-terminated line from stdin, writes one JSON object per line to stdout (flushed).

## Commands

- `render` — `{cmd, path, page, dpi=120}` → `{type: "image", page, pages, png_base64}`. Opens the PDF with `pymupdf.open`, calls `get_pixmap(dpi=...)`, returns the page as base64-encoded PNG. Also returns total page count.
- `ocr` — `{cmd, path, page}` → `{type: "text", page, text}`. Runs marker's `PdfConverter` with `config={"page_range": [page]}` and returns the rendered markdown via `text_from_rendered`.
- `quit` — exits the worker loop.

## Marker model loading

`_ensure_marker()` lazy-loads the surya bundle (layout, recognition, table-rec, detection, OCR-error) via `marker.models.create_model_dict()` on the first OCR request and caches the dict in the module global `_marker_models`. So:

- Startup is fast — no model load on launch.
- First OCR click is slow — models load (and possibly download to `~/.cache/datalab`).
- Subsequent OCRs reuse the cached models. A new `PdfConverter` is built per call so the `page_range` is fresh, but that's cheap once models are loaded.

## Errors

Every command runs inside a try/except that emits `{type: "error", message, traceback}` instead of crashing the worker. Out-of-range pages on `render` give a structured `page N out of range (doc has M pages)`; JSON parse errors give `bad json: ...`. The worker stays alive across errors.

The PyMuPDF doc is opened and closed per `render` call. Marker manages its own provider lifecycle.
