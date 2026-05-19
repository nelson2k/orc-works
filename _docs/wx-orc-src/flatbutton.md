# FlatButton

`FlatButton.h` / `FlatButton.cpp` — flat, owner-drawn toolbar button with
an optional icon + bold text label.

Subclass of `wxPanel`, not `wxButton`, so it can ignore the native
look-and-feel.

## Colors

```
normal   (33, 150, 243)   // Material Blue 500
hover    (66, 165, 245)   // Blue 400
pressed  (21, 101, 192)   // Blue 800
disabled (70, 90, 110)    // (declared but currently unused on paint)
text     white
```

## Construction

```cpp
FlatButton(wxWindow* parent, const wxString& label,
           const wxBitmapBundle& icon = wxBitmapBundle());
```

The constructor bumps the inherited font by 1 pt and bolds it, measures
the label, computes a `MinSize` that fits:

```
14 px pad | icon | 6 px gap | label | 14 px pad     height 36 DIP
```

If `icon` is empty, the gap collapses to zero.

## Click

`FlatButton` synthesises a normal `wxEVT_BUTTON` event so callers can use
`Bind(wxEVT_BUTTON, &Handler, this)` as if it were a real `wxButton`. The
sequence:

1. `LEFT_DOWN` while enabled → `pressed_ = true`, `CaptureMouse()`, repaint.
2. `LEFT_UP` → release capture; if `pressed_` was true *and* the cursor is
   still inside the panel, call `EmitClick()`.
3. `MOUSE_CAPTURE_LOST` is handled defensively to drop `pressed_`.
4. `Enter` / `Leave` flip `hover_` and repaint.

`Enable(bool)` is overridden to reset `hover_`/`pressed_` and trigger a
repaint so a button disabled mid-press doesn't get stuck looking pressed.

## Paint

`OnPaint` picks the background colour based on `pressed_` / `hover_`,
fills the whole client rect, draws the icon centered vertically at
`x = 14 px`, then draws the label after a 6 px gap. There's no rounded
corner, border, or focus ring — the colour change is the entire
affordance.

## Note on disabled

`kDisabled` is declared but never selected by `OnPaint` — a disabled
button currently still paints in the blue normal colour. The only visible
disabled cue is the cleared `hover_`/`pressed_` state from
`Enable(false)`. Worth fixing if it ever matters.
