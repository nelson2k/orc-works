# Build And Tests

## Build metadata

`pyproject.toml` is minimal:

```toml
[build-system]
requires = ['pipcl']
build-backend = "setup"
backend-path = ["."]
```

Most build behavior lives in `setup.py` and local `pipcl` helpers.

## Running tests

The repo's `tests/README.md` says to:

1. Create and enter a virtual environment.
2. Install PyMuPDF.
3. Install test dependencies listed in `scripts/gh_release.py:test_packages`.
4. Run pytest on the PyMuPDF directory.

Example dependency set shown in the repo:

```powershell
python -m pip install pytest fontTools psutil pymupdf-fonts pillow
pytest repos-folder/PyMuPDF
```

## OCR tests

`tests/test_tesseract.py` checks both cases:

- With `TESSDATA_PREFIX` set, `page.get_textpage_ocr(full=True)` should succeed.
- Without tessdata, calls are expected to raise the known Tesseract initialization/missing data errors.

This means local OCR test success depends on local Tesseract/tessdata configuration, not only Python dependencies.

## Test guardrails

`tests/conftest.py` wraps tests with additional checks, including MuPDF warning checks and checks that tests do not mutate selected globals.
