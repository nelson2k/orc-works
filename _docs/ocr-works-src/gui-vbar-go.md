# src/gui/vbar.go

Custom Fyne widget for a single vertical fill bar. Used five times by [main.go](../../src/gui/main.go) for the CPU / RAM / GPU / VRAM / TEMP column.

## Shape

```
+--------+
|  47%   |   <- value label (centered, caption size)
+--------+
| ▓▓▓▓▓▓ |
| ▓▓▓▓▓▓ |   <- track (theme InputBackground) + fill (per-bar color)
| ████   |      fill rect grows from the bottom
| ████   |
+--------+
|  CPU   |   <- name label (centered, caption size)
+--------+
```

## API

- `newVBar(name string, fill color.Color) *vBar` — constructor; remembers a per-bar fill color and starts with the placeholder text `--`.
- `(*vBar).set(frac float64, label string)` — `frac` clamped to `[0, 1]` drives fill height; `label` is the small text shown above the bar. Calls `Refresh()`, safe from the UI goroutine.

## Renderer

`vBarRenderer` implements `fyne.WidgetRenderer`. `Layout` computes:

- `valueH` from `value` label `MinSize` → top strip.
- `nameH` from `name` label `MinSize` → bottom strip.
- Track fills the strip between, inset by 4 px padding.
- Bar width = `size.Width − 8` (min 6), centered horizontally.
- Fill height = `trackH × value`; positioned so its top is `trackTop + (trackH − fillH)` (grows from the bottom).

`MinSize()` returns `{34, 120}`, so the 5-bar grid claims ~170 px of width minimum.
