# Docling Models

Docling model stages live under `docling/models/stages/`, with factory helpers
under `docling/models/factories/`.

## PDF build stages

The threaded PDF pipeline wires these stage objects:

- `PagePreprocessingModel` - prepares rendered page state and backend data.
- OCR model from `get_ocr_factory(...)` - selected from OCR options.
- Layout model from `get_layout_factory(...)` - document layout detection.
- Table model from `get_table_structure_factory(...)` - table reconstruction.
- `PageAssembleModel` - assembles page-level predictions into structured page data.
- `ReadingOrderModel` - builds the final ordered document.

## OCR engines

Docling has multiple OCR option classes and implementations:

- auto OCR
- RapidOCR
- EasyOCR
- Tesseract CLI
- Tesseract Python bindings
- macOS OCR
- KServe V2 OCR

`RapidOcrOptions` defaults to the `onnxruntime` backend. `OcrAutoOptions`
chooses based on runtime availability.

## Enrichment

After document assembly, `ConvertPipeline` can run enrichment models:

- picture classification
- picture description
- chart extraction
- code/formula enrichment

These are optional and controlled by pipeline options.

## Model location

Pipelines use `artifacts_path` from pipeline options or global settings. If set,
it must be a directory containing required model artifacts.
