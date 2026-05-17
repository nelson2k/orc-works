# Pipelines

[docling/pipeline/](../../repos-folder/docling/docling/pipeline/). All subclass `BasePipeline`, which defines the lifecycle:

```python
execute(in_doc, raises_on_error)
  ├─ _build_document(conv_res)       # produce the DoclingDocument
  ├─ _assemble_document(conv_res)    # finalize structure
  ├─ _enrich_document(conv_res)      # run enrichment_pipe
  ├─ _determine_status(conv_res)
  └─ _unload(conv_res)                # close backends
```

Two thin intermediates:

- `ConvertPipeline` — adds enrichment models that work on any backend (`DocumentPictureClassifier`, picture description, chart extraction).
- `PaginatedPipeline` — for paginated backends; iterates pages in `settings.perf.page_batch_size` batches, runs `self.build_pipe` (a list of per-page callables) over each batch.

## `SimplePipeline`

[simple_pipeline.py](../../repos-folder/docling/docling/pipeline/simple_pipeline.py). 50-line implementation. Used for every declarative backend (Word, PowerPoint, Excel, HTML, Markdown, XML, JSON, CSV, LaTeX, AsciiDoc, WebVTT). `_build_document` literally just calls `conv_res.input._backend.convert()` — the backend already produces a `DoclingDocument`.

## `StandardPdfPipeline`

[standard_pdf_pipeline.py](../../repos-folder/docling/docling/pipeline/standard_pdf_pipeline.py). The flagship. ~900 lines. Multi-threaded, queue-based stage graph designed for parallelism between pipeline stages and model batching.

Stages, wired in order:

1. **preprocess** — `PagePreprocessingModel`. Lazy-loads each page's PDF backend (`PdfPageBackend`), gets size, prepares page images at `images_scale`.
2. **ocr** — chosen via `OcrFactory` from `pipeline_options.ocr_options.kind` (auto / easyocr / tesserocr / tesseract_cli / rapidocr / ocrmac / kserve_v2).
3. **layout** — chosen via `LayoutFactory` from `pipeline_options.layout_options`. Default is the Heron model (`docling-ibm-models`). Other presets: Egret L / M / XL, Heron-101, v2 (legacy).
4. **table** — chosen via `TableStructureFactory` from `pipeline_options.table_structure_options`. Default TableFormer V1 (`docling_tableformer`), or TableFormer V2 (`docling_tableformer_v2`), or `granite_vision_table`. Skipped if `do_table_structure=False`.
5. **assemble** — `PageAssembleModel`. Glues per-page predictions into `AssembledUnit(elements, headers, body)` and runs reading-order assembly via `ReadingOrderModel`.

Each stage is a `ThreadedPipelineStage` running on its own thread, with a bounded `ThreadedQueue` (`queue_max_size` from options). Items carry a per-call `run_id` so concurrent `execute` calls on the same pipeline instance can't cross-contaminate. Stage failures mark `ThreadedItem.is_failed=True` and pass through downstream as no-ops; the orchestrator collects them into `ProcessingResult.failed_pages`.

Document-level timeout: `pipeline_options.document_timeout`. If exceeded mid-run, the input queue is closed, in-flight pages are abandoned, and remaining pages become `RuntimeError("document timeout exceeded")` entries. Final status becomes `PARTIAL_SUCCESS`.

After build, `_assemble_document`:

- builds `conv_res.document` via the reading-order model;
- when `generate_page_images` is set, copies each page image into `document.pages[n].image`;
- when `generate_picture_images` / `generate_table_images` is set, crops every `PictureItem` / `TableItem` from its page image at `images_scale`;
- aggregates per-page `ConfidenceReport.{layout_score, parse_score, table_score, ocr_score}` into document-level scores (parse_score takes the 10th percentile — worst pages dominate);
- adds placeholder entries for failed pages so page numbering remains correct in exports.

Enrichment is added on top: code/formula VLM enrichment (`CodeFormulaVlmModel`), picture classification, picture description, chart extraction.

## `VlmPipeline`

[vlm_pipeline.py](../../repos-folder/docling/docling/pipeline/vlm_pipeline.py). Bypasses OCR/layout/table — feeds the page image to a vision-language model that emits DocTags (or markdown), which is then parsed back into a `DoclingDocument`. Subclass of `PaginatedPipeline`. Two runtime systems:

- "new" runtime when `pipeline_options.vlm_options` is `VlmConvertOptions` — uses `VlmConvertModel`.
- "legacy" path for direct `InlineVlmOptions` / `ApiVlmOptions` — uses `ApiVlmModel`, `HuggingFaceTransformersVlmModel`, or `HuggingFaceMlxModel` directly.

Supports DeepSeek-OCR style outputs via `parse_deepseekocr_markdown`. Keeps the PDF backend alive for the full run (`keep_backend = True`) so cropped page images remain available for enrichment.

## `AsrPipeline`

[asr_pipeline.py](../../repos-folder/docling/docling/pipeline/asr_pipeline.py). Audio in (WAV / MP3 etc.), `DoclingDocument` out. Backend is `NoOpBackend` (audio doesn't need parsing). The pipeline runs a Whisper variant — `mlx-whisper` on Apple Silicon, otherwise `openai-whisper`. Each ASR segment becomes a `TextItem` with provenance pointing to the audio time range. Zero-duration segments get a tiny `ZERO_DURATION_SEGMENT_EPS = 0.001` bump so Docling's validation doesn't reject them.

## `ExtractionVlmPipeline`

[extraction_vlm_pipeline.py](../../repos-folder/docling/docling/pipeline/extraction_vlm_pipeline.py). Used by `DocumentExtractor` (not `DocumentConverter`). Subclass of `BaseExtractionPipeline`. Runs an extraction-tuned VLM (e.g. `nuextract_2b`) over each page with a user-provided schema/template; merges per-page outputs into a single structured `ExtractionResult`.

## `legacy_standard_pdf_pipeline.py`

The pre-threaded PDF pipeline kept around for compatibility. Not selected by default. Same stage order, sequential.
