# main.cpp

Entry point, `App`, and `MainFrame`. ~600 lines.

## Window structure

A single `MainFrame` (`wxFrame`) sized to 1200×800 logical pixels. Inside,
top to bottom:

1. **Top toolbar** (`topPanel`, horizontal `wxBoxSizer`):
   `Open PDF` button, engine `wxChoice`, `Extract Page`, `Extract PDF`,
   stretch spacer, `Prev`, page label, `Next`. All custom buttons are
   [FlatButton](flatbutton.md) instances loaded with SVG icons via
   `wxBitmapBundle::FromSVGFile`.
2. **Outer `wxSplitterWindow`** — left pane is a 5-column metrics panel
   ([VBar](vbar.md) × 5: CPU, RAM, GPU, VRAM, TEMP). Right pane is the
   inner splitter. `SetSashGravity(0.0)` keeps the metrics column fixed
   width.
3. **Inner `wxSplitterWindow`** — left is the [ZoomPanel](zoompanel.md)
   preview, right is a `wxTextCtrl` (multiline, no wrap, dark) for OCR
   output. Initial sash 700 DIP, gravity 0.7 (preview gets the room).
4. **Status label** (`wxStaticText`) at the bottom.

## Colors

```
kBg = (45, 45, 48)
kFg = (220, 220, 220)
```

Plus `MSWEnableDarkMode(DarkMode_Always)` and `DwmSetWindowAttribute` to
paint the title bar/border black on Windows 11.

## State

`MainFrame` owns:

- `Worker worker_` — the Python child process (see [worker.md](worker.md))
- `MetricsCollector collector_` — system metrics, polled every 1 s by
  `wxTimer metricsTimer_`
- `wxString curPath_`, `int curPage_`, `int curTotal_` — current PDF
- `std::atomic<bool> busy_` — guards re-entrancy while a worker request
  is in flight
- `bool ctrlDown_`, `bool spaceDown_` — modifier state propagated to
  `ZoomPanel` via `wxEVT_CHAR_HOOK` / `wxEVT_KEY_UP`

## Event handlers

- `OnOpen` — `wxFileDialog` for a PDF, then `RenderPage(0)`.
- `OnPrev` / `OnNext` — bounded page step.
- `RenderPage(page)` — sends `{"cmd":"render", path, page, dpi:120}` on a
  detached `std::thread`, base64-decodes the PNG, hands it to
  `ZoomPanel::SetImage` via `CallAfter`.
- `OnExtractPage` — single-page OCR via `{"cmd":"ocr", ...}`, streaming
  progress events through `MakeOCRProgress`. Result text is dumped into
  the text area; `saved_to` path goes to the status label.
- `OnExtractPDF` — loops every page on one detached thread, accumulates
  per-page text in a `std::ostringstream` (`# Page N\n\n<text>` separator),
  updates the text area after each page.
- `OnMetricsTick` — polls `MetricsCollector::collect()` and writes each
  result into its `VBar`.
- `OnClose` — stops the timer and calls `worker_.shutdown()`.

## Threading

All worker requests run on detached `std::thread`s. UI mutations always go
through `CallAfter([this, ...] { ... })`, which posts a lambda to the wx
main loop. Captured wx objects (`wxImage`, `wxString`) are safe to copy
across threads because they're ref-counted internally.

## Progress streaming

`MakeOCRProgress(int page)` returns a `std::function<void(const json&)>`
that the worker thread invokes for each `type:"progress"` message:

- `kind:"image"` — base64 PNG of the current page, pushed to the preview
- `kind:"stage"` — high-level pipeline stage (remembered for the next tqdm)
- `kind:"tqdm"` — per-step progress, formatted as
  `● page N — <stage> — <desc> n/total`

The current stage name is stashed in a `std::shared_ptr<std::string>`
captured by the lambda so each tqdm event can prefix itself with whatever
stage was last announced.
