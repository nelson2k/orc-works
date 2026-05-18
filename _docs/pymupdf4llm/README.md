# pymupdf4llm - notes

Source: `repos-folder/pymupdf4llm`. Package version in this checkout: `1.27.2.3`. License: dual licensed GNU AGPL v3 or Artifex commercial license.

PyMuPDF4LLM is a Python package for converting PDF and other PyMuPDF-readable documents into LLM/RAG-friendly outputs:

- Markdown via `pymupdf4llm.to_markdown(...)`
- JSON via `pymupdf4llm.to_json(...)`
- plain text via `pymupdf4llm.to_text(...)`
- LlamaIndex documents via `pymupdf4llm.LlamaMarkdownReader()`

It is built on PyMuPDF/MuPDF and, by default, activates `pymupdf.layout` for layout-aware extraction. The main value is reading-order reconstruction, table handling, image references, page chunks, and selective OCR for scanned or damaged text regions.

Good first files:

- `README.md` - upstream feature overview and examples.
- `setup.py` - version and install requirements.
- `src/__init__.py` - public API and layout/legacy dispatch.
- `src/helpers/document_layout.py` - current layout-mode parser and serializers.
- `src/helpers/pymupdf_rag.py` - legacy markdown extraction path.
- `src/ocr/` - OCR plugin callbacks.

