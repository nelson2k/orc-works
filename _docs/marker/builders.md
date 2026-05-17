# Builders

Builders construct the `Document` and fill in layout / lines / OCR / structure. All in [marker/builders/](../../repos-folder/marker/marker/builders/). All subclass `BaseBuilder` (just `assign_config` + a `__call__` stub).

## DocumentBuilder

[document.py](../../repos-folder/marker/marker/builders/document.py). Renders low-res (96 DPI) and high-res (192 DPI) page images, creates one `PageGroup` per page, then runs:

```
layout_builder(document, provider)
line_builder(document, provider)
if not disable_ocr:
    ocr_builder(document, provider)
```

Config: `lowres_image_dpi=96`, `highres_image_dpi=192`, `disable_ocr=False`.

## LayoutBuilder

[layout.py](../../repos-folder/marker/marker/builders/layout.py). Runs surya `LayoutPredictor` on lowres page images, batched (12 on CUDA, 6 elsewhere). Each detected box becomes a typed `Block` (Text, Picture, Table, …) added to the page's `structure` in reading order.

- `force_layout_block` — skip the model, force every page to a single block type. Used by table-only flows.
- `expand_block_types` — `Picture`, `Figure`, `ComplexRegion` get their boxes expanded up to `max_expand_frac=0.05` toward neighbours so cropped images aren't clipped.
- `top_k` predictions are stored on each block (used by `BlockRelabelProcessor` later).

## LineBuilder

[line.py](../../repos-folder/marker/marker/builders/line.py). Decides per page: pdftext or surya OCR? Steps:

1. `OCRErrorPredictor` on joined provider text labels each page good/bad.
2. For each page: pass requires lines exist, ocr_error label is good, ≥25% of non-image layout blocks intersect provider lines, and lines don't overflow the page or self-intersect.
3. For "good" pages: keep pdftext provider lines verbatim, mark `text_extraction_method="pdftext"`.
4. For "bad" pages: run surya `DetectionPredictor` on the page image (with `Table`/`Form`/`TOC` masked white to avoid double-OCR), keep boxes with confidence ≥0.8, store as line stubs to be filled in by OcrBuilder.
5. `filter_blank_lines` drops boxes whose crop is blank (invisible text artifacts).

Tunables: `layout_coverage_threshold=0.25`, `detection_line_min_confidence=0.8`, `min_document_ocr_threshold=0.85`, `disable_ocr`.

## OcrBuilder

[ocr.py](../../repos-folder/marker/marker/builders/ocr.py). Runs surya `RecognitionPredictor` only on pages where `text_extraction_method=="surya"`. Skips `Equation`, `Figure`, `Picture`, `Table`, `Form`, `TableOfContents` (those have their own processors). For some block types (`SectionHeader`, `ListItem`, `Footnote`, `Text`, `TextInlineMath`, `Code`, `Caption`) it runs OCR at the block level instead of per-line, falling back to line mode for tall/multi-line/overlapping blocks.

Returns characters with HTML-style format tags (`<b>`, `<i>`, `<math>`, `<br>`) which it parses into `Span`s with `formats`. Hyperlinks from the original spans are re-injected by matching text. `keep_chars=True` stores per-character boxes.

## StructureBuilder

[structure.py](../../repos-folder/marker/marker/builders/structure.py). Two grouping passes:

- `group_caption_blocks`: any `Table`/`Figure`/`Picture` with a `Caption` or `Footnote` directly above or below (gap < 5% of page height) gets wrapped in `TableGroup` / `FigureGroup` / `PictureGroup`.
- `group_lists`: contiguous `ListItem`s with gap < 10% of page height get wrapped in `ListGroup`.
- `unmark_lists`: any leftover ungrouped `ListItem` is demoted to plain `Text`.
