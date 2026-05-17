# Converters

All in [marker/converters/](../../repos-folder/marker/marker/converters/). All subclass `BaseConverter`, which:

- stores config on `self`,
- exposes `resolve_dependencies(cls)` — inspects `__init__` and injects matching keys from `artifact_dict` (surya models, `llm_service`) plus `config`,
- and `initialize_processors(list)` — wraps all `BaseLLMSimpleBlockProcessor`s in a single `LLMSimpleBlockMetaProcessor` so they batch.

## `PdfConverter` — default

[converters/pdf.py](../../repos-folder/marker/marker/converters/pdf.py). The full pipeline: providers → DocumentBuilder → StructureBuilder → ~25 processors → MarkdownRenderer by default.

Important constructor args (passed by `convert_single_cli`):
- `artifact_dict` — dict of preloaded surya predictors from `create_model_dict()`.
- `config` — flat dict merged onto every builder/processor/service via `assign_config`. Common keys: `use_llm`, `force_ocr`, `page_range`, `output_format`, `debug_*`, plus any `Annotated` field from any pipeline class.
- `processor_list` — full module paths; overrides `default_processors`.
- `renderer` — full module path; overrides the markdown renderer.
- `llm_service` — full module path; only resolved if set, otherwise resolved from default when `use_llm` is on.

Exposes `page_count` after a call.

## `TableConverter`

[converters/table.py](../../repos-folder/marker/marker/converters/table.py). Subclass of `PdfConverter`. Trims `page.structure` to only `Table` / `Form` / `TableOfContents` blocks before running a much shorter processor list (`TableProcessor`, the LLM table/form/complex processors). Disables OCR in the document builder — assumes tables come from native PDF text or from forced-layout boxes. Combine with `force_layout_block=Table` to treat every page as one big table.

## `OCRConverter`

[converters/ocr.py](../../repos-folder/marker/marker/converters/ocr.py). Forces `force_ocr=True` and swaps the renderer to `OCRJSONRenderer`. Only equation processor runs. Output is per-line OCR text plus polygons. Useful for "give me bounding boxes and characters" without any layout-aware reformatting. Pass `keep_chars=True` to keep individual char boxes.

## `ExtractionConverter`

[converters/extraction.py](../../repos-folder/marker/marker/converters/extraction.py). Two-phase:

1. Runs the full `PdfConverter` pipeline forced to markdown + paginated output (or reuses `existing_markdown` if provided).
2. Splits markdown on the page separator, fans out to `PageExtractor` (per-page LLM call), then `DocumentExtractor` (merge), then `ExtractionRenderer` produces `ExtractionOutput.document_json`.

Requires an LLM service. The `page_schema` config key drives the schema the model is asked to fill.
