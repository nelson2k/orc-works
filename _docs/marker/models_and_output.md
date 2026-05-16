# Models and Output (`marker/models.py`, `marker/output.py`)

## `create_model_dict(device=None, dtype=None, attention_implementation=None)`

Builds the dict that converters expect as `artifact_dict`:

```python
{
    "layout_model":      LayoutPredictor(FoundationPredictor(... LAYOUT_MODEL_CHECKPOINT)),
    "recognition_model": RecognitionPredictor(FoundationPredictor(... RECOGNITION_MODEL_CHECKPOINT)),
    "table_rec_model":   TableRecPredictor(...),
    "detection_model":   DetectionPredictor(...),
    "ocr_error_model":   OCRErrorPredictor(...),
}
```

Sets `PYTORCH_ENABLE_MPS_FALLBACK=1` (Transformers' `.isin` op is not on MPS).
`BaseConverter.resolve_dependencies` matches builder `__init__` arg names
(`layout_model`, `detection_model`, etc.) to keys in this dict.

Load once and reuse — model loading is the slow part of the pipeline. The
batch driver (`scripts/convert.py`) calls this once per worker process.

## `output.py` helpers

- `text_from_rendered(rendered) -> (text, ext, images)` — handles all renderer
  output types: `MarkdownOutput`, `HTMLOutput`, `JSONOutput`, `ChunkOutput`,
  `OCRJSONOutput`, `ExtractionOutput`.
- `save_output(rendered, output_dir, fname_base)` — writes
  `<base>.<ext>`, `<base>_meta.json` (metadata only), and any extracted
  images using `settings.OUTPUT_ENCODING` (UTF-8) and
  `settings.OUTPUT_IMAGE_FORMAT` (JPEG by default).
- `output_exists(output_dir, fname_base)` — checks if a `.md` / `.html` /
  `.json` already exists (used by the batch script to skip done files).
- `json_to_html(block)` — recursively expands the `<content-ref>` markers in a
  JSON block tree into nested HTML; useful when you have JSON output and want
  HTML for an individual block. The renderer base class uses the same idea via
  `extract_block_html`.
- `unwrap_outer_tag(html)` — strips a wrapping `<p>` tag if present.
- `convert_if_not_rgb(image)` — JPEG-safe (RGBA → RGB) before saving.
