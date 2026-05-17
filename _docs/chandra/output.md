# Output (`chandra/output.py`)

Turns the model's raw HTML string into the four artifacts a `BatchOutputItem`
carries.

## `parse_layout(html, image, bbox_scale=1000)` → `list[LayoutBlock]`

- Iterates top-level `<div>`s of the model output.
- Drops `data-label="Blank-Page"` blocks.
- Reads `data-bbox` as four ints in 0..`bbox_scale`; scales to actual pixel
  coords using `image.size`, clamped to bounds.
- Returns `LayoutBlock(bbox, label, content)`. The block's `content` is its
  inner HTML, with nested `data-bbox` attrs stripped (those are kept internal
  to the closed-source pipeline).

`parse_chunks(...)` = `parse_layout(...)` re-shaped via `asdict`.

## `parse_html(html, include_headers_footers=False, include_images=True)`

Cleans the model HTML for downstream rendering:

- Drops `Blank-Page`. Optionally drops `Page-Header` / `Page-Footer`,
  `Image` / `Figure` based on flags.
- For Image/Figure: rewrites `<img src=…>` to the deterministic
  `<md5>_<idx>_img.webp`. If the model didn't emit an `<img>`, one is
  inserted (so the markdown still references the cropped file).
- Strips `<img>` tags **without** `src` from non-image blocks — these are
  model hallucinations.
- Wraps `Text` blocks in `<p>` if there are no inner tags.

## `parse_markdown(...)`

Runs `parse_html` first, then a custom `Markdownify` subclass
(`markdownify==1.1.0`):

- `heading_style="ATX"` (`#`-style headers).
- Math handled by `convert_math`: `<math>...</math>` → `$...$`, `<math
  display>...</math>` → `\n$$...$$\n`.
- Tables: `convert_table` passes them through as **raw HTML**, not GFM pipes.
  This preserves `colspan`/`rowspan` exactly.
- `escape_dollars=True` — replaces `$` in body text with `\$` so it doesn't
  collide with math delimiters.
- Bullets always `-`.
- `process_text` preserves whitespace inside `<pre>`, `<code>`, `<kbd>`,
  `<samp>`, `<math>`.

## `extract_images(html, chunks, page_image)`

For each chunk labelled `Image` or `Figure`:

- Find the `<img>` in the chunk's content (skip if missing).
- `image.crop(bbox)` from the page image; key by `<md5>_<idx>_img.webp`.
- Returns `dict[str, PIL.Image]`. CLI writes each to disk.

Image filenames are deterministic (hash of the HTML + the running div index),
so re-running on identical input yields identical filenames — safe for caches
and Git-tracked outputs.

## Helpers

- `get_image_name(html, div_idx)` — `f"{md5(html)}_{div_idx}_img.webp"`.
- `_hash_html` is `@lru_cache`'d so the same HTML hashes once.
- `draw_layout(image, blocks)` in `chandra/util.py` — debug overlay of bboxes
  + labels on the original page image.
