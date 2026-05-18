# Gotchas

## OCR is slow

The docs describe OCR as roughly one thousand times slower than normal text extraction (`docs/recipes-ocr.rst:45`). OCR once per page, keep the `TextPage`, and reuse it for extraction/search.

## Explicit tessdata is safer

Automatic tessdata discovery exists (`get_tessdata()` tries the `tessdata` arg, then `TESSDATA_PREFIX`, then `tesseract --list-langs`, then `where tesseract` on Windows or `whereis` on Unix), but production code should pass `tessdata=...` or set `TESSDATA_PREFIX`.

## Pyodide has no OCR

Browser/WebAssembly builds raise `code=6: No OCR support in this build` (`tests/test_tesseract.py:23-24`).

## Scanned PDFs have no real text

If `page.get_text()` returns empty output for a scanned page, render/OCR the page. Normal extraction cannot read text that is only pixels.

## Garbled text may need OCR

PDFs with broken/missing Unicode maps may show glyphs visually but extract bad characters. OCR can be a fallback.

## Images need no alpha for OCR

Standalone image OCR can fail when the pixmap has transparency. Remove alpha:

```python
if pix.alpha:
    pix = pymupdf.Pixmap(pix, 0)
```

## Vector graphics are not embedded images

`page.get_images()` does not find vector charts/drawings. Rasterize the page if visual content must be analyzed.

## Threading

PyMuPDF does not support multithreaded use (`docs/recipes-multiprocessing.rst:19`: "doing so may cause incorrect behaviour or even crash Python itself"). Use multiprocessing, with each process opening its own document handle. The module-level `pymupdf.get_text(path, method="mp")` is the supported parallel path; `method="fork"` is also available but not on Windows.

## Coordinates

Page geometry APIs use PDF points (1 point = 1/72 inch). Rendered pixmap dimensions depend on DPI or transformation matrix, so do not mix page coordinates and pixel coordinates without scaling.

## fitz vs pymupdf imports

`import fitz` still works as a legacy alias (v1.24.0+), but new code should use `import pymupdf`. Mixed imports within one process work because `fitz` re-exports `pymupdf` symbols (`src/fitz___init__.py`).

## Page rotation affects extraction

`get_textpage` temporarily resets page rotation to 0 before extracting and restores it after (`src/__init__.py:12182-12196`). This is usually invisible, but if you cache rectangles from extraction and later compare them to drawing/annotation rectangles on a rotated page, you may see mismatches — apply the rotation transform first.
