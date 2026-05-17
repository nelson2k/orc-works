# Structured extraction (beta)

Parallel to `DocumentConverter`, docling exposes a `DocumentExtractor` that runs a VLM with a user-provided JSON schema and returns the extracted structured data. Marked beta in the README.

## Entry point

[docling/document_extractor.py](../../repos-folder/docling/docling/document_extractor.py).

```python
from docling.document_extractor import DocumentExtractor, ExtractionFormatOption
from docling.datamodel.extraction import ExtractionTemplate

extractor = DocumentExtractor()
result = extractor.extract(
    source="invoice.pdf",
    template=ExtractionTemplate(schema={...}),
)
print(result.records)
```

`DocumentExtractor` mirrors `DocumentConverter`'s shape: an `allowed_formats` + `format_options` constructor, then `.extract(source)` / `.extract_all(iterable)` methods. Each result is an `ExtractionResult` (defined in `datamodel/extraction.py`).

Default backends per format (from `_get_default_extraction_option`):

| InputFormat | Backend |
|---|---|
| `PDF` | `PyPdfiumDocumentBackend` |
| `IMAGE` | `ImageDocumentBackend` |

Other formats currently raise `RuntimeError("No default extraction option configured for ...")`. Pass an explicit `ExtractionFormatOption(backend=..., pipeline_cls=...)` to extend.

## Pipeline

[docling/pipeline/extraction_vlm_pipeline.py](../../repos-folder/docling/docling/pipeline/extraction_vlm_pipeline.py). `ExtractionVlmPipeline` extends `BaseExtractionPipeline` ([base_extraction_pipeline.py](../../repos-folder/docling/docling/pipeline/base_extraction_pipeline.py)).

Flow:

1. Backend rasterizes each page to a PIL image.
2. The pipeline calls a VLM (default preset `NU_EXTRACT_2B_TRANSFORMERS` from `vlm_model_specs.py`) with the user's prompt/schema and the page image.
3. The VLM returns JSON, which is validated against the schema.
4. Per-page records are merged into a single `ExtractionResult`.

## Options

[docling/datamodel/extraction_options.py](../../repos-folder/docling/docling/datamodel/extraction_options.py):

- `ExtractionPipelineOptions` — pipeline-level (extends `PipelineOptions`): `vlm_options`, `accelerator_options`, `template_max_pages`.
- `ExtractionTemplateType` — `JSON_SCHEMA` / `EXAMPLE` / `KEY_VALUE`.
- `ExtractionPromptStyle` — controls how the schema is rendered into the prompt.

## Extraction models

[docling/models/extraction/](../../repos-folder/docling/docling/models/extraction/):

- `nuextract_transformers_model.py` — NuExtract 2B running via transformers. The default.
- `transformers_extraction_model.py` — generic transformers wrapper for other extraction-tuned VLMs.
- `prompt_utils.py` — turns a JSON schema (or examples, or key-value template) into a prompt the model understands.
