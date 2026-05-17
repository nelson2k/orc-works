# Surya Integration Notes

Surya is directly relevant to this OCR Works app because Marker uses Surya for
its core PDF intelligence.

## What can be shown over the PDF

Surya outputs can drive frontend overlays:

- text detection boxes
- OCR text lines
- OCR words/chars
- layout block labels
- table rows/columns/cells
- confidence values

The existing frontend PDF viewer can draw these as a second layer over the PDF
canvas.

## Coordinate handling

Most outputs use `PolygonBox` or subclasses. A backend overlay API should return
coordinates in a clear system:

- original image pixels, plus image width/height; or
- normalized 0..1 coordinates.

The frontend can then scale boxes to the current PDF canvas size and zoom.

## Useful drawing utility

`surya/debug/draw.py` has helpers:

- `draw_bboxes_on_image`
- `draw_polys_on_image`

These draw boxes and labels onto PIL images. For the web UI, the same geometry
should be sent as JSON and drawn in React/SVG/canvas instead of baking it into
the image.
