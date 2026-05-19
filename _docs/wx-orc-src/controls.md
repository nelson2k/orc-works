# Controls

Keyboard and mouse cheatsheet for the wx GUI.

## Buttons

| Button         | Effect                                              | Enabled when         |
|----------------|-----------------------------------------------------|----------------------|
| Open PDF       | Pick a PDF; render page 1                           | not busy             |
| Engine ▾       | `auto` / `marker` / `marker_llm` / `vlm`            | not busy             |
| Extract Page   | OCR the current page                                | PDF loaded, not busy |
| Extract PDF    | OCR every page, accumulating into one Markdown doc  | PDF loaded, not busy |
| Stop  (red)    | Kill the worker, abort the current extract          | busy                 |
| Prev / Next    | Step one page back / forward                        | bounds permitting    |

## Keyboard

| Key                   | Effect                                               |
|-----------------------|------------------------------------------------------|
| ← / →                 | Previous / next page (same as Prev / Next)           |
| Space (held)          | Enable pan-mode in the preview; cursor → hand        |
| Space + drag          | Pan the preview; cursor → 4-way arrows               |
| Ctrl + mouse wheel    | Zoom the preview in/out (1.1× per click)             |
| Mouse wheel (no Ctrl) | Scroll the preview vertically                        |

Arrow keys are ignored while the OCR text area has focus, so editing /
scrolling output works normally.

## Notes on state robustness

Modifier handling lives at the frame level via `wxEVT_CHAR_HOOK`, but a
KEY_UP can be swallowed by whichever widget has focus, so the
[ZoomPanel](zoompanel.md) also queries `wxGetKeyState` on every mouse
event. This way zoom only fires when Ctrl is actually held, and pan
state self-corrects the moment the mouse moves over the preview.
