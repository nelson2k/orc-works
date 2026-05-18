# gui/vbar.go

`vbar.go` defines a custom Fyne widget for the metrics column.

Each `vBar` has:

- a static name label at the bottom
- a value label at the top
- a themed background track
- a colored vertical fill rectangle

`set(frac, label)` clamps the fill fraction to `[0,1]`, stores the display
label, and refreshes the widget.

The renderer keeps a minimum size of `34 x 120`. The fill grows upward from
the bottom of the track. Runtime theme colors are refreshed in `Refresh`, so
foreground and track colors follow Fyne theme changes.
