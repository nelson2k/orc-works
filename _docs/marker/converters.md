# Converters (`marker/converters/`)

The top-level orchestrator. A converter owns a renderer, an LLM service, and
the ordered processor list, and it knows how to assemble a `Document` from a
file.

`BaseConverter` (in `converters/__init__.py`) provides:

- `assign_config` to inject CLI/JSON config values onto attributes.
- `resolve_dependencies(cls)` — inspects a class `__init__` signature and pulls
  values from `self.artifact_dict` (models, the LLM service, etc.) or
  `self.config`. This is how processors / builders / renderers are constructed
  without an explicit DI container.
- `initialize_processors(...)` — wraps all `BaseLLMSimpleBlockProcessor`
  instances behind a single `LLMSimpleBlockMetaProcessor` so per-block prompts
  are batched.

## PdfConverter (`pdf.py`)

The default converter. Used for everything that ends up in markdown / json /
html / chunks via the standard pipeline. Key knobs:

- `override_map: Dict[BlockTypes, Type[Block]]` — swap in custom block classes.
- `use_llm: bool` — turn on the LLM-backed processors.
- `default_processors` — long ordered tuple of processors; see
  [processors.md](processors.md).
- `default_llm_service` — `GoogleGeminiService`.

Flow: `__call__(filepath)` → `build_document` (provider → builders → processors)
→ render. Accepts `str` or `io.BytesIO` (BytesIO is written to a temp `.pdf`).

## TableConverter (`table.py`)

Subclass of `PdfConverter` that only keeps `Table`, `Form`, and
`TableOfContents` blocks. Disables OCR in the document builder. Processor list
is reduced to table / form / complex-region. Pair with
`--force_layout_block Table` to treat every page as a table.

## OCRConverter (`ocr.py`)

Subclass of `PdfConverter` that forces OCR (`force_ocr=True`) and outputs
`OCRJSONRenderer` results (per-line bboxes + text). Only runs the
`EquationProcessor`. Use `--keep_chars` to keep per-character info.

## ExtractionConverter (`extraction.py`)

For structured (JSON-schema) extraction. Runs the full markdown pipeline,
re-paginates the markdown (`{n}-{48 dashes}` separator), then runs:

- `PageExtractor` — per-page notes via the LLM.
- `DocumentExtractor` — merges page notes into a final structured result.
- `ExtractionRenderer` — combines that with the original markdown.

Set `existing_markdown` in config to skip re-parsing.
