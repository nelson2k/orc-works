# Builders (`marker/builders/`)

A builder takes the empty / partial `Document` and fills it in with the help of
a provider and/or a surya model. `BaseBuilder.__call__(data, ...)` is the
entry point.

## DocumentBuilder (`document.py`)

Creates the initial `Document` with one `PageGroup` per page, populated with
low-res (`lowres_image_dpi=96`) and high-res (`highres_image_dpi=192`) page
images, page polygons, and `pdftext` references. Then it sequentially invokes
`LayoutBuilder`, `LineBuilder`, and (unless `disable_ocr=True`) `OcrBuilder`.

## LayoutBuilder (`layout.py`)

Runs `surya.layout.LayoutPredictor` on low-res page images and adds the
resulting block boxes to each page's `structure`. Notables:

- `force_layout_block` — bypasses the model and assigns every page to one
  block type (used by `TableConverter` for table-only pages).
- `expand_block_types` — `Picture`, `Figure`, `ComplexRegion`. Their polygons
  are expanded up to `max_expand_frac` (0.05) into nearby whitespace to catch
  missed margins.
- Batch size auto-picks 12 on CUDA, 6 otherwise; override with
  `layout_batch_size`.

## LineBuilder (`line.py`)

Reconciles surya line detection with provider-supplied lines.

- Uses `DetectionPredictor` for line boxes and `OCRErrorPredictor` to decide
  per-page whether the provider's existing text is trustworthy.
- `layout_coverage_min_lines` / `layout_coverage_threshold` —  thresholds for
  accepting provider lines.
- `min_document_ocr_threshold` (0.85) — if fewer than this fraction of pages
  look good, the document is OCR'd; otherwise OCR is skipped.
- `ocr_remove_blocks` — Table / Form / TableOfContents lines are stripped here
  because they are re-OCR'd later by the table processor.

## OcrBuilder (`ocr.py`)

Runs `surya.recognition.RecognitionPredictor` on the blocks flagged for OCR
(see `skip_ocr_blocks` for the exceptions). Modes:

- **Line mode** (default) — OCR each detected line region.
- **Block mode** — OCR whole-block regions for `full_ocr_block_types` (Section
  headers, list items, paragraphs, …) when the block is small and isolated.
  Falls back to line mode if too tall, too many lines, or overlapping
  neighbours.

Reconstructs `Span` objects from surya's HTML-like character stream (handles
`<math>`, formatting tags, `<br>`) and re-applies link URLs from the provider
spans via `replace_line_spans`.

## StructureBuilder (`structure.py`)

Pure-Python grouping pass (no models):

- `group_caption_blocks` — wraps a Figure / Picture / Table with adjacent
  `Caption` / `Footnote` blocks (within `gap_threshold * page_height`) into a
  `FigureGroup` / `PictureGroup` / `TableGroup`.
- `group_lists` — packs consecutive `ListItem`s into a `ListGroup`.
- `unmark_lists` — converts isolated `ListItem`s back to `Text`.
