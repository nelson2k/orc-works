# Rendering And Images

## Render a page

```python
import pymupdf

doc = pymupdf.open("document.pdf")
page = doc[0]

pix = page.get_pixmap(dpi=150)
pix.save("page-1.png")
```

For OCR, use a higher DPI when quality matters:

```python
pix = page.get_pixmap(dpi=300)
```

## Pixmap notes

OCR expects image data that Tesseract can handle cleanly:

- RGB color space is safest.
- No alpha channel. Remove alpha with `pymupdf.Pixmap(pix, 0)`.
- Vector drawings are not image text. Tesseract will not read line art as text unless the page is rendered to pixels first.

## Extract embedded images

```python
import pymupdf
from pathlib import Path

doc = pymupdf.open("document.pdf")
out = Path("images")
out.mkdir(exist_ok=True)

for page_index, page in enumerate(doc):
    for image_index, image in enumerate(page.get_images()):
        xref = image[0]
        pix = pymupdf.Pixmap(doc, xref)
        if pix.n > 4:
            pix = pymupdf.Pixmap(pymupdf.csRGB, pix)
        pix.save(out / f"page{page_index}-image{image_index}.png")
```

`page.get_images()` only reports embedded raster image objects. Charts and drawings may be vector graphics instead.

## OCR PDF from image

```python
pix = pymupdf.Pixmap("scan.png")
if pix.alpha:
    pix = pymupdf.Pixmap(pix, 0)

pix.pdfocr_save("scan-searchable.pdf", language="eng", tessdata="...")
```

The generated PDF looks like the original image and includes a hidden recognized text layer.
