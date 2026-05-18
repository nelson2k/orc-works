# gui/vbar.go

A custom Fyne widget: a vertical fill bar with a value label on top
and a name label on the bottom. Five of them stack in a
`container.NewGridWithColumns(5, ...)` to form the left-edge metrics
column.

## vBar (lines 12–24)

`widget.BaseWidget` plus:

- `name`     — static label drawn at the bottom (e.g. "CPU")
- `fillCol`  — color of the filled portion
- `value`    — 0..1 fill fraction
- `valueStr` — top label (e.g. "73%", "1.4G", "n/a")

`newVBar(name, fill)` constructs one with `valueStr="--"` so it has
something to draw before the first `set()`.

## set (lines 26–36)

Clamps `frac` to `[0,1]`, stores the new label, calls `Refresh()`.
Refresh on a Fyne widget is safe from any goroutine, but the metrics
loop already wraps `set` calls in `fyne.Do` for tidiness.

## Renderer (lines 50–110)

Three canvas primitives:

- `track` — full-height rectangle in theme InputBackground color
- `fill` — the colored portion, anchored to the *bottom* of the track
  (height = `trackH * value`, y = `trackTop + (trackH - fillH)`)
- two `canvas.Text` labels — `value` on top, `name` on bottom

### Layout (lines 56–87)

- 4-px vertical padding between text and track.
- Bar width = container width − 8 px, floored at 6 px so it never
  vanishes when the grid shrinks.
- `MinSize() = (34, 120)` — the widget refuses to draw below that.

### Refresh (lines 93–104)

Re-pulls theme colors so light/dark theme switches at runtime, then
calls `Layout(r.bar.Size())` to recompute the fill rectangle for the
new `value`. (Layout is normally called by the parent; doing it here
makes `set()` redraw correctly without a reflow.)
