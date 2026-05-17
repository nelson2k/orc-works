# docling — notes

Source: `repos-folder/docling` (docling-project, originally IBM Research Zurich). Package: `docling-slim` on PyPI, version `2.93.0`. License: MIT (code); per-model licenses vary. Now part of the LF AI & Data Foundation.

What it is: a Python SDK + CLI that parses many document formats — PDF, DOCX, PPTX, XLSX, HTML, Markdown, LaTeX, plain text, images, audio (ASR), WebVTT, XML (USPTO / JATS / XBRL), CSV, AsciiDoc — into a single unified `DoclingDocument` representation (from the `docling-core` package), and exports back to Markdown, HTML, JSON, YAML, DocTags, plain text, or WebVTT.

Shape of the pipeline:

1. `DocumentConverter` (in `docling/document_converter.py`) is the main entry point. You hand it a file path, URL, or in-memory `DocumentStream`.
2. It sniffs the `InputFormat` and picks a `(backend, pipeline)` pair from `format_to_options`.
3. The backend opens the file. Backends fall into two categories: **declarative** (transforms the source straight into a `DoclingDocument` — Office, HTML, Markdown, XML, …) and **paginated** (PDF/image — exposes pages on demand).
4. The pipeline runs:
   - `SimplePipeline` for declarative backends — just calls `backend.convert()`.
   - `StandardPdfPipeline` for paginated backends — a multi-threaded stage graph (preprocess → OCR → layout → table → assemble) followed by reading-order assembly.
   - `VlmPipeline` for vision-language-model conversion (e.g. `granite_docling`, `smoldocling`).
   - `AsrPipeline` for audio.
5. Optional enrichment passes (`do_picture_description`, `do_picture_classification`, `do_chart_extraction`, `do_code_enrichment`, `do_formula_enrichment`) run on the `DoclingDocument`.
6. `ConversionResult` is returned with `.document` (the `DoclingDocument`), `.status`, `.errors`, `.confidence`, and profiling data. You then call `result.document.export_to_markdown()` / `save_as_html()` / `save_as_json()` etc.

Two parallel converters exist:

- `DocumentConverter` — produces `DoclingDocument`.
- `DocumentExtractor` (in `docling/document_extractor.py`) — structured extraction beta: takes the same inputs plus a schema/template, runs a VLM, returns an `ExtractionResult`.

Plus a built-in chunking module (`docling/chunking/__init__.py`) re-exporting `HierarchicalChunker` / `HybridChunker` from `docling-core` for RAG use cases.
