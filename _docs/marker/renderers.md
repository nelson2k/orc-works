# Renderers (`marker/renderers/`)

Renderers walk the finalized `Document` and emit a final pydantic model. The
result is what the converter's `__call__` returns. The converter selects a
renderer via `--output_format` (markdown / json / html / chunks) or by
explicit `renderer=…`.

## `BaseRenderer`

Knobs all renderers share:

- `image_blocks` (Picture, Figure by default), `extract_images=True`,
  `image_extraction_mode` (`"highres"` / `"lowres"`).
- `keep_pageheader_in_output`, `keep_pagefooter_in_output` — render or hide
  detected headers / footers.
- `add_block_ids` — annotate output HTML with the internal `BlockId`s.

Helpers:

- `extract_image(document, image_id, to_base64=True)` — produces JPEG bytes
  (configurable via `settings.OUTPUT_IMAGE_FORMAT`).
- `extract_block_html(document, block_output)` — recursively inlines
  `<content-ref>` placeholders into the child HTML, separating out image data.
- `generate_document_metadata(...)` — adds `table_of_contents` + per-page
  stats to the renderer output.

## Built-in renderers

| Module           | Class               | Output model        | Notes |
|------------------|---------------------|---------------------|-------|
| `markdown.py`    | `MarkdownRenderer`  | `MarkdownOutput`    | HTML → markdown via a tweaked `MarkdownConverter`; supports inline / block math delimiters, page pagination, optional HTML tables. |
| `html.py`        | `HTMLRenderer`      | `HTMLOutput`        | Pure HTML with `<math>` for equations and `<img>` for figures. |
| `json.py`        | `JSONRenderer`      | `JSONOutput`        | Tree of `JSONBlockOutput` (per-page; children carry `section_hierarchy` + base64 `images`). |
| `chunk.py`       | `ChunkRenderer`     | `ChunkOutput`       | Flat list of top-level blocks per page, with assembled HTML — designed for RAG. |
| `ocr_json.py`    | `OCRJSONRenderer`   | `OCRJSONOutput`     | Per-page list of lines (and, if `keep_chars`, per-character bboxes). |
| `extraction.py`  | `ExtractionRenderer`| `ExtractionOutput`  | Wraps `DocumentExtractionSchema` + original markdown for `ExtractionConverter`. |

## Picking text out of a rendered object

`marker.output.text_from_rendered(rendered)` returns `(text, ext, images)` for
all of the above (json variants return their pydantic `model_dump_json`).
`save_output(rendered, dir, basename)` writes the body, a sibling
`{basename}_meta.json`, and any extracted images.
