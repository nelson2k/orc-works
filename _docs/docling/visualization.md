# Docling Visualization

Docling already contains a small visualization helper:

`docling/utils/visualization.py`

## `draw_clusters`

`draw_clusters(image, clusters, scale_x, scale_y)` draws layout clusters over a
PIL image.

For each cluster and child cluster it draws:

- cell rectangles
- cluster rectangle
- label
- confidence

It uses `DocItemLabel.get_color(...)` so labels get consistent colors.

## Why this matters for OCR Works

Docling is already structured around page images, boxes, labels, confidence, and
page-level pipeline stages. That makes it suitable for a visual debug mode where
the frontend can draw:

- layout clusters
- table cells
- page-level processing status
- confidence labels
- final document item positions

## Best backend shape

For a live overlay UI, expose stage events from the backend:

```text
page_started
preprocess_done
ocr_done
layout_done
table_done
assemble_done
document_done
```

Each event can carry page number plus normalized or pixel-space boxes.
