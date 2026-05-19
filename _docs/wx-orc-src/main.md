# main.cpp — frame, toolbar, extract loops

## App + frame

[`class App : public wxApp`](../../wx-ocr-src/src/main.cpp#L803-L815)
enables MSW dark mode (`MSWEnableDarkMode(DarkMode_Always)`), constructs
`MainFrame`, and shows.

[`MainFrame`](../../wx-ocr-src/src/main.cpp#L98-L160) owns a `Worker`,
a `MetricsCollector`, the toolbar widgets, the metrics bars, the
preview, the text area, and a 1Hz `wxTimer`.

## Title bar

[main.cpp:335-342](../../wx-ocr-src/src/main.cpp#L335-L342) tells DWM
to paint the title bar / window border black via `DwmSetWindowAttribute`
with the (un-headered) `DWMWA_CAPTION_COLOR` (35) and `DWMWA_BORDER_COLOR`
(34). The title itself is set when a PDF is opened
([main.cpp:435](../../wx-ocr-src/src/main.cpp#L435)) using a hard-coded
em-dash UTF-8 byte sequence so the display name comes through cleanly:
`"OCR Works \xE2\x80\x94 " + filename`.

## Toolbar

Built into a single `wxBoxSizer` inside `topPanel`
([main.cpp:202-239](../../wx-ocr-src/src/main.cpp#L202-L239)):

```
[Open PDF] [Engine ▾] [Backend ▾] [Extract Page] [Extract PDF] [Stop]                 [Prev] Page X of Y [Next]
```

Buttons are `FlatButton`s; dropdowns are `wxChoice`. `Stop` is colored
red via `SetColors`. Prev/Next/ExtractPage/ExtractPDF/Stop start
disabled and become enabled when there's a loaded PDF and no work in
flight — see [`SetBusy`](../../wx-ocr-src/src/main.cpp#L373-L401).

## Layout

A two-level splitter:

- Outer: metrics column (~180 px) vs everything else.
- Inner: preview (`ZoomPanel`) vs text area (`wxTextCtrl`, monospace
  feel via `wxTE_DONTWRAP`).

Bottom of the root sizer: a `wxStaticText` status label.

## Open

[`OnOpen`](../../wx-ocr-src/src/main.cpp#L421-L438) opens a PDF dialog,
resets state (clears the remote-upload cache, clears the text area,
sets the title), and triggers `RenderPage(0)`. `fitOnNextImage_` is
set so the first render auto-fits-to-width.

## Render

[`RenderPage`](../../wx-ocr-src/src/main.cpp#L448-L502) goes async on a
detached thread:

1. Resolves `PathForWorker()` (may scp in Remote mode).
2. Sends `{cmd:"render", path, page, dpi:120}` to the worker.
3. Decodes the base64 PNG into `wxImage`, posts back to the GUI via
   `CallAfter` — pumps the image into the `ZoomPanel`, updates
   `curPage_`/`curTotal_`, clears the text area, drops `SetBusy(false)`.

## Extract Page

[`OnExtractPage`](../../wx-ocr-src/src/main.cpp#L623-L687):

- `SetBusy(true)`, clear text, status = `"starting (<engine>)..."`.
- On a detached thread, send `{cmd:"ocr", path, page, engine}` with a
  progress callback from `MakeOCRProgress`.
- On success: text area = result, status = `"[engine] saved → <path>"`
  or `"[engine] done"`.
- On error: dialog + clear status. On Stop: status = `"stopped"`.

## Extract PDF

[`OnExtractPDF`](../../wx-ocr-src/src/main.cpp#L689-L801) loops over
every page sequentially:

1. Check `stopRequested_` before each iteration.
2. Update `curPage_` / page label.
3. **Re-render** the current page (so the preview shows what's being
   OCR'd) — wrapped in try/catch so a render hiccup doesn't kill the
   loop ([main.cpp:723-739](../../wx-ocr-src/src/main.cpp#L723-L739)).
4. Send the `ocr` command, propagate stage/tqdm/image progress.
5. On success, **show only the current page's text** in the textbox
   (the most recent run replaces it — see the `setValue` at
   [main.cpp:765-770](../../wx-ocr-src/src/main.cpp#L765-L770)).
6. After all pages, status = `"saved → <last_savedTo>"`.

If `stopRequested_` flips, the loop breaks and status becomes
`"stopped"`.

## Progress wiring

[`MakeOCRProgress`](../../wx-ocr-src/src/main.cpp#L504-L543) builds a
lambda that:

- For `kind == "image"`: decodes the base64 PNG and replaces the
  preview (used by Marker's layout overlay).
- For `kind == "stage"`: renders `"● page N — <stage>"` into the
  status label.
- For `kind == "tqdm"`: appends the desc + `n/total` to the last
  stage label.

All status updates marshal to the GUI thread via `CallAfter`.

## Metrics tick

[`OnMetricsTick`](../../wx-ocr-src/src/main.cpp#L403-L419) — 1Hz timer
handler. In Remote mode, tries `worker_.getRemoteMetrics()` first;
falls back to the local `MetricsCollector`. Pushes values to the 5
`VBar`s. See [metrics.md](metrics.md).

## Keyboard hooks

`wxEVT_CHAR_HOOK` and `wxEVT_KEY_UP` track Ctrl + Space state and feed
the `ZoomPanel`; Left/Right pages through the PDF when not focused on
the text area and not busy
([main.cpp:311-331](../../wx-ocr-src/src/main.cpp#L311-L331)).

## Close

[`OnClose`](../../wx-ocr-src/src/main.cpp#L352-L356) stops the metrics
timer and calls `worker_.shutdown()`. In Local mode this sends a
`{cmd:"quit"}` JSON line and reaps the subprocess; in Remote mode it's
a no-op (we don't own the FastAPI lifecycle).
