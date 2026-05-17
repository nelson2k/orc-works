# Datamodel

[docling/datamodel/](../../repos-folder/docling/docling/datamodel/). Pydantic models defining everything that flows through the pipeline. Note: the public `DoclingDocument` itself comes from the `docling-core` package, not from here — this folder holds the *converter's* internal types plus the option models.

## Core conversion types

[base_models.py](../../repos-folder/docling/docling/datamodel/base_models.py):

- `InputFormat` — string enum: `DOCX`, `PPTX`, `HTML`, `IMAGE`, `PDF`, `ASCIIDOC`, `MD`, `CSV`, `XLSX`, `XML_USPTO`, `XML_JATS`, `XML_XBRL`, `METS_GBS`, `JSON_DOCLING`, `AUDIO`, `VTT`, `LATEX`.
- `OutputFormat` — `md` / `json` / `yaml` / `html` / `html_split_page` / `text` / `doctags` / `vtt`.
- `FormatToExtensions` / `MimeTypeToFormat` / `FormatToMimeType` — mapping tables.
- `ConversionStatus` — `pending` / `started` / `failure` / `success` / `partial_success` / `skipped`.
- `DoclingComponentType` — error categories: `USER_INPUT`, `DOCUMENT_BACKEND`, `PIPELINE`, `DOC_ASSEMBLER`.
- `ErrorItem(component_type, module_name, error_message)` — what populates `ConversionResult.errors`.
- `Page(page_no, size, image, …)` — the per-page intermediate. Holds `_backend: PdfPageBackend`, `parsed_page: SegmentedPdfPage`, `predictions`, `assembled`, `_image_cache`.
- `AssembledUnit(elements, headers, body)` — output of the assemble stage.
- `ConfidenceReport(layout_score, parse_score, table_score, ocr_score, pages: dict[int, PageConfidence])` — quality metrics.
- `BaseFormatOption` — parent of `FormatOption` (in `document_converter.py`) and `ExtractionFormatOption`. Holds `pipeline_options` + `backend` class.
- `DocumentStream` — re-exported from `docling-core.types.io`; pairs a `name` with a `BytesIO`.

## `document.py`

[document.py](../../repos-folder/docling/docling/datamodel/document.py) defines `InputDocument`, `_DocumentConversionInput`, `ConversionResult`, and the format-detection logic.

- `InputDocument(file, document_hash, valid, format, _backend, …)` — what the converter passes to a pipeline.
- `ConversionResult(input, status, errors, pages, assembled, document, confidence, timings)` — what comes back.
- `_DocumentConversionInput.docs(format_to_options)` — yields `InputDocument` instances from an iterable of paths/streams; this is where format detection happens (extension match → magic-byte sniff → fallback to extension).
- `DoclingVersion` — gathers versions of `docling`, `docling-core`, `docling-ibm-models`, `docling-parse`, Python, platform — used by `docling --version`.
- A large `layout_label_to_ds_type` mapping bridges `DocItemLabel` (the new label enum) to the legacy `ds_type` strings, used by the legacy-export path.

## `settings.py`

[settings.py](../../repos-folder/docling/docling/datamodel/settings.py). Top-level pydantic-settings config:

```python
AppSettings(
    perf = BatchConcurrencySettings(
        doc_batch_size=1, doc_batch_concurrency=1,
        page_batch_size=4, page_batch_concurrency=1,
        elements_batch_size=16,
    ),
    debug = DebugSettings(
        visualize_cells=False, visualize_ocr=False,
        visualize_layout=False, visualize_raw_layout=False,
        visualize_tables=False, profile_pipeline_timings=False,
        debug_output_path=str(Path.cwd() / "debug"),
    ),
    inference = InferenceSettings(compile_torch_models=True),
    cache_dir = Path.home() / ".cache" / "docling",
    artifacts_path = None,                  # override model artifact location
)
```

Env-prefix is `DOCLING_`, nested keys join with `_` (e.g. `DOCLING_PERF_PAGE_BATCH_SIZE=8`).

`PageRange = Annotated[Tuple[int, int], AfterValidator(...)]` enforces 1-indexed, `end >= start`. `DEFAULT_PAGE_RANGE = (1, sys.maxsize)`. `DocumentLimits(max_num_pages, max_file_size, page_range)` collects per-document limits.

