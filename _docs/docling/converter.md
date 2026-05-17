# DocumentConverter

[docling/document_converter.py](../../repos-folder/docling/docling/document_converter.py). The main programmatic entry point.

## Construction

```python
DocumentConverter(
    allowed_formats: list[InputFormat] | None = None,
    format_options: dict[InputFormat, FormatOption] | None = None,
)
```

- `allowed_formats` — restricts which formats this converter will accept. Anything outside the list errors (or, with `raises_on_error=False`, returns `ConversionStatus.SKIPPED`).
- `format_options` — overrides the default `(pipeline, backend, options)` tuple per format. The class enforces one quirk: passing a non-`ImageDocumentBackend` backend under `InputFormat.IMAGE` raises a `DeprecationWarning` and is auto-corrected.

If you don't pass `format_options`, the converter calls `_get_default_option(format)` for each allowed format. The defaults:

| Format | Pipeline | Backend |
|---|---|---|
| `CSV` | SimplePipeline | CsvDocumentBackend |
| `XLSX` | SimplePipeline | MsExcelDocumentBackend |
| `DOCX` | SimplePipeline | MsWordDocumentBackend |
| `PPTX` | SimplePipeline | MsPowerpointDocumentBackend |
| `MD` | SimplePipeline | MarkdownDocumentBackend |
| `ASCIIDOC` | SimplePipeline | AsciiDocBackend |
| `HTML` | SimplePipeline | HTMLDocumentBackend |
| `XML_USPTO` | SimplePipeline | PatentUsptoDocumentBackend |
| `XML_JATS` | SimplePipeline | JatsDocumentBackend |
| `XML_XBRL` | SimplePipeline | XBRLDocumentBackend |
| `METS_GBS` | StandardPdfPipeline | MetsGbsDocumentBackend |
| `IMAGE` | StandardPdfPipeline | ImageDocumentBackend |
| `PDF` | StandardPdfPipeline | DoclingParseDocumentBackend |
| `JSON_DOCLING` | SimplePipeline | DoclingJSONBackend |
| `AUDIO` | AsrPipeline | NoOpBackend |
| `VTT` | SimplePipeline | WebVTTDocumentBackend |
| `LATEX` | SimplePipeline | LatexDocumentBackend |

(VLM swap-in: pass a `PdfFormatOption(pipeline_cls=VlmPipeline, pipeline_options=VlmPipelineOptions(...))`.)

## Methods

### `convert(source, …) -> ConversionResult`

Single-source convenience wrapper. `source` is one of `Path`, `str` (path or URL), or `DocumentStream`. Internally calls `convert_all([source])` and returns the first result.

Optional kwargs:

- `headers: dict[str, str] | None` — HTTP headers for URL sources.
- `raises_on_error: bool = True` — toggle exception vs in-result error.
- `max_num_pages: int = sys.maxsize` — skip if larger.
- `max_file_size: int = sys.maxsize` — skip if larger (bytes).
- `page_range: tuple[int, int] = (1, sys.maxsize)` — inclusive 1-indexed.

### `convert_all(sources, …) -> Iterator[ConversionResult]`

Generator. Drives the document-batching machinery (`settings.perf.doc_batch_size`, `settings.perf.doc_batch_concurrency` — both default 1 but support free-threaded execution).

If `raises_on_error=True` and any result is not `SUCCESS`/`PARTIAL_SUCCESS`, raises `ConversionError` with the collected error messages.

### `convert_string(content, format, name=None) -> ConversionResult`

Wraps a string into a `DocumentStream`. Only `InputFormat.MD` and `InputFormat.HTML` accepted; anything else is a `ValueError`. `name` defaults to a UTC timestamp; the correct extension is appended.

### `initialize_pipeline(format)`

Eagerly construct the pipeline for a format so first-call latency is paid up front. Raises `ConversionError` if no pipeline is configured, `RuntimeError` if `artifacts_path` is invalid, `FileNotFoundError` if local model files are missing.

## Pipeline caching

`DocumentConverter` keeps a `(pipeline_class, options_hash)` cache so multiple format options that share a pipeline class + identical pipeline_options share a single in-memory model instance. The lock is module-level (`_PIPELINE_CACHE_LOCK`), so multiple converters in the same process don't share, but a single converter is safe under threading.

`_get_pipeline_options_hash` is MD5 of `pipeline_options.model_dump()` — purely a cache key, not security-sensitive.

## What's actually returned

`ConversionResult` (defined in [docling/datamodel/document.py](../../repos-folder/docling/docling/datamodel/document.py)) contains:

- `input: InputDocument` — file metadata, backend handle, format, document hash.
- `status: ConversionStatus`.
- `errors: list[ErrorItem]` — each tagged with a `DoclingComponentType` (`USER_INPUT`, `DOCUMENT_BACKEND`, `PIPELINE`, `DOC_ASSEMBLER`, …).
- `pages: list[Page]` — only paginated pipelines populate this.
- `assembled: AssembledUnit` — intermediate (elements / headers / body).
- `document: DoclingDocument` — the final unified representation.
- `confidence: ConfidenceReport` — quality metrics.
- `timings: dict[str, ProfilingItem]` — per-stage durations (when `settings.debug.profile_pipeline_timings`).
