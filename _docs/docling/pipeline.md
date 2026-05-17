# Docling Pipeline

The high-level call path starts at `DocumentConverter.convert(...)`.

```text
DocumentConverter
  -> InputDocument
  -> backend for format
  -> pipeline for format
  -> ConversionResult
  -> DoclingDocument
```

## Pipeline cache

`DocumentConverter` caches initialized pipelines by `(pipeline class,
pipeline_options_hash)`. Heavy models are reused when options match.

## Base pipeline lifecycle

`BasePipeline.execute(...)` runs:

1. `_build_document`
2. `_assemble_document`
3. `_enrich_document`
4. `_determine_status`
5. `_unload`

Errors are converted into `ConversionStatus.FAILURE` when
`raises_on_error=False`, otherwise they are raised.

## Standard PDF pipeline

The checked-out `StandardPdfPipeline` is threaded. It builds a per-document run
context with bounded queues and stages:

```text
preprocess -> ocr -> layout -> table -> assemble
```

Each stage can batch pages, apply backpressure, and pass `ThreadedItem`
envelopes downstream. The final assembly builds reading order and the
`DoclingDocument`.

## Useful PDF options

`PdfPipelineOptions` / `ThreadedPdfPipelineOptions` include:

- `do_ocr`
- `do_table_structure`
- `do_code_enrichment`
- `do_formula_enrichment`
- `generate_page_images`
- `generate_picture_images`
- `generate_parsed_pages`
- `document_timeout`
- `ocr_batch_size`
- `layout_batch_size`
- `table_batch_size`
- `queue_max_size`
