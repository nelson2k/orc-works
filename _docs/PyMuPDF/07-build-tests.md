# Build And Tests

## Build metadata

`pyproject.toml` is minimal:

```toml
[build-system]
    requires = ['pipcl']
    build-backend = "setup"
    backend-path = ["."]
```

Most build behavior lives in `setup.py` (~57k bytes) and the vendored `src/pipcl.py` PEP-517 backend. Windows toolchain discovery is in `src/wdev.py`.

At install time, `setup.py` generates `src/_build.py` with the resolved version info (`pymupdf_version`, `pymupdf_version_tuple`, `mupdf_location`, git sha, swig version, etc.). It is then re-exported as `pymupdf.pymupdf_version` from `src/__init__.py:420-441` and used to set `VersionBind` (PyMuPDF) and `VersionFitz` (MuPDF).

The release split:

- `PyMuPDFb` wheel - shared MuPDF C/C++ libs. Not specific to a Python version. Reduces total release size.
- `PyMuPDF` wheel - Python-version-specific bindings that depend on `PyMuPDFb`.
- `PyMuPDFd` wheel - build-time files used to build PyMuPDF (optional, for redistributors).

## Running tests

The repo's `tests/README.md` says to:

1. Create and enter a virtual environment.
2. Install PyMuPDF.
3. Install test dependencies listed in `scripts/gh_release.py:test_packages`.
4. Run pytest on the PyMuPDF directory.

The canonical test_packages list (from `scripts/gh_release.py:611`):

```
pytest fontTools pymupdf-fonts flake8 pylint codespell [+ pillow] [+ psutil]
```

`tests/conftest.py:install_required_packages()` additionally installs these on first run (plus `mypy` and `pipcl`):

```powershell
python -m pip install pytest fontTools pymupdf-fonts flake8 pylint codespell mypy pipcl pillow psutil
pytest repos-folder/PyMuPDF
```

## OCR tests

`tests/test_tesseract.py` checks both cases:

- With `TESSDATA_PREFIX` set, `page.get_textpage_ocr(full=True)` should succeed.
- Without tessdata, calls are expected to raise the known Tesseract initialization/missing data errors. Expected message: `code=3: Tesseract language initialisation failed`. On Pyodide, the expected error is `code=6: No OCR support in this build`.

`test_3842` and `test_3842b` exercise OCR on a specific bug-fixture PDF; they skip cleanly when Tesseract is missing.

This means local OCR test success depends on local Tesseract/tessdata configuration, not only Python dependencies.

## Test guardrails

`tests/conftest.py:wrap()` wraps every test:

- Asserts `pymupdf.TOOLS.mupdf_warnings()` is empty after the test.
- Asserts the test did not leave `pymupdf.TOOLS.set_small_glyph_heights` toggled.

There is a separate `test_threads.py` which exercises multi-threaded `open`/`get_text` and is one of the few places where PyMuPDF is intentionally driven from multiple Python threads (mostly to demonstrate the failure mode the docs warn about).

## Developer scripts

- `scripts/test.py` - main developer build+test driver. Supports a pre-existing local MuPDF checkout, an internal downloaded copy, or a `git:URL` source. Wraps pytest invocations.
- `scripts/gh_release.py` - CI entry, drives `cibuildwheel` for the wheel matrix. Also defines `test_packages`.
- `scripts/sysinstall.py` - Linux-only system install path; builds and installs MuPDF + PyMuPDF into a prefix and runs tests via `LD_PRELOAD`/`PYTHONPATH`.
