# main.cpp

Entry point, `App`, and `MainFrame`.

## Window structure

A single `MainFrame` (`wxFrame`) sized to 1200×800 logical pixels.
Top-to-bottom layout:

1. **File-name banner** (`fileLabel_`, `wxStaticText`) — centered, bold,
   +2 pt over default; populated from `OnOpen` with the basename of the
   loaded PDF. Empty (zero-height) until something is opened.
2. **Top toolbar** (`topPanel`, horizontal `wxBoxSizer`):
   `Open PDF`, engine `wxChoice`, `Extract Page`, `Extract PDF`, **`Stop`**
   (red), stretch spacer, `Prev`, page label, `Next`. All custom buttons
   are [FlatButton](flatbutton.md) instances.
3. **Outer `wxSplitterWindow`** — left pane is a 5-column metrics panel
   ([VBar](vbar.md) × 5: CPU, RAM, GPU, VRAM, TEMP). Right pane is the
   inner splitter. `SetSashGravity(0.0)` keeps the metrics column fixed
   width.
4. **Inner `wxSplitterWindow`** — left is the [ZoomPanel](zoompanel.md),
   right is a multiline `wxTextCtrl` (no wrap, dark) for OCR output.
   Initial sash 700 DIP, gravity 0.7.
5. **Status label** (`wxStaticText`) at the bottom.

## Theme

```
kBg = (45, 45, 48)
kFg = (220, 220, 220)
```

Plus `MSWEnableDarkMode(DarkMode_Always)` and `DwmSetWindowAttribute`
to paint the title bar and border black on Windows 11. The window title
is updated to `OCR Works — <filename>` when a PDF is opened.

## Icon

Right after construction:

- The Win32 resource icon (`SetIcon(wxICON(orcgui))`) seeds the icon
  early; backed by [src/icon.ico](../../wx-ocr-src/src/icon.ico).
- Then `wxBitmapBundle::FromSVGFile("icons/OCR_toolbar_icon.svg")` is
  rasterized at 16/24/32/48/64/128/256 px, turned into a `wxIconBundle`,
  and passed to `SetIcons(...)`. Provides crisp DPI scaling at runtime;
  see [resources.md](resources.md).

## State

`MainFrame` owns:

- `Worker worker_` — Python child process (see [worker.md](worker.md)).
- `MetricsCollector collector_` — system metrics, polled every 1 s
  by `wxTimer metricsTimer_`.
- `wxString curPath_`, `int curPage_`, `int curTotal_` — current PDF.
- `std::atomic<bool> busy_` — guards re-entrancy during worker requests.
- `std::atomic<bool> stopRequested_` — set by Stop, checked in catch
  blocks and the Extract PDF loop.
- `bool ctrlDown_`, `bool spaceDown_` — modifier mirror state forwarded
  to `ZoomPanel` via `wxEVT_CHAR_HOOK` / `wxEVT_KEY_UP`.

## Event handlers

- `OnOpen` — `wxFileDialog` for a PDF, updates `fileLabel_` and the
  window title, then `RenderPage(0)`.
- `OnPrev` / `OnNext` — bounded page step (also wired to Left/Right
  arrow keys via the `CHAR_HOOK`, see [controls.md](controls.md)).
- `RenderPage(page)` — sends `{"cmd":"render", path, page, dpi:120}` on
  a detached `std::thread`, base64-decodes the PNG, hands it to
  `ZoomPanel::SetImage` via `CallAfter`.
- `OnExtractPage` — single-page OCR; clears `stopRequested_`, streams
  progress events through `MakeOCRProgress`, dumps result text into the
  text area, status shows the `saved_to` path.
- `OnExtractPDF` — loops every page on one detached thread, checks
  `stopRequested_` at the top of each iteration, accumulates per-page
  text in a `std::ostringstream` (`# Page N\n\n<text>` separator),
  refreshes the text area after each page.
- `OnStop` — sets `stopRequested_` and calls `worker_.cancel()`, which
  kills the Python process; the in-flight `request()` thread throws on
  EOF and the catch block shows `stopped` instead of an error dialog.
- `OnMetricsTick` — polls `MetricsCollector::collect()` and writes each
  result into its `VBar`.
- `OnClose` — stops the timer and calls `worker_.shutdown()`.

## Threading

All worker requests run on detached `std::thread`s. UI mutations always
go through `CallAfter([this, ...] { ... })`, which posts a lambda to the
wx main loop. Captured wx objects (`wxImage`, `wxString`) are safe to
copy across threads because they're ref-counted internally.

## Progress streaming

`MakeOCRProgress(int page)` returns a `std::function<void(const json&)>`
that the worker thread invokes for each `type:"progress"` message:

- `kind:"image"` — base64 PNG of the current page, pushed to the preview
- `kind:"stage"` — high-level pipeline stage (remembered for the next
  tqdm event)
- `kind:"tqdm"` — per-step progress, formatted as
  `● page N — <stage> — <desc> n/total`

The current stage name lives in a `std::shared_ptr<std::string>`
captured by the lambda so each tqdm event can prefix itself with
whatever stage was last announced.
