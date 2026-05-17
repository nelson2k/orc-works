# Output / HTML post-processing

[chandra/output.py](../../repos-folder/chandra/chandra/output.py). The model returns a single HTML string per page, structured as a sequence of top-level `<div>` elements each with `data-label` and `data-bbox` attributes. Post-processing turns this into the four exposed outputs.

## Raw model output shape

```html
<div data-bbox="50 80 950 120" data-label="Section-Header">Chapter 1</div>
<div data-bbox="50 140 950 600" data-label="Text">Lorem ipsum...</div>
<div data-bbox="60 620 940 900" data-label="Table"><table>…</table></div>
<div data-bbox="0 0 1000 1000" data-label="Image"><img alt="diagram of the bowtie network"/></div>
```

Bbox coordinates are normalized 0–1000 in both axes. `settings.BBOX_SCALE=1000`.

## `parse_html(html, include_headers_footers, include_images)`

Walks top-level divs. Behavior per `data-label`:

- `Blank-Page` → skipped.
- `Page-Header` / `Page-Footer` → skipped unless `include_headers_footers=True`.
- `Image` / `Figure` → skipped unless `include_images=True`. When kept, ensures the inner `<img>` has a `src` set to `get_image_name(html, div_idx)`; if no `<img>` exists, inserts one.
- Other labels → `<img>` tags without `src` are dropped (these are model hallucinations).
- `Text` blocks with bare text (no inner HTML tags) get wrapped in `<p>...</p>`.

Returns the concatenated inner HTML of all kept blocks (top-level `<div>` wrappers stripped).

## `parse_markdown(html, ...)`

Calls `parse_html` first, then runs a custom `Markdownify` (subclass of `markdownify.MarkdownConverter`) with:

- `heading_style="ATX"` (uses `#`), `bullets="-"`.
- Math: `<math display="block">` → `\n$$ … $$\n`; inline → ` $…$ `.
- Tables: pass through as raw HTML (`convert_table` returns `\n\n<table>…</table>\n\n`).
- Dollar signs in text are escaped to `\$`.
- Links escape `[`, `]`, `(`, `)` in link text.
- Whitespace handling mostly follows upstream, except `process_text` normalizes whitespace and escapes special chars unless inside `pre`/`code`/`kbd`/`samp`/`math`.

If conversion raises, returns `""` and prints the error.

## `parse_layout(html, image, bbox_scale=1000)`

Returns a list of `LayoutBlock(bbox, label, content)`. Steps per top-level div:

- Skips `Blank-Page`.
- Parses `data-bbox` into 4 ints; on failure falls back to `[0,0,1,1]` and prints a warning.
- Scales bbox to image pixel coordinates: `x_new = int(x * width / 1000)`, clamped to image bounds.
- `data-label` falls back to `"block"` if missing.
- Strips nested `data-bbox` attributes inside `content` (those are stripped in open source builds).

`parse_chunks(html, image, bbox_scale)` calls `parse_layout` and `asdict`s each block — produces the `chunks` field on `BatchOutputItem`.

## `extract_images(html, chunks, image)`

For every chunk with label `Image` or `Figure`, crops the source image to the chunk's bbox and stores it under `get_image_name(html, div_idx)` (md5 of full page HTML + div index, suffix `_img.webp`). Returns `{filename: PIL.Image}`. Invalid bboxes are skipped silently.

## Layout debug helper

[chandra/util.py: draw_layout(image, layout_blocks)](../../repos-folder/chandra/chandra/util.py). Draws each block's bbox as a red rectangle with the label in blue. Returns a copy of the image.
