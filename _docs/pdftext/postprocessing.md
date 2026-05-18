# Post-processing

Module: [pdftext/postprocessing.py](../../repos-folder/pdftext/pdftext/postprocessing.py)

Cleanups applied to extracted text and (for plain-text output) to block ordering.

## Per-span text cleanup

`postprocess_text(s: str) -> str` chains the following:

- `replace_control_chars(s)` — strips/normalizes Unicode category `Cc` (control chars) — TAB/LF/CR are preserved, the rest are removed.
- `replace_special_chars(s)` — normalizes a curated set of replacements (CJK fullwidth, no-break space, soft hyphen, etc.).
- `replace_ligatures(s)` — maps common typographic ligatures back to ASCII pairs:

  | Char | Replacement |
  |---|---|
  | `ﬀ` | `ff` |
  | `ﬁ` | `fi` |
  | `ﬂ` | `fl` |
  | `ﬃ` | `ffi` |
  | `ﬄ` | `ffl` |
  | `ﬅ` | `ft` |
  | `ﬆ` | `st` |

PDF authors who embed only the ligature glyph but not the underlying letters benefit — without this step, the extracted text contains the ligature codepoint which most consumers handle poorly.

## Hyphen handling

`handle_hyphens(text: str, keep_hyphens=False) -> str`

When `keep_hyphens=False` (the default for CLI plain text), the function:

1. Walks the text looking for `-\n` patterns (end-of-line hyphen).
2. Joins the broken pair into a single word (`hyphen-\nated` → `hyphenated`).
3. Collapses the line break.

When `keep_hyphens=True`, the hyphen and newline are preserved as-is.

For JSON output (`dictionary_output`), `handle_hyphens(..., keep_hyphens=True)` is *always* used at the span level — clients can rejoin themselves if they want.

## Block sorting (reading order)

`sort_blocks(blocks: List[Block], tolerance=1.25) -> List[Block]`

PDF content streams have no obligation to draw text in reading order. Multi-column layouts in particular can come out as a jumble of left-then-right or interleaved blocks. `sort_blocks` re-orders by:

1. Clustering blocks into horizontal rows: blocks whose vertical centers differ by less than `tolerance * line_height` are on the same row.
2. Within a row, sorting by `x_start` (left-to-right).
3. Between rows, sorting by `y_start` (top-to-bottom).

This is best-effort: complex layouts (tables, wrapping figures, magazine columns) may not be perfectly recovered. Use a layout-detection model upstream for those cases.

## `merge_text`

`merge_text(page: Page, sort=False, hyphens=False) -> str` is what `plain_text_output` uses:

1. Optionally re-sort blocks via `sort_blocks`.
2. For each block / line / span, concatenate `span["text"]`.
3. Insert line separators between lines, paragraph separators between blocks.
4. Apply `handle_hyphens(text, keep_hyphens=hyphens)` to the final string.

The default separators are tuned for code-style flat text. Override by setting span/line/block separators directly if you fork.

## What's NOT done here

- No language-aware normalization (no NFC vs NFKC choice).
- No quote normalization (curly vs straight).
- No paragraph rejoining other than hyphen handling — soft-wrapped lines in the source PDF stay wrapped.

If you need any of those, post-process the output of `plain_text_output` yourself.
