# wx-ocr-src

Notes on the `wx-ocr-src/` folder.

C++/wxWidgets desktop OCR application. Mirrors the (now-retired) Go +
Fyne port and talks to its own Python OCR worker over newline-delimited
JSON on stdin/stdout.

The app opens a PDF, renders pages for preview, and extracts Markdown
from a single page or every page. Engine selectable from the toolbar:

- `auto` — automatic routing
- `marker` — Marker OCR
- `marker_llm` — Marker OCR with a local LLM
- `vlm` — Qwen2.5-VL visual-language extraction

Windows-only (`__WXMSW__` paths, `dwmapi`, `nvidia-smi`, raw Win32 pipes
for the worker process).

Per-folder docs:

- [layout.md](layout.md) — folder tree, runtime path resolution
- [build.md](build.md) — CMake target, deps
- [main.md](main.md) — `MainFrame`, layout, threading, extract flow
- [worker.md](worker.md) — Python child process spawn / cancel
- [metrics.md](metrics.md) — CPU / RAM / GPU sampling
- [vbar.md](vbar.md) — vertical metric bar control
- [zoompanel.md](zoompanel.md) — scrollable, zoomable preview
- [flatbutton.md](flatbutton.md) — owner-drawn toolbar button
- [controls.md](controls.md) — keyboard and mouse cheatsheet
- [resources.md](resources.md) — SVG icons, RC manifest

The folder is self-contained at runtime: `worker.py`, `requirements.txt`,
and `venv/` (gitignored) all live inside `wx-ocr-src/`. Build with
`wx-run.ps1` from the repo root — it configures, builds, and launches
`build/orcgui.exe`.
