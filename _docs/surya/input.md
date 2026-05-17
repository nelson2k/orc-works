# Input loading

## CLI loader

[surya/scripts/config.py](../../repos-folder/surya/surya/scripts/config.py): `CLILoader` is shared by every CLI command. Given an input path (file or folder) it:

1. Parses `--page_range` (`"0,5-10,20"` → sorted, deduplicated list of zero-indexed pages).
2. Calls `load_from_folder` or `load_from_file` from [surya/input/load.py](../../repos-folder/surya/surya/input/load.py).
3. If `highres=True`, repeats the load at `IMAGE_DPI_HIGHRES=192` to produce a parallel highres-image list (used by OCR).
4. Sets `result_path = OUTPUT_DIR/<input-stem>/` and creates the directory.
5. Exposes `self.images`, `self.highres_images`, `self.names`.

`load_from_file` sniffs the type with `filetype.guess(path)`:

- `.pdf` → `load_pdf`: opens via `pypdfium2.PdfDocument`, optionally flattens form fields, renders each page in `page_range` at the requested DPI with `convert("RGB")`.
- otherwise → `load_image`: a single PIL `Image.open(path).convert("RGB")`.

`load_from_folder` iterates every file in the folder (skipping subdirectories and dotfiles), routing each through `load_from_file`. Errors load images that fail to open with a warning (`PIL.UnidentifiedImageError`) and continue.

## Programmatic equivalents

```python
from surya.input.load import load_from_file, load_pdf, load_image

# PDF
images, names = load_pdf("doc.pdf", page_range=[0, 1, 2], dpi=192)

# image
images, names = load_image("page.png")

# auto-detect
images, names = load_from_file("either.pdf", page_range=None, dpi=96)
```

`names` is a list of the same length as `images` — for a PDF it's the basename of the file repeated per page; for a single image it's a one-element list.

## Image preprocessing

[surya/input/processing.py](../../repos-folder/surya/surya/input/processing.py) holds the helpers each predictor uses internally:

- `open_pdf(path)` — `pypdfium2.PdfDocument` factory with `FLATTEN_PDF` honored.
- `get_page_images(doc, page_range, dpi)` — renders the requested pages to PIL.
- `convert_if_not_rgb(image_or_images)` — ensures everything is in RGB mode.
- `slice_polys_from_image(image, polygons)` — crops each polygon to its own sub-image. Used by `RecognitionPredictor.detect_and_slice_bboxes` to feed line crops to the OCR model.
- `slice_bboxes_from_image(image, bboxes)` — axis-aligned variant of the above.

## DPI rules

- 96 DPI low-res images go into detection, layout, reading order, OCR-error detection.
- 192 DPI high-res images go into OCR / recognition and table recognition.

Both are derived from the same `pypdfium2.PdfDocument`. If you only render once and pass it as `images` to recognition (no `highres_images`), the model will still work — it just won't have the extra resolution to chew on for character detail.

## Image quality tips (from README)

- If text is small, raise DPI so the short side is at least ~1024 px. If too large (>2048 px width), downscale.
- For old / blurry / skewed scans, preprocess: binarize, deskew, denoise.
- If line detection joins boxes that should be separate, raise `DETECTOR_TEXT_THRESHOLD`. If it splits things that should be joined, lower it. `DETECTOR_BLANK_THRESHOLD` controls inter-line spacing.
