# Repo Map

Source inspected at `repos-folder/PyMuPDF`.

## Top level

- `README.md` - product overview, quick starts, feature list, FAQ.
- `COPYING` - AGPL license text.
- `changes.txt` - changelog.
- `setup.py` - build logic.
- `pyproject.toml` - build backend points to local `setup`.
- `pytest.ini` - pytest config.
- `valgrind.supp` - memory tooling suppressions.

## Source

- `src/__init__.py` - large generated/main API surface. Contains classes like `Document`, `Page`, `Pixmap`, `Rect`, `Matrix`, `TextPage`, and helpers like `get_tessdata`.
- `src/utils.py` - higher-level helper methods attached to core classes. OCR page logic lives here in `get_textpage_ocr`.
- `src/table.py` - table detection implementation and table result classes.
- `src/fitz___init__.py` and `src/fitz_utils.py` - legacy `fitz` compatibility layer.
- `src/__main__.py` - command-line entry helpers.
- `src_classic/` - older/classic SWIG-style sources and helper interface files.

## Docs

- `docs/index.rst` - documentation table of contents.
- `docs/installation.rst` - installation and OCR setup.
- `docs/recipes-ocr.rst` - focused OCR recipes.
- `docs/functions.rst` - global functions, including `get_tessdata`.
- `docs/document.rst`, `docs/page.rst`, `docs/pixmap.rst`, `docs/textpage.rst` - class API docs.
- `docs/pymupdf4llm/` - Markdown/JSON extraction for RAG workflows.
- `docs/locales/` - translated docs assets.

## Tests

- `tests/test_tesseract.py` - OCR/tessdata behavior.
- `tests/test_textextract.py`, `tests/test_textsearch.py`, `tests/test_tables.py` - extraction/search/table coverage.
- `tests/resources/` - PDF/image fixtures.
- `tests/README.md` - test setup notes.
