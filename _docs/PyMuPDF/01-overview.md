# Overview

PyMuPDF is a Python binding around MuPDF for reading, rendering, extracting, converting, and editing documents.

## Main capabilities

- Open PDFs and several document/image formats with `pymupdf.open(...)`. Supported: PDF, XPS, EPUB, MOBI, FB2, CBZ, SVG, TXT, plus common raster images (JPG, PNG, etc.).
- Extract text as plain text, words, blocks, dictionaries, HTML, XHTML, XML, or JSON.
- Render pages to images via `page.get_pixmap(...)`.
- Extract embedded raster images and inspect page objects.
- Search text locations with `page.search_for(...)`.
- Add annotations, redactions, links, drawings, inserted text, fonts, and images.
- Merge, split, reorder, save (`doc.save(...)`) or serialize to memory (`doc.tobytes()` / `doc.write()`) PDFs.
- Detect and export tables with `page.find_tables()`.
- Run integrated OCR using Tesseract language data.
- Convert non-PDF documents (EPUB, XPS, etc.) to PDF in memory via `doc.convert_to_pdf()`.

## Import style

Use:

```python
import pymupdf
```

The old `fitz` import is legacy compatibility (still works as of v1.24.0+ but flagged as legacy in the upstream README). New code should use `pymupdf`.

`pymupdf.open` is an alias for the `Document` constructor.

## Opening from various sources

```python
import pymupdf

doc = pymupdf.open("scan.pdf")                   # path
doc = pymupdf.open("epub", epub_bytes)           # type + buffer
doc = pymupdf.open(stream=pdf_bytes, filetype="pdf")  # kwargs form
doc = pymupdf.open(filename, filetype="pdf")     # unknown extension

# Reflowable formats (EPUB etc.) accept layout hints:
doc = pymupdf.open("book.epub", rect=pymupdf.Rect(0, 0, 595, 842), fontsize=11)
```

## Why it matters for OCR work

PyMuPDF is useful both before and after OCR:

- Detect whether a page already has usable text.
- Render scanned pages at a chosen DPI.
- Run OCR into a `TextPage`.
- Search/extract recognized text through the same APIs as native PDF text.
- Save image OCR results as searchable PDF pages.
- For higher-level RAG/LLM pipelines, the sibling `pymupdf4llm` package adds Markdown/JSON extraction with optional OCR plugins for Tesseract, RapidOCR, and PaddleOCR.
