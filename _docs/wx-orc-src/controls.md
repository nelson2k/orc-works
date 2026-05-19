# Controls

## Toolbar

| Widget | Purpose | File:line |
| --- | --- | --- |
| Open PDF | `wxFileDialog`, loads + renders page 0 | [main.cpp:421-438](../../wx-ocr-src/src/main.cpp#L421-L438) |
| Engine dropdown | Auto / Marker / Marker+LLM / VLM / MinerU | [main.cpp:50-56](../../wx-ocr-src/src/main.cpp#L50-L56) |
| Backend dropdown | Local / Remote, resets remote-upload cache | [main.cpp:605-614](../../wx-ocr-src/src/main.cpp#L605-L614) |
| Extract Page | OCR the current page | [main.cpp:623-687](../../wx-ocr-src/src/main.cpp#L623-L687) |
| Extract PDF | OCR every page, sequential | [main.cpp:689-801](../../wx-ocr-src/src/main.cpp#L689-L801) |
| Stop | Cancel the in-flight request | [main.cpp:616-621](../../wx-ocr-src/src/main.cpp#L616-L621) |
| Prev / Next | Re-render adjacent page | [main.cpp:440-446](../../wx-ocr-src/src/main.cpp#L440-L446) |

Most buttons are disabled when there's no PDF loaded, and a `SetBusy(true)`
disables everything except Stop while a request is in flight
([main.cpp:373-401](../../wx-ocr-src/src/main.cpp#L373-L401)).

## Keyboard

Bound via `wxEVT_CHAR_HOOK` so they work regardless of focus
([main.cpp:311-331](../../wx-ocr-src/src/main.cpp#L311-L331)):

| Key | Action |
| --- | --- |
| Left arrow | Previous page (only if not busy, total > 0, focus isn't the text area) |
| Right arrow | Next page (same guards) |
| Ctrl (held) | Modifier — enables Ctrl+wheel zoom in the preview |
| Space (held) | Modifier — enables pan-drag in the preview |

`Ctrl` / `Space` state is mirrored into the `ZoomPanel` via
`SetCtrlDown` / `SetSpaceDown`.

## Mouse — ZoomPanel

[ZoomPanel.cpp](../../wx-ocr-src/src/ZoomPanel.cpp):

- **Ctrl + wheel** → zoom in/out by factor `1.1` per notch, clamped to
  `[100, 8000]` px on either side
  ([ZoomPanel.cpp:117-133](../../wx-ocr-src/src/ZoomPanel.cpp#L117-L133)).
- **Space + drag** → pan. Mouse down with `spaceDown_` captures the
  mouse; motion scrolls the panel
  ([ZoomPanel.cpp:135-169](../../wx-ocr-src/src/ZoomPanel.cpp#L135-L169)).
- **Cursor** changes based on state:
  - dragging → sizing cursor
  - space-held → hand cursor
  - else → default
  ([ZoomPanel.cpp:40-44](../../wx-ocr-src/src/ZoomPanel.cpp#L40-L44)).
- **SyncSpaceFromKeyboard** re-checks `wxGetKeyState(WXK_SPACE)` on
  every mouse event because focus changes can cause us to miss a key-up
  ([ZoomPanel.cpp:28-38](../../wx-ocr-src/src/ZoomPanel.cpp#L28-L38)).

## Fit-to-width

[`ZoomPanel::FitWidth`](../../wx-ocr-src/src/ZoomPanel.cpp#L56-L76)
computes the scale so `image.width * s == client.width − vertical-scrollbar-width`,
clamped to the same `[100, 8000]` range, and resets the scroll to `(0,0)`.
It's invoked automatically on the first render after Open
(`fitOnNextImage_` flag in [main.cpp:436](../../wx-ocr-src/src/main.cpp#L436)).

## Stop semantics

[`OnStop`](../../wx-ocr-src/src/main.cpp#L616-L621) sets
`stopRequested_` and calls `worker_.cancel()`.

- **Local**: `cancelLocal()` calls `TerminateProcess` on the Python
  subprocess. Hard kill; any half-loaded model is lost. The next
  request relaunches the worker.
- **Remote**: `cancelRemote()` closes the WinHTTP request handle so
  the read loop unblocks with a failure. The server continues
  processing (no cooperative cancel).

In Extract PDF, the loop checks `stopRequested_` before each page so
the user sees an immediate "stopped" status even if the kill is racy.
