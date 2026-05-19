# ZoomPanel

`ZoomPanel.h` / `ZoomPanel.cpp` — scrollable, zoomable image preview.

Subclass of `wxScrolled<wxPanel>` with scroll rate 20 × 20 and a black
background. Uses `wxBG_STYLE_PAINT` + `wxAutoBufferedPaintDC` so there's
no flicker.

## State

- `wxImage image_` — the source bitmap, set by `SetImage`.
- `wxBitmap bitmap_` — the scaled bitmap actually painted.
- `double scale_` — current zoom factor (starts 1.0, persists across
  `SetImage` calls so navigating pages keeps the zoom).
- `ctrlDown_`, `spaceDown_`, `dragging_`, `dragLast_` — input state.

## Zoom

Constants:

```
kZoomStep    = 1.10
kZoomMinSide = 100 px
kZoomMaxSide = 8000 px
```

`wxEVT_MOUSEWHEEL`: when `ctrlDown_` and an image is loaded, multiply
`scale_` by `1.1` (wheel up) or `1/1.1` (wheel down). The new scaled size
must fit `[kZoomMinSide, kZoomMaxSide]` on both axes — otherwise the
event is dropped silently. Without Ctrl held, the wheel event is
`Skip()`-ed so the scrolled window can do normal vertical scrolling.

## Pan

Hold Space to grab. The cursor swaps to `wxCURSOR_HAND` while Space is
down (`SetSpaceDown` from [main.cpp](main.md)).

- `LEFT_DOWN` while `spaceDown_` → start dragging, `CaptureMouse()`.
- `MOTION` while dragging → diff against `dragLast_`, divide by the
  scroll pixels-per-unit, and `Scroll()` to the new offset.
- `LEFT_UP` → release capture.

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

`DoPrepareDC(dc)` translates the DC by the scroll offset, then the bitmap
is blitted at (0, 0) with no transparency. Areas outside the bitmap show
the black background.
