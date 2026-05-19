# ZoomPanel

`ZoomPanel.h` / `ZoomPanel.cpp` — scrollable, zoomable image preview.

Subclass of `wxScrolled<wxPanel>` with scroll rate 20 × 20 and a black
background. Uses `wxBG_STYLE_PAINT` + `wxAutoBufferedPaintDC` so there's
no flicker.

## State

- `wxImage image_` — source bitmap, set by `SetImage`.
- `wxBitmap bitmap_` — scaled bitmap actually painted.
- `double scale_` — current zoom factor (persists across `SetImage`
  calls so navigating pages keeps the zoom).
- `ctrlDown_`, `spaceDown_`, `dragging_`, `dragLast_` — input state.

`ctrlDown_` / `spaceDown_` are mirrored from the frame-level CHAR_HOOK
in [main.md](main.md), but the panel **never relies on them blindly** —
see "Robust modifier state" below.

## Zoom (Ctrl + wheel)

Constants:

```
kZoomStep    = 1.10
kZoomMinSide = 100 px
kZoomMaxSide = 8000 px
```

`OnMouseWheel` refreshes `ctrlDown_` from the live keyboard state at the
moment the wheel event fires:

```cpp
ctrlDown_ = wxGetKeyState(WXK_CONTROL) || evt.ControlDown();
```

Then, only when Ctrl is held and an image is loaded, it multiplies
`scale_` by `1.1` (wheel up) or `1/1.1` (wheel down). The new size must
fit `[kZoomMinSide, kZoomMaxSide]` on both axes — otherwise the event is
dropped silently. Plain wheel rotation, without Ctrl, falls through to
`evt.Skip()` and scrolls normally.

## Pan (Space + drag)

Hold Space to grab; the cursor reflects the state via `UpdateCursor()`:

| State                          | Cursor              |
|--------------------------------|---------------------|
| Space released                 | default arrow       |
| Space held, not dragging       | `wxCURSOR_HAND`     |
| Space held, mouse button down  | `wxCURSOR_SIZING`   |

Sequence:

- `LEFT_DOWN` while `spaceDown_` → start dragging, `CaptureMouse()`.
- `MOTION` while dragging → diff against `dragLast_`, divide by the
  scroll pixels-per-unit, `Scroll()` to the new offset.
- `LEFT_UP` → release capture; cursor reverts to "ready to grab" or
  default depending on whether space is still held.

## Robust modifier state

A KEY_UP event can be swallowed by a focused widget elsewhere (e.g. the
OCR text area), leaving `spaceDown_` stuck true if we only listened to
events. To recover, `SyncSpaceFromKeyboard()` queries
`wxGetKeyState(WXK_SPACE)` and reconciles state + cursor. It's called
from `OnMouseMotion`, `OnMouseDown`, and `OnEnter` — so the next time
the mouse touches the panel, state is corrected. Drag is also cancelled
on the spot if the user released space outside the panel.

## Rebuild

`SetImage` and any successful zoom call `RebuildBitmap()`:

```
w = max(1, image.width  * scale)
h = max(1, image.height * scale)
bitmap = image.Scale(w, h, wxIMAGE_QUALITY_BILINEAR)
```

then `UpdateVirtualSize(bitmap.width, bitmap.height)` so the scrollbars
match the scaled image.

## Paint

`DoPrepareDC(dc)` translates the DC by the scroll offset, then the
bitmap is blitted at (0, 0) with no transparency. Areas outside the
bitmap show the black background.
