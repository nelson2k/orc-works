# ocr-works/src documentation

In-repo notes on the `src/` tree of this project. Each file gets a
small markdown doc; this README is the index.

## What the app does

A Fyne desktop GUI that opens a PDF, lets the user page through it,
and runs marker-pdf OCR on a chosen page. The Go process owns the
window; a long-lived Python subprocess owns PyMuPDF rendering and the
ML pipeline. They talk over stdin/stdout in newline-delimited JSON.

## Start here

- [layout.md](layout.md) — directory tree and the two-process split
- [worker-protocol.md](worker-protocol.md) — JSON wire format

## Python side

- [worker.md](worker.md) — `src/worker.py` (PyMuPDF + marker driver)

## Go side

- [gui-main.md](gui-main.md) — `src/gui/main.go` (window, worker driver, buttons)
- [gui-metrics.md](gui-metrics.md) — `src/gui/metrics.go` (CPU/RAM/GPU sampling)
- [gui-vbar.md](gui-vbar.md) — `src/gui/vbar.go` (vertical bar widget)
- [gui-zoom.md](gui-zoom.md) — `src/gui/zoom.go` (ctrl-zoom, space-pan)

## Build / dependency notes

- [dependencies.md](dependencies.md) — `requirements.txt` and `go.mod`

## Related external docs

The repo also vendors notes for the third-party libraries this code
depends on; the most relevant ones:

- [`_docs/marker/`](../marker/) — OCR pipeline internals
- [`_docs/surya/`](../surya/) — layout + recognition models marker uses
- [`_docs/fyne/`](../fyne/) — GUI toolkit
- [`_docs/pdf-font-extractor/`](../pdf-font-extractor/) — related PDF work
