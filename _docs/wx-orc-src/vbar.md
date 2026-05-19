# VBar

`VBar.h` / `VBar.cpp` — a thin vertical bar used to render one metric.

## Construction

```cpp
VBar(wxWindow* parent, const wxString& name, const wxColour& fill);
```

`name` is the label shown at the bottom (`CPU`, `RAM`, `GPU`, `VRAM`,
`TEMP`). `fill` is the fill colour for the bar. Min size 34 × 120 DIP.
Background uses `wxBG_STYLE_PAINT` and the control double-buffers via
`wxAutoBufferedPaintDC`.

## Update

```cpp
void Set(double frac, const wxString& label);
```

`frac` is clamped to `[0, 1]`. `label` is the text shown above the bar
(e.g. `"42%"`, `"3.2G"`, `"68°"`). Both call `Refresh(false)` to schedule
a repaint. `OnSize` also forces a repaint so the bar redraws when the
metrics column is resized.

## Paint

Each frame:

1. Clear with the panel's background.
2. Draw the current value text at the top, centered.
3. Draw the metric name at the bottom, centered.
4. Compute a "track" rectangle between the two labels with a 4 px pad.
5. Fill the track with `(60, 60, 60)`.
6. Fill the bottom `frac * trackH` of the track with `fillCol_`.

Font size is fixed at 8 pt regardless of the inherited font.