## `accelerator_options.py`

[accelerator_options.py](../../repos-folder/docling/docling/datamodel/accelerator_options.py). `AcceleratorOptions(num_threads, device, cuda_use_flash_attention2)`. `AcceleratorDevice` enum: `auto`, `cpu`, `cuda`, `mps`, `xpu`. `cuda:N` is accepted as a string. Reads env: `DOCLING_NUM_THREADS` (or `OMP_NUM_THREADS`), `DOCLING_DEVICE`, `DOCLING_CUDA_USE_FLASH_ATTENTION2`.

## `pipeline_options.py`

[pipeline_options.py](../../repos-folder/docling/docling/datamodel/pipeline_options.py). The big one. Each `BaseOptions` subclass has a `kind: ClassVar[str]` discriminator for polymorphic deserialization.

Top-level option classes:

- `PipelineOptions` — common: `artifacts_path`, `accelerator_options`, `enable_remote_services`, `allow_external_plugins`, `document_timeout`, `kserve_options`.
- `ConvertPipelineOptions` — adds enrichment toggles (`do_picture_classification`, `do_picture_description`, `do_chart_extraction`) and their option sub-objects.
- `PdfPipelineOptions` / `ThreadedPdfPipelineOptions` — for the standard PDF pipeline: `do_ocr`, `do_table_structure`, `do_code_enrichment`, `do_formula_enrichment`, `ocr_options`, `layout_options`, `table_structure_options`, `code_formula_options`, `images_scale`, `generate_page_images`, `generate_picture_images`, `generate_table_images`, `generate_parsed_pages`, `ocr_batch_size`, `layout_batch_size`, `table_batch_size`, `queue_max_size`, `batch_polling_interval_seconds`.
- `VlmPipelineOptions` — for VLM-as-converter: `vlm_options: ApiVlmOptions | InlineVlmOptions | VlmConvertOptions`.
- `AsrPipelineOptions` — Whisper preset selection.

OCR / table / layout option families:

- `OcrOptions` (abstract) → `OcrAutoOptions`, `EasyOcrOptions`, `RapidOcrOptions`, `TesseractOcrOptions`, `TesseractCliOcrOptions`, `OcrMacOptions`, `KserveV2OcrOptions`.
- `BaseTableStructureOptions` → `TableStructureOptions`, `TableStructureV2Options`, `GraniteVisionTableStructureOptions`.

`PdfBackend` enum + `normalize_pdf_backend()` for CLI-to-class mapping (`dlparse_v1` / `dlparse_v2` / `dlparse_v4` / `pypdfium2`).

## `extraction.py` and `extraction_options.py`

[extraction.py](../../repos-folder/docling/docling/datamodel/extraction.py) and [extraction_options.py](../../repos-folder/docling/docling/datamodel/extraction_options.py). Types for `DocumentExtractor`: `ExtractionResult`, `ExtractionTemplateType`, `ExtractionPromptStyle`.

## Model spec modules

- `asr_model_specs.py` — Whisper presets.
- `vlm_model_specs.py` — generic VLM presets (Granite Vision, SmolDocling, NuExtract).
- `stage_model_specs.py` — VLM presets per stage (chart extraction, table structure, code/formula).
- `layout_model_specs.py` — `DOCLING_LAYOUT_HERON` etc.
- `pipeline_options_vlm_model.py` — `ApiVlmOptions`, `InlineVlmOptions`, `InferenceFramework`, `ResponseFormat`, `TransformersPromptStyle`.
- `pipeline_options_asr_model.py` — `InlineAsrNativeWhisperOptions`, `InlineAsrMlxWhisperOptions`.
- `vlm_engine_options.py`, `chart_extraction_options.py`, `picture_classification_options.py`, `image_classification_engine_options.py`, `object_detection_engine_options.py` — area-specific options.

## `service/` subpackage

[service/](../../repos-folder/docling/docling/datamodel/service/) — Pydantic request/response models used by the optional service-client layer ([docling/service_client/](../../repos-folder/docling/docling/service_client/)) for talking to remote KServe v2 / Triton inference endpoints.
