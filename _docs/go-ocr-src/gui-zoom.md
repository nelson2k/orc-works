# gui/zoom.go

`zoom.go` wraps a `canvas.Image` so the preview can zoom and pan inside a
Fyne `container.Scroll`.

Controls:

- ctrl + mouse wheel: zoom in or out
- space + drag: pan the scroll offset

Zoom is implemented by changing the wrapped image's minimum size. The bounds
are:

- minimum side: `100`
- maximum side: `8000`
- zoom step: `1.1`

The wrapper forwards normal wheel events to the parent scroll when ctrl is
not held. While space is held, it reports a pointer cursor as a visual cue
that drag panning is active.
