# gui/main.go

Single-window Fyne app. Drives the Python OCR worker, displays the
current page, runs a 1 Hz metrics loop.

## Worker struct (lines 28–104)

Wraps the long-lived Python subprocess.

- `ensure()` spawns `..\venv\Scripts\python.exe ..\worker.py` on first
  use. Output goes through a `bufio.Scanner` with a 64 MiB buffer —
  required because base64-encoded page PNGs land in a single line.
- `request(req, onProgress)` is the single I/O entry point. It holds
  `mu` for the whole exchange, writes one JSON line, and scans
  replies. Progress messages are passed to `onProgress` and the loop
  continues; the first non-progress reply is returned. See
  [worker-protocol.md](worker-protocol.md).
- `shutdown()` writes `{"cmd":"quit"}\n`, closes stdin, waits for
  exit. Wired via `defer worker.shutdown()` in `main`.

## main() (lines 136–435)

Layout:

```
┌─ topBar ──────────────────────────────────┐
│ [Open] [OCR Page]        [Prev] N/M [Next]│
├──────────┬───────────────────┬────────────┤
│ metrics  │  page image       │  text area │
│ column   │  (scrollable,     │  (markdown │
│ (5 vBar) │  zoom/pan via     │  output)   │
│          │  zoomableImage)   │            │
├──────────┴───────────────────┴────────────┤
│ statusLabel                                │
└────────────────────────────────────────────┘
```

Key wires:

- `imgCanvas` is a `canvas.Image` wrapped by `zoomableImage` (see
  [gui-zoom.md](gui-zoom.md)). The wrapper sits inside a
  `container.Scroll` and reads `ctrlDown`/`spaceDown` flags maintained
  by `dc.SetOnKeyDown`/`SetOnKeyUp`.
- `render(page)` is a closure (line 239). Disables buttons via
  `setBusy(true)`, kicks off a goroutine that calls
  `worker.request({"cmd":"render", ...})`, decodes the base64 PNG,
  then marshals back to the UI thread with `fyne.Do`.
- `ocrBtn.OnTapped` (line 302) is the long-running counterpart. Its
  `onProgress` callback handles three event kinds:
  - `image` — swap a layout-overlay PNG onto the page canvas
  - `stage` — update `statusLabel` to `● <name>`, remembered in
    `lastStage`
  - `tqdm` — format as `● <stage> — <desc> N/total`, falling back to
    the stage name when tqdm has no total.
- The terminal reply puts the markdown text in `textArea` and shows
  `saved → <dir>` in the status bar.
- `openBtn.OnTapped` uses `sqweek/dialog` (native OS file picker)
  rather than Fyne's `dialog.ShowFileOpen`. Cancel is detected via
  `nfd.ErrCancelled`.

## runMetricsLoop (lines 106–134)

Ticks once per second, calls `collect()` (see
[gui-metrics.md](gui-metrics.md)), and pushes formatted values into
five `vBar` widgets. UI updates are wrapped in `fyne.Do` so they cross
back onto the UI thread.

If no GPU is detected (`!s.hasGPU`), the GPU/VRAM/TEMP bars are
explicitly reset to `0, "n/a"` each tick — so a GPU disappearing
mid-session is reflected.

A primer `collect()` call is made before entering the loop because
gopsutil's `cpu.Percent(0, false)` needs a previous reading to
diff against.

## Concurrency model

Three goroutines:

1. UI / main goroutine — Fyne's `ShowAndRun`.
2. One transient goroutine per `render` or OCR request.
3. The metrics ticker goroutine, stopped via `close(stop)` deferred
   from `main`.

All UI mutations happen through `fyne.Do`. The worker's `mu` ensures
only one request is in flight; the GUI also disables buttons via
`setBusy(true)` while a request runs, so the user can't queue two.

## Why these libraries

- `fyne.io/fyne/v2` — chosen GUI toolkit (see `_docs/fyne/`).
- `github.com/shirou/gopsutil/v4` — CPU + RAM stats, cross-platform.
- `github.com/sqweek/dialog` — native file-picker dialog. Fyne's
  built-in dialog is sandboxed to its own widget tree and didn't fit.
