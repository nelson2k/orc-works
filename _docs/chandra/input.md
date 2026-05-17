# Input loading

[chandra/input.py](../../repos-folder/chandra/chandra/input.py).

`load_file(filepath, config) -> List[PIL.Image]` is the single entry point:

```python
images = load_file("doc.pdf", {"page_range": "1-3,5"})
```

The format is sniffed with `filetype.guess`. If it's a PDF, `load_pdf_images` is used; otherwise `load_image` (single page).

## PDF path

`load_pdf_images(filepath, page_range, image_dpi, min_pdf_image_dim)`:

1. Opens the PDF with `pypdfium2.PdfDocument` and `init_forms()`.
2. For each page in scope:
   - Computes a per-page DPI: `scale_dpi = max(image_dpi, (min_pdf_image_dim / min_page_dim) * 72)`. This guarantees the short side of the page is at least `MIN_PDF_IMAGE_DIM=1024` px.
   - `flatten(page)` calls `FPDFPage_Flatten` with `FLAT_NORMALDISPLAY` so form fields render into the bitmap. Failures print a warning but don't abort.
   - Renders to PIL via `page.render(scale=scale_dpi/72).to_pil().convert("RGB")`.

Settings used: `IMAGE_DPI=192`, `MIN_PDF_IMAGE_DIM=1024`.

## Image path

`load_image(filepath, min_image_dim=1536)`:

- Opens with PIL, converts to RGB.
- If either dimension is below `min_image_dim`, upscales with LANCZOS so the shorter side equals `min_image_dim`.

## Page range parsing

`parse_range_str("0,5-10,20")` returns a deduplicated sorted list `[0, 5, 6, 7, 8, 9, 10, 20]`.

Note: when `page_range` is passed via the CLI it's already a string here, not yet parsed — `load_file` calls `parse_range_str` internally.
