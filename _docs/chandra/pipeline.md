# Pipeline (per page)

Driven by `InferenceManager.generate` in `chandra/model/__init__.py`. A batch
of `BatchInputItem`s comes in; a list of `BatchOutputItem`s comes out.

## 1. Load and rasterize

`load_file(filepath, config)` in `chandra/input.py`:

- Sniffs the file with `filetype.guess`. If PDF → `load_pdf_images`; otherwise
  → `load_image`.
- **PDFs** are opened with `pypdfium2`, forms initialized, each page flattened
  (`FPDFPage_Flatten`), then rendered at `IMAGE_DPI` (192) — but never smaller
  than `MIN_PDF_IMAGE_DIM` (1024 px shortest side).
- **Images** are loaded with PIL and upscaled with LANCZOS if the shortest
  side is under `MIN_IMAGE_DIM` (1536 px).
- Returns `list[PIL.Image]` in page order.

## 2. Build a batch

```python
batch = [BatchInputItem(image=img, prompt_type="ocr_layout") for img in images]
```

`prompt_type` resolves to one of `OCR_LAYOUT_PROMPT` / `OCR_PROMPT` via
`PROMPT_MAPPING` (see [prompts.md](prompts.md)). Pass `prompt=` directly to
override.

## 3. Pre-process the image

Both backends call `scale_to_fit(img, max=(3072, 2048), min=(1792, 28),
grid=28)`:

- Snaps dimensions to multiples of 28 pixels (Qwen VL's vision-transformer
  patch grid).
- Caps total pixels at 3072×2048; upsamples tiny crops to at least 1792×28.
- Aspect-ratio-preserving block-shrink loop trims one axis at a time, picking
  whichever loses the least AR.

## 4. Inference

| Backend | Path |
|---------|------|
| `hf`    | `processor.apply_chat_template` → `model.generate(max_new_tokens=12384, eos_token_id=[<endoftext>, <im_end>])` → batch-decode → `GenerationResult`. |
| `vllm`  | Base64-PNG embedded in an OpenAI `chat.completions.create` call. Errors caught and surfaced as `GenerationResult(error=True)`; the wrapper `process_item` retries on errors or repeat-token suffixes. |

Result is a `GenerationResult(raw=<html string>, token_count, error)`.

## 5. Post-process

For each `(GenerationResult, BatchInputItem)` pair:

1. `parse_chunks(raw, image)` → `parse_layout` walks top-level `<div>`s,
   reads `data-bbox` + `data-label`, normalizes 0-1000 bboxes to actual pixel
   coords, returns `LayoutBlock[]` → dict.
2. `parse_markdown(raw, **kwargs)` → first re-renders the HTML through
   `parse_html` (drops Blank-Page, optionally drops Page-Header/-Footer and
   Image/Figure; wires up `<img src>` to the deterministic
   `<md5>_<idx>_img.webp` name; wraps loose Text in `<p>`; strips
   src-less `<img>` hallucinations), then runs a custom `Markdownify`
   subclass with `<math>` / `<chem>` preservation and `$...$` / `$$...$$`
   delimiters.
3. `parse_html(raw, **kwargs)` — same pre-render but skipping the markdown
   convert.
4. `extract_images(raw, chunks, page_image)` — for each `Image` / `Figure`
   chunk, `image.crop(bbox)` and key it by the deterministic filename.

Result: `BatchOutputItem(markdown, html, chunks, raw, page_box, token_count,
images, error)`.

## 6. Save (CLI only)

`save_merged_output` in `chandra/scripts/cli.py`:

- Concatenates all pages' markdown (with optional `N--------…` separators if
  `--paginate_output`), writes `<name>.md` / `<name>.html`.
- Writes each per-page extracted image as its own file in the per-document
  subfolder.
- Writes `<name>_metadata.json` (page counts, tokens, chunk counts).
