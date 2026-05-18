# gui/zoom.go

A thin wrapper around a `canvas.Image` that adds ctrl-wheel zoom and
space-drag panning. Sits inside a `container.Scroll` (the wrapper
stores a back-pointer to it via `parent`).

## Constants

```go
zoomStep    = 1.1   // multiplicative factor per wheel notch
zoomMinSide = 100   // smallest dimension allowed
zoomMaxSide = 8000  // largest dimension allowed
```

## zoomableImage struct (lines 17–23)

Holds the wrapped image, a back-pointer to the parent scroll, and two
`*bool` pointers (`ctrlDown`, `spaceDown`) read from
`main.go`'s key handlers.

`main.go` constructs it like:

```go
zoomWrap := newZoomableImage(imgCanvas, &ctrlDown, &spaceDown)
scrollable := container.NewScroll(zoomWrap)
zoomWrap.parent = scrollable
```

The two-step construction is because the scroll needs the wrapper as
content before the wrapper can know its parent.

## Input

### Scrolled (lines 54–80)

If `ctrl` is held and the wheel moved vertically, multiply the image's
`MinSize` by `zoomStep` (or its reciprocal). Clamped to
`[zoomMinSide, zoomMaxSide]`. Otherwise, the event is forwarded to the
parent `Scroll` so the page still scrolls normally.

The zoom is achieved by changing `SetMinSize` rather than touching
the image bytes — the parent `Scroll` then re-lays out with the new
desired size, and the image's `FillMode = ImageFillContain` does the
actual resampling.

### Dragged (lines 31–43)

Space-drag panning: when `space` is held, the drag delta is
subtracted from the scroll's `Offset`. Without space held, drag
events are ignored (so the user can still ctrl-wheel without
accidentally grabbing the page).

### Cursor (lines 47–52)

Returns a pointer cursor while space is held — visual cue that the
view is in pan mode.

## Renderer (lines 82–107)

Minimal: lays out the wrapped image to fill the wrapper, reports the
image's `MinSize` as the wrapper's `MinSize`. That `MinSize`
propagation is what lets the `Scroll` notice that zooming made the
image bigger.
