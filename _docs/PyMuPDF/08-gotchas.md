# Gotchas

## OCR is slow

The docs describe OCR as roughly one thousand times slower than normal text extraction. OCR once per page, keep the `TextPage`, and reuse it for extraction/search.

## Explicit tessdata is safer

Automatic tessdata discovery exists, but production code should pass `tessdata=...` or set `TESSDATA_PREFIX`.

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

PyMuPDF does not support multithreaded use. Use multiprocessing, with each process opening its own document handle.

## Coordinates

Page geometry APIs use PDF points. Rendered pixmap dimensions depend on DPI or transformation matrix, so do not mix page coordinates and pixel coordinates without scaling.
