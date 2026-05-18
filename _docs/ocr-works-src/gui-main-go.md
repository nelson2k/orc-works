# src/gui/main.go

Fyne desktop app entry point. Sibling files [vbar.go](../../src/gui/vbar.go) and [metrics.go](../../src/gui/metrics.go) provide the left metrics column.

## Worker plumbing

`Worker` struct owns the Python child process:

- `ensure()` lazily spawns `../venv/Scripts/python.exe ../worker.py` (paths relative to `src/gui/`) and wires `StdinPipe` + a `bufio.Scanner` over `StdoutPipe` with a 64 MiB max line so base64 PNGs and big OCR replies fit.
- `request(map, onProgress)` marshals JSON, writes one line, then scans lines back in a loop. Any reply with `type=="progress"` is handed to `onProgress` (if non-nil) and the loop continues; the first non-progress reply is returned. Mutex-guarded so the UI can fire requests from multiple goroutines safely.
- `shutdown()` (deferred from `main`) sends `{"cmd":"quit"}` and `Wait`s.

## UI layout

- Far left: narrow metrics column — five `vBar`s (CPU / RAM / GPU / VRAM / TEMP) side by side in a `container.NewGridWithColumns(5, ...)`. See [gui-vbar-go.md](gui-vbar-go.md).
- Center: `canvas.Image` inside a `container.Scroll`, `ImageFillContain`, min size 500×700.
- Right: multi-line `widget.Entry` with word wrap for OCR text.
- Top bar: **Open PDF** + **OCR Page** on the left; **Prev / page label / Next** on the right.
- Bottom: `statusLabel` — single-line label that shows the current OCR pipeline stage and tqdm batch progress while OCR is running.
- Window 1200×800, centered.

The center+right pair is an `HSplit` (offset 0.6). The metrics column slots into the left of `container.NewBorder(topBar, statusLabel, metricsCol, nil, split)`.

## State

Three locals capture the open doc: `curPath`, `curPage`, `curTotal`. `setBusy(bool)` is the single place that flips button enabled state based on doc state + whether a worker request is in flight.

## Flows

- **Open PDF**: `github.com/sqweek/dialog` native file picker filtered to `*.pdf`, then `render(0)`.
- **render(page)**: goroutine → `worker.request({cmd:"render", path, page, dpi:120})` → base64-decode PNG → `image.Decode` → marshal back to UI thread via `fyne.Do` to assign `imgCanvas.Image` + refresh. Updates `curTotal` from the reply's `pages`.
- **OCR Page**: goroutine → `worker.request({cmd:"ocr", path, page}, onProgress)` → text area shows the markdown returned by marker. The `onProgress` callback receives stage events (`● layout`, `● ocr_recognition`, …) and tqdm batch events (`● ocr_recognition — Recognizing 3/10`) and pushes the formatted string into `statusLabel` via `fyne.Do`. First call is slow (model load); see [worker-py.md](worker-py.md).
- **Prev / Next**: bounds-checked `render(curPage±1)`.

All worker replies are checked for `type == "error"` and surfaced via `dialog.ShowError`.

## Metrics ticker

`runMetricsLoop(stop, ...bars)` lives in this file. Primes CPU sampling with one `collect()` call, then on a 1 s `time.Ticker` re-samples and pushes updates onto the UI thread via `fyne.Do`. Labels:

- CPU / GPU: `N%` utilization
- RAM / VRAM: `X.XG` used (fill is still % of total)
- TEMP: `N°` GPU temp (fill is `temp/100`, clamped to `[0,1]`)

If `nvidia-smi` is absent the three GPU bars show `n/a` and stay empty. The loop is stopped via a `defer close(stop)` from `main`.
