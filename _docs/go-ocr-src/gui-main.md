# gui/main.go

`main.go` owns the Fyne window and the Python subprocess driver.

Main UI pieces:

- `Open PDF`: opens a native file picker through `github.com/sqweek/dialog`
- engine selector: `Auto`, `Marker`, `Marker + LLM`, `VLM (Qwen2.5-VL)`
- `Extract Page`: extracts the current page
- `Extract PDF`: loops through all pages and extracts each one
- `Prev` / `Next`: page navigation
- page preview: scrollable image with zoom and pan support
- text panel: accumulated Markdown output
- status bar: current stage or saved path
- left metrics column: CPU, RAM, GPU, VRAM, temperature

The `Worker` struct starts `../venv/Scripts/python.exe ../worker.py` lazily
on first request. It uses a `bufio.Scanner` with a 64 MiB maximum token size
because rendered PNG images are sent as base64 on one JSON line.

Rendering calls `{"cmd":"render"}` and decodes the returned PNG into the
canvas image. Extraction calls `{"cmd":"ocr"}` with the selected engine.

`Extract PDF` processes pages sequentially. It updates the page label,
appends each page's Markdown to the text area with a page heading, and relies
on the worker to save each page to disk.
