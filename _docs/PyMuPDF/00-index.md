# PyMuPDF Notes

Local notes from inspecting `repos-folder/PyMuPDF`.

## Files

- `01-overview.md` - what PyMuPDF is useful for, opening from various sources, import style.
- `02-repo-map.md` - full repo layout: `src/`, `src_classic/`, `docs/`, `tests/`, `scripts/`.
- `03-ocr.md` - OCR setup, APIs, full vs partial OCR, image OCR, expected errors.
- `04-text-extraction.md` - text extraction patterns, `get_text` formats, search, concurrent multi-doc extraction.
- `05-rendering-images.md` - rendering pages, pixmap serialization, image extraction, OCR-from-image.
- `06-pdf-editing.md` - PDF creation, merge/split, delete/reorder, save options, annotations (full list), redaction, TOC, metadata, non-PDF→PDF conversion.
- `07-build-tests.md` - build backend (`pipcl`), generated `_build.py`, wheel split, test setup, developer scripts.
- `08-gotchas.md` - practical caveats for OCR/document work.

## Best starting points upstream

- `repos-folder/PyMuPDF/README.md`
- `repos-folder/PyMuPDF/docs/index.rst`
- `repos-folder/PyMuPDF/docs/recipes-ocr.rst`
- `repos-folder/PyMuPDF/docs/recipes-text.rst`
- `repos-folder/PyMuPDF/docs/recipes-multiprocessing.rst`
- `repos-folder/PyMuPDF/docs/installation.rst`
- `repos-folder/PyMuPDF/docs/ocr/tesseract-language-packs.rst`
- `repos-folder/PyMuPDF/docs/pymupdf4llm/ocr-plugins.rst` (for the LLM/RAG sibling package)
- `repos-folder/PyMuPDF/src/utils.py` (`get_textpage_ocr`, `get_text` wrapper)
- `repos-folder/PyMuPDF/src/__init__.py` (every public class)
