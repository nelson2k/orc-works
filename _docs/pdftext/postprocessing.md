# Post-processing (`pdftext/postprocessing.py`)

After `get_pages` builds the block tree, post-processing cleans text and —
optionally — reorders blocks. Applied in different combos by the public API:

- `plain_text_output` → `merge_text` (full post-processing + hyphen joining)
- `dictionary_output` → only per-span `postprocess_text` + `handle_hyphens(keep_hyphens=True)` (always preserves the soft-hyphen marker)

## `postprocess_text(text)`

Pipeline:

1. `\r\n` → `\n`.
2. `replace_special_chars`: maps any character in
   `pdf/utils.SPACES` (`" ￾ ﻿ \xa0"`) to a regular space;
   `LINE_BREAKS` (`"\n  
"`) to `\n`; `TABS` (`"\t 	"`) to `\t`.
3. `replace_control_chars`: drops anything whose unicode category starts
   with `C` (control) — except `\x02` (the soft-hyphen marker pdfium uses)
   and `WHITESPACE_CHARS`.
4. `replace_ligatures`:

   ```
   ﬀ → ff   ﬃ → ffi   ﬄ → ffl
   ﬁ → fi   ﬂ → fl   ﬆ → st   ﬅ → st
   ```

## `handle_hyphens(text, keep_hyphens=False)`

pdfium emits `\x02` for soft hyphens (end-of-line word breaks). Two modes:

- `keep_hyphens=True` → `\x02` becomes `-\n` (you see "well-\nformed").
- `keep_hyphens=False` (default for plain-text CLI) → the hyphen is dropped,
  the next-line whitespace is collapsed, words are joined across the break
  (`"well-\nformed"` → `"wellformed"` on the same line, then trailing newline).

## `sort_blocks(blocks, tolerance=1.25)`

Coarse reading-order pass. Used when `sort=True`.

1. Bucket blocks by `round(block.bbox[1] / 1.25) * 1.25` — groups same-y
   blocks (i.e. left & right columns at the same vertical position).
2. Sort each bucket left-to-right (`bbox[0]`).
3. Flatten.

Doesn't model columns explicitly — relies on the y-grouping to keep
neighbouring columns together. Works well for 1–2 column documents, less so
for complex magazine layouts.

## `merge_text(page, sort=False, hyphens=False)`

Used by `plain_text_output` to render a page to a string:

```
for block in page.blocks (sorted if sort=True):
    for line in block.lines:
        for span in line.spans:
            line_text += span.text
        line_text = postprocess_text(line_text).rstrip() + "\n"
    block_text = block_text.rstrip() + "\n\n"
text = handle_hyphens(text, keep_hyphens=hyphens)
```

So you always get **`\n` between lines** and **`\n\n` between blocks** in
plain-text mode.

## What's not done here

- No de-duplication beyond the char-level pass in `deduplicate_chars`.
- No language detection.
- No column detection beyond the y-bucket heuristic.
- No table reconstruction (use [tables.md](tables.md) with bboxes from an
  external detector).
- No image extraction (use pypdfium2 directly or marker for that).
