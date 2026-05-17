# Surya Schemas

Surya model outputs are pydantic objects with geometry.

## Detection

`TextDetectionResult`

- `bboxes: list[PolygonBox]`
- `heatmap`
- `affinity_map`
- `image_bbox`

Use this for line/region overlays.

## Layout

`LayoutResult`

- `bboxes: list[LayoutBox]`
- `image_bbox`
- `sliced`

`LayoutBox` extends `PolygonBox` with:

- `label`
- `position`
- `top_k`

Labels are relabeled by `LAYOUT_PRED_RELABEL`, for example:

- `<text>` -> `Text`
- `<table>` -> `Table`
- `<section-header>` -> `SectionHeader`
- `<figure>` -> `Figure`
- `<equation-block>` -> `Equation`
- `<code-block>` -> `Code`

## Recognition

`OCRResult`

- `text_lines: list[TextLine]`
- `image_bbox`

`TextLine` includes:

- polygon geometry
- `text`
- `confidence`
- `chars`
- optional `words`
- `original_text_good`

Characters and words also carry geometry and confidence.

## Table Recognition

`TableResult`

- `cells`
- `unmerged_cells`
- `rows`
- `cols`
- `image_bbox`

`TableCell` includes row/column IDs, spans, header flags, merge flags, optional
text lines, and polygon geometry.
