# OCR

PyMuPDF has integrated OCR support through MuPDF/Tesseract. There is no Python `pytesseract` dependency for the PyMuPDF API, but Tesseract language data is still required.

## Required setup

PyMuPDF needs a `tessdata` folder. Typical locations from the docs:

- Windows: `C:/Program Files/Tesseract-OCR/tessdata`
- Unix: `/usr/share/tesseract-ocr/4.00/tessdata`

Ways to provide it:

- Pass `tessdata=...` to OCR calls.
- Set `os.environ["TESSDATA_PREFIX"]` in Python.
- Set the `TESSDATA_PREFIX` environment variable before running Python.

If no path is provided, `pymupdf.get_tessdata()` tries to discover Tesseract by shelling out to commands like `tesseract --list-langs`, `where tesseract` on Windows, or `whereis` on Unix-like systems. Prefer explicit configuration for reliable apps.

## Page OCR

Primary API:

```python
import pymupdf

doc = pymupdf.open("scan.pdf")
page = doc[0]

tp = page.get_textpage_ocr(
    language="eng",
    dpi=300,
    full=True,
    tessdata=r"C:/Program Files/Tesseract-OCR/tessdata",
)

text = page.get_text("text", textpage=tp)
```

Arguments:

- `language`: Tesseract language code, default `eng`.
- `dpi`: render resolution for OCR, default `72`; OCR usually benefits from higher DPI.
- `full`: if `True`, OCR the full rendered page. If `False`, keep readable digital text and OCR only non-readable/remainder areas.
- `tessdata`: explicit language data folder.
- `flags`: text extraction flags for the produced `TextPage`.

## Partial vs full OCR

`full=True` renders the whole page to a pixmap, creates a one-page OCR PDF from that image, and extracts a `TextPage` from it.

`full=False` is the default. It builds a normal text page first, redacts readable digital text from a temporary copy, OCRs what remains, and extends the original text page with OCR text. This is useful for mixed digital/scanned PDFs.

## Image OCR

For a standalone image, load it as a `Pixmap`, remove alpha if present, convert to an OCR PDF, and then extract text:

```python
import pymupdf

pix = pymupdf.Pixmap("image.png")
if pix.alpha:
    pix = pymupdf.Pixmap(pix, 0)

pdf_bytes = pix.pdfocr_tobytes(
    language="eng",
    tessdata=r"C:/Program Files/Tesseract-OCR/tessdata",
)

doc = pymupdf.open("pdf", pdf_bytes)
text = doc[0].get_text()
```

## Errors to expect

- Missing Tesseract/tessdata can raise errors like `No tessdata specified and Tesseract is not installed`.
- Bad or missing language files can raise Tesseract language initialization errors.
- Pyodide builds may have no OCR support.
