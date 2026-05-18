# src/ overview

Top-level layout of [src/](../../src/):

- [gui/](../../src/gui/) — Go (Fyne) desktop app. See [gui-main-go.md](gui-main-go.md), [gui-vbar-go.md](gui-vbar-go.md), [gui-metrics-go.md](gui-metrics-go.md), [gui-go-mod.md](gui-go-mod.md).
- [worker.py](../../src/worker.py) — Python child process that renders PDF pages via PyMuPDF and OCRs them via marker. See [worker-py.md](worker-py.md).
- [requirements.txt](../../src/requirements.txt) — Python deps. See [requirements.md](requirements.md).
- `venv/` — local Python virtualenv (gitignored, expected at `src/venv/`).

## Architecture

The Go GUI spawns `venv/Scripts/python.exe worker.py` once and talks to it over stdin/stdout, one JSON object per line. The Go side handles UI, window state, and live system-metrics polling; the Python side does all PDF work. Replies come back as `{"type": "image" | "text" | "error", ...}`.

```
+----------------+   stdin  JSON line   +-------------------+
|  Go / Fyne GUI | -------------------> | Python worker.py  |
|  (main.go,     |                      |  pymupdf: render  |
|   vbar.go,     | <------------------- |  marker:  OCR     |
|   metrics.go)  |   stdout JSON line   |  (surya models)   |
+----------------+                      +-------------------+
```
