# Tests

Tests are under `tests/` and use pytest.

Main test areas:

- `test_ocr.py` - OCR behavior with replacement characters and SVG-like text.
- `test_markdown_to_pdf.py` - Markdown to PDF round trip and back to Markdown.
- `test_tablulate.py` - table/tabulate regression fixture.
- `test_370.py`, `test_376.py`, `test_137.py` - issue/regression fixtures.
- `tests/pymupdf4llm/llama_index/` - LlamaIndex reader/layout tests.

Fixtures include small PDFs such as:

- `test_370.pdf`
- `test_137.pdf`
- `test_ocr_loremipsum_FFFD.pdf`
- `test_ocr_loremipsum_svg.pdf`
- `test_tablulate_bug.pdf`

`tests/conftest.py` installs extra packages at test startup unless `PYODIDE_ROOT` is set:

```text
llama_index pytest-asyncio rapidocr-onnxruntime
```

That means a normal test run may perform network installs before tests start. The OCR tests also adapt assertions based on whether Tesseract tessdata or `rapidocr_onnxruntime` is available.

