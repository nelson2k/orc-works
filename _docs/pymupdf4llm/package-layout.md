# Package Layout

Contents of `repos-folder/pymupdf4llm/`:

```text
src/                         source copied into the pymupdf4llm package
  __init__.py                public API, version check, layout dispatch
  helpers/
    document_layout.py       layout-mode parse/serialize pipeline
    pymupdf_rag.py           legacy markdown/RAG extraction implementation
    utils.py                 page analysis, reading order, tables, geometry
    get_text_lines.py        text-line extraction helpers
    multi_column.py          legacy multi-column detection
    image_quality.py         numpy image-quality / OCR-worthiness helpers
    progress.py              fallback progress bar
  ocr/                       OCR callback plugins
  llama/
    pdf_markdown_reader.py   LlamaIndex reader integration
tests/                       pytest tests and PDF fixtures
examples/                    RAG and GUI demo material
pdf4llm/                     compatibility package re-exporting pymupdf4llm
setup.py                     pipcl package definition
pyproject.toml               pipcl build backend
CHANGES.md                   release notes
README.md                    upstream documentation
LICENSE                      AGPL text
```

`setup.py` builds a pure Python package by copying everything under `src/` into `pymupdf4llm/` and generating `versions_file.py`.

Normal installs require exact matching versions of:

- `pymupdf==1.27.2.3`
- `pymupdf_layout==1.27.2.3`
- `tabulate`
- `markdown`
- `pymdown-extensions`

If `PYMUPDF_SETUP_VERSION` is set, the strict PyMuPDF/Layout dependencies are relaxed for testing.

