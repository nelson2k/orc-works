# Renderers

[marker/renderers/](../../repos-folder/marker/marker/renderers/). All subclass `BaseRenderer` (in [renderers/__init__.py](../../repos-folder/marker/marker/renderers/__init__.py)). The base class handles:

- walking `document.render()` which produces a nested `BlockOutput` tree of `(html, polygon, id, children, section_hierarchy)`,
- resolving `<content-ref src='/page/X/Type/N'></content-ref>` child placeholders,
- pulling image blocks (`Picture`, `Figure`) and base64-encoding them,
- assembling the document-level `metadata` dict (`table_of_contents`, `page_stats` with per-page extraction method and block counts).

## Shared config

- `extract_images=True` — set false to inline `[image]` placeholders / descriptions.
- `image_extraction_mode="highres"` — use the 192-DPI page image when cropping.
- `keep_pageheader_in_output=False` / `keep_pagefooter_in_output=False`.
- `add_block_ids=False` — useful for debugging; emits `id` attrs in HTML.

## Concrete renderers

| Renderer | Output | Notes |
|---|---|---|
| `MarkdownRenderer` | `MarkdownOutput(markdown, images, metadata)` | Default. Subclasses `HTMLRenderer`; converts HTML → markdown via a custom `Markdownify`. Supports inline/block math delimiters, page pagination markers (`{N}` + 48 dashes), HTML tables in markdown. |
| `HTMLRenderer` | `HTMLOutput(html, images, metadata)` | Cleans up consecutive `<math>` tags, unwraps stray outer `<p>`. |
| `JSONRenderer` | `JSONOutput(children=[…], block_type, metadata)` | Pages-as-tree. Each block has `html`, `polygon`, `id`, `children`, `section_hierarchy`. Images go on child blocks as base64. |
| `ChunkRenderer` | `ChunkOutput` | Flat list of top-level page blocks with HTML inline — good for RAG. |
| `OCRJSONRenderer` | `OCRJSONOutput` | Used by `OCRConverter`. Per-page text lines + polygons; ignores layout. |
| `ExtractionRenderer` | `ExtractionOutput(document_json, original_markdown)` | Used by `ExtractionConverter`. Merges per-page LLM extractions into a single doc-level JSON. |

## Selecting from CLI

`--output_format markdown|html|json|chunks`. `ConfigParser.get_renderer` returns the full class path which `BaseConverter.resolve_dependencies` instantiates.

## `text_from_rendered`

[marker/output.py](../../repos-folder/marker/marker/output.py) — utility that turns any of the above into `(text, ext, images)`. Useful when embedding.

```python
from marker.output import text_from_rendered
text, ext, images = text_from_rendered(rendered)
```
