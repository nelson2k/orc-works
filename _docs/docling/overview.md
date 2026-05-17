# Docling Overview

Source: `repos-folder/docling`.

## Package

- Project name: `docling-slim`
- Version in checkout: `2.93.0`
- License: MIT
- Python: `>=3.10,<4.0`
- Main user API: `docling.document_converter.DocumentConverter`
- CLI scripts: `docling`, `docling-tools`

## Main folders

- `docling/backend/` - format-specific readers/backends.
- `docling/pipeline/` - conversion pipelines.
- `docling/models/` - model stages and factories.
- `docling/datamodel/` - options, statuses, inputs, conversion results.
- `docling/service_client/` - client/job helpers for service use.
- `docling/utils/` - profiling, export, OCR helpers, visualization.

## Input formats

`DocumentConverter` maps formats to backends and pipelines. Notable defaults:

- PDF and images use `StandardPdfPipeline`.
- DOCX, PPTX, XLSX, HTML, Markdown, CSV, XML-ish formats use `SimplePipeline`.
- Audio uses `AsrPipeline`.
- PDF default backend is `DoclingParseDocumentBackend`.

## Output model

Conversions return `ConversionResult`, which holds:

- `input`
- `status`
- `errors`
- per-page data
- assembled intermediate units
- final `DoclingDocument`

The final document is exported through `docling-core` APIs, for example
`result.document.export_to_markdown()`.
