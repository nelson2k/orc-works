# Extraction pipeline

Module: [pdftext/extraction.py](../../repos-folder/pdftext/pdftext/extraction.py)

The public API. Backs both the CLI and library use. Internally drives [pdf/pages.py](../../repos-folder/pdftext/pdftext/pdf/pages.py) which orchestrates per-page extraction through pdfium.

## Public functions

```python
plain_text_output(pdf_path, sort=False, hyphens=False,
                  page_range=None, flatten_pdf=False, workers=None) -> str
paginated_plain_text_output(pdf_path, ...) -> List[str]
dictionary_output(pdf_path, sort=False, page_range=None,
                  keep_chars=False, flatten_pdf=False,
                  quote_loosebox=True, disable_links=False,
                  workers=None) -> Pages
table_output(pdf_path, table_inputs, page_range=None,
             flatten_pdf=False, quote_loosebox=True,
             workers=None, pages=None) -> List[Tables]
```

| Function | Returns |
|---|---|
| `plain_text_output` | Single string, pages joined by `\n` |
| `paginated_plain_text_output` | List of per-page strings |
| `dictionary_output` | Structured `Pages` list (see schema.md) |
| `table_output` | Per-page lists of `TableCell` text for the supplied table bboxes |

## Per-page pipeline

`extraction._get_pages` is the entry point that dispatches single-process or multi-process extraction:

1. `_load_pdf(pdf_path, flatten_pdf)` — open the PDF via pypdfium2. If `flatten_pdf=True`, call `pdf.init_forms()` so form fields render into the page content.
2. If `workers > 1` and there are enough pages (`>= settings.WORKER_PAGE_THRESHOLD * workers`), shard the page list across a `ProcessPoolExecutor` (one PDF handle per worker via `worker_init`).
3. For each page range, call `get_pages(pdf_doc, page_range, flatten_pdf, quote_loosebox)`.

## Inside `get_pages` ([pdf/pages.py](../../repos-folder/pdftext/pdftext/pdf/pages.py))

For each page index:

1. **Char extraction** — `chars.get_chars(textpage, page_bbox, page_rotation, quote_loosebox)` walks the pdfium `PdfTextPage`, reads each char via `get_char_box4`, and produces a list of:
   ```python
   {"bbox": Bbox, "char": str, "rotation": float, "font": {...}, "char_idx": int}
   ```
   `quote_loosebox` controls whether quote-mark bboxes get loosened (pdfium reports quote glyphs with tight bboxes that don't include the trailing/leading whitespace).
2. **Char deduplication** — `chars.deduplicate_chars(chars)` removes near-duplicate glyphs (e.g. fake-bold overlays where the same char is drawn twice).
3. **Span grouping** — `pages.get_spans(chars)` groups consecutive chars with the same font into a `Span` (a "run" of text). The `font` dict matches the first char in the run. Span text comes from concatenating each char's `char` field.
4. **Script detection** — `pages.assign_scripts(lines)` flags `Span.superscript` / `Span.subscript` based on vertical offset, height ratio, and whether the span text is a single char / digit / math symbol. Heuristics: `height_threshold=0.8`, `line_distance_threshold=0.1`.
5. **Line grouping** — `pages.get_lines(spans)` groups spans by line (y-overlap + horizontal gap thresholds).
6. **Block grouping** — `pages.get_blocks(lines)` groups lines into blocks (paragraph-like clusters).

## Links + references

[pdf/links.py](../../repos-folder/pdftext/pdftext/pdf/links.py) adds:

- `Span.url` — populated when a link annotation's bbox overlaps with the span's bbox.
- `Page.refs` — `Reference(idx, page, coord)` entries for in-document links (`/Dest` references). Used by downstream consumers (e.g. citation resolution).

`dictionary_output(..., disable_links=False)` enables this step; the CLI disables it for JSON output by default.

## Text post-processing

[postprocessing.py](../../repos-folder/pdftext/pdftext/postprocessing.py)

- `postprocess_text(s)` — chains `replace_control_chars`, `replace_special_chars`, `replace_ligatures` (e.g. `ﬁ → fi`, `ﬂ → fl`).
- `handle_hyphens(s, keep_hyphens=False)` — strip end-of-line hyphens and join the broken word.
- `sort_blocks(blocks, tolerance=1.25)` — re-sort blocks for reading order using y-band clustering.
- `merge_text(page, sort=False, hyphens=False)` — convert a `Page` dict into a flat string (used by `plain_text_output`).
- `replace_special_chars(s)` — Unicode normalization on selected ranges.

## Tables

[tables.py](../../repos-folder/pdftext/pdftext/tables.py) — given per-page table bounding boxes (e.g. from a layout model), assigns each char to the table cell whose bbox it's inside, then joins back into cell text.

```python
table_cell_text(
    tables: List[List[int]],      # list of cell bboxes
    page: Page,                   # pdftext Page (with keep_chars=True)
    img_size: List[int],          # image-space dims for re-scaling
    table_thresh=0.8,             # min overlap to count
    space_thresh=0.01,            # min horizontal gap to insert a space
) -> List[TableCell]              # cell text + bbox per cell
```

`is_same_span(bbox, curr_box, img_size, space_thresh, rotation)` decides whether two chars belong to the same word/run inside a cell.

`get_dynamic_gap_thresh(page, img_size, default_thresh=.01, min_chars=100)` adapts the gap threshold to the page's character density.

## Worker model

When `workers > 1`:

- `worker_init(pdf_path, flatten_pdf)` opens the PDF in each worker process.
- `atexit.register(worker_shutdown, pdf_doc)` ensures the handle closes.
- The page list is split into `ceil(len(page_range) / workers)` chunks and dispatched via `ProcessPoolExecutor.map(_get_page_range, ...)`.
- Pages are reassembled in original order after the map.

The `WORKER_PAGE_THRESHOLD` setting (default 10) prevents over-parallelization: process startup costs outweigh extraction gains below the threshold.

## Page rotation

PDF pages can have a `/Rotate` of 0, 90, 180, or 270. The extraction returns bboxes in *unrotated* page coordinates and a `rotation` field. `dictionary_output` swaps `width` / `height` and the page bbox when rotation is 90 / 270 so consumers see the visual orientation.
