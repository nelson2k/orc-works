# Overview

PyMuPDF is a Python binding around MuPDF for reading, rendering, extracting, converting, and editing documents.

## Main capabilities

- Open PDFs and several document/image formats with `pymupdf.open(...)`.
- Extract text as plain text, words, blocks, dictionaries, HTML, XML, XHTML, or JSON.
- Render pages to images via `page.get_pixmap(...)`.
- Extract images and inspect page objects.
- Search text locations with `page.search_for(...)`.
- Add annotations, redactions, links, drawings, inserted text, and images.
- Merge, split, reorder, and save PDFs.
- Detect and export tables with `page.find_tables()`.
- Run integrated OCR using Tesseract language data.

## Import style

Use:

```python
import pymupdf
```

The old `fitz` import is legacy compatibility. New code should use `pymupdf`.

## Why it matters for OCR work

PyMuPDF is useful both before and after OCR:

- Detect whether a page already has usable text.
- Render scanned pages at a chosen DPI.
- Run OCR into a `TextPage`.
- Search/extract recognized text through the same APIs as native PDF text.
- Save image OCR results as searchable PDF pages.
