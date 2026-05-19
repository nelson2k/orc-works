# wx-ocr-src

Notes on the `wx-ocr-src/` folder.

C++ port of the desktop OCR app, built with wxWidgets. It mirrors the Go +
Fyne app in `go-ocr-src/` and talks to the same Python worker over
newline-delimited JSON on stdin/stdout.

The app opens a PDF, renders pages for preview, and extracts Markdown from a
single page or every page. Engines selectable from the toolbar:

- `auto` — automatic routing
- `marker` — Marker OCR
- `marker_llm` — Marker OCR with a local LLM
- `vlm` — Qwen2.5-VL visual-language extraction

Windows-only (`__WXMSW__` paths, `dwmapi`, `nvidia-smi`, raw Win32 pipes for
the Python worker). See [layout.md](layout.md) for runtime assumptions.

The folder is self-contained: `worker.py`, `requirements.txt`, and a
ready-to-run `venv/` (gitignored) all live inside `wx-ocr-src/`. Build
with `wx-run.ps1` from the repo root — it configures, builds, and
launches `build/orcgui.exe` directly.
