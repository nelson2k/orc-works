# Pipeline (per page)

The heart of pdftext lives in `pdftext/pdf/pages.py`. For each page in
`page_range`, `get_pages` runs the same five-step chain.

## 0. Open the page

```python
page = pdf.get_page(page_idx)
if flatten_pdf:
    flatten(page)                    # FPDFPage_Flatten — merges form fields
    page = pdf.get_page(page_idx)    # must re-fetch after flatten

textpage = page.get_textpage()
page_bbox = page.get_bbox()
page_rotation = page.get_rotation()  # 0 / 90 / 180 / 270
```

## 1. `get_chars` (`pdf/chars.py`)

Walks `textpage.count_chars()` and pulls each character with pypdfium2's
low-level `FPDFText_*` calls:

- `FPDFText_GetUnicode` — the character.
- `FPDFText_GetCharAngle` — rotation in radians.
- `textpage.get_charbox(i, loose=…)` — bbox. `loose=True` (default unless
  `quote_loosebox=False` and the char is `'`) gives the looser PDF-standard
  bbox; `loose=False` is tighter but can miss ascenders/descenders.
- `FPDFText_GetFontSize`, `FPDFText_GetFontWeight`, `get_fontname` — font
  metadata.

Bboxes are translated to **top-left origin** (PDF uses bottom-left) and then
rotated through the page rotation.

Returns `list[Char]`. See [schema.md](schema.md).

## 2. `deduplicate_chars`

PDFs commonly draw the same text twice (e.g. fill + stroke for the same
glyphs, or overlapping text objects). This pass:

1. Greedily packs chars into "words" (same font/rotation, no whitespace or
   hyphen yet).
2. Deduplicates words by `(rounded_bbox, text, rotation, font_*)`.
3. Flattens back to `list[Char]`.

Without this, you'd get every glyph twice in 30-50% of real-world PDFs.

## 3. `get_spans`

Greedy left-to-right grouping. Start a new span when:

- Any font field (`name`, `flags`, `size`, `weight`) differs from the current
  span.
- Rotation changes.
- Previous span ended with `\n` or `\x02` (soft-hyphen).
- The new char is small + above + to-the-right (likely a superscript).

Spans carry `bbox` (merged from member chars), `text`, `font`, `chars` list
(kept until post-processing strips it unless `keep_chars=True`).

## 4. `get_lines`

Groups spans into lines. Break on:

- Last span ends with `\n` or `\x02`.
- Rotation differs by ≥ 45°.
- Current span's `y_start > previous_line.y_end` (pdfium occasionally skips
  a linebreak, so we fall back to position).

## 5. `assign_scripts`

Per line with ≥ 2 spans (and wider than it is tall), looks for spans that
are short + at the top (`superscript=True`) or short + at the bottom
(`subscript=True`). Guarded by:

- Span text is a single character or all digits.
- Span text is alphanumeric or in unicode category `Sm` (math symbol).
- The span and its neighbours satisfy a "this one is genuinely raised/lowered
  vs the rest of the line" geometry test.

## 6. `get_blocks`

Clusters lines into blocks (≈ paragraphs):

1. Compute median x-center gap and y-center gap between consecutive lines.
2. Set `allowed_*_gap = median * 1.5`.
3. For each line, merge into the current block if it's within both
   tolerances. Special cases:
   - Indented first line of a paragraph.
   - Last line shorter than the rest.
   - Lines extremely close in y (inline math etc.).
   - Any geometric overlap with the current block bbox.
4. Final pass merges any remaining blocks whose bboxes still overlap.

That's it — there is no neural model, no PDF object model lookup, just gap
statistics. Result is `list[Block]`.

## Output

`get_pages` returns a list of `Page` dicts:

```python
{
    "page": page_idx,
    "bbox": [x1, y1, x2, y2],
    "width": page_width,
    "height": page_height,
    "rotation": page_rotation,
    "blocks": blocks,
}
```

After this, public API functions may layer on:

- `add_links_and_refs` (see [links.md](links.md)) — splits spans on link
  bboxes, adds `refs`.
- `postprocess_text` + `handle_hyphens` (see [postprocessing.md](postprocessing.md)).
- `sort_blocks` reading-order pass if `sort=True`.
