# Tables (`pdftext/tables.py`)

pdftext does not detect tables. `table_output` *takes* table bboxes from an
external detector (typical caller: a layout model like surya, or marker's
`TableProcessor`) and extracts the embedded text inside each table's cell
positions.

## `table_output(pdf_path, table_inputs, *, pages=None, …) → list[Tables]`

```python
table_inputs = [
    # one dict per page in page_range
    {"tables": [[100, 100, 500, 400]],  # 1+ table bboxes
     "img_size": [612, 792]},            # image-space dims of those bboxes
    {"tables": [], "img_size": [612, 792]},
]
result = table_output("doc.pdf", table_inputs)
# result[0] = [ [TableCell, TableCell, ...], ... ]  -- per table on page 0
```

Behaviour:

- If `pages=None`, calls `dictionary_output(keep_chars=True)` internally.
  Pass an existing `pages` list when you already extracted them.
- `len(pages) == len(table_inputs)` is asserted — one entry per page,
  including empty `{"tables": []}` for pages without tables.
- Per page, returns a `list[Tables]` (one inner list per table input).

## `table_cell_text(tables, page, img_size, table_thresh=0.8, space_thresh=0.01) → Tables`

The work happens here. Per table bbox:

1. `get_dynamic_gap_thresh` — looks at intra-span char-to-char distances
   across the whole page (≥ 100 chars required), takes the 80th percentile
   to learn what a "space" looks like for this document's font. Combined
   with the caller's `space_thresh=0.01` minimum.
2. For every line on the page:
   - Rescale the line bbox into image-space coords (`Bbox.rescale`).
   - Skip if `line.intersection_pct(table_poly) < 0.8` (i.e. the line is
     not mostly inside the table).
3. Walk every char in surviving lines and group into cells:
   - `is_same_span` checks whether the next char belongs to the same cell:
     - Char is just to the right of the previous one (`x0 - prev.x_end < space_thresh * width`).
     - Y-alignment within `space_thresh * height`.
     - Total run length not absurd (`mult=5` cap on x distance).
   - When the test fails, the run breaks and becomes a `{"text", "bbox"}`
     entry.
4. Rotation-aware: separate branches for `0`, `90`, `180`, `270` page
   rotation handle which axis is "horizontal flow".
5. Final cell bboxes are reported **relative to the table bbox** (subtracted
   `table[0]`, `table[1]`), then sorted with `sort_blocks` (vertical-bucket
   then x-order).

## Returned shape

```python
[ # per page
  [ # per table on that page
    {"text": "Cell 1", "bbox": [x1, y1, x2, y2]},
    {"text": "Cell 2", "bbox": [x1, y1, x2, y2]},
    ...
  ],
  ...
]
```

Cells are **runs of contiguous characters**, not grid cells. Reconstructing
actual rows/columns from these is up to the caller — typically by clustering
y-centers (rows) and x-centers (columns).

## Caveats

- Relies on a working text layer. Scanned/OCR'd-image PDFs return empty
  cells.
- The `img_size` is what the layout detector saw; coords are converted
  back to PDF space before intersection tests. Get this wrong and every
  cell will be empty.
- `keep_chars=True` is required during the upstream `dictionary_output`
  call — `table_cell_text` walks individual `char["bbox"]`s.
