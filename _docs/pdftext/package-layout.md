# Package layout

Contents of `repos-folder/pdftext/`:

```
pdftext/
├── pyproject.toml             Poetry, Python ^3.10
├── poetry.lock
├── pytest.ini
├── README.md / LICENSE
├── extract_text.py            Tiny shim: from pdftext.scripts.extract_text import extract_text_cli
│
├── pdftext/                   The library
│   ├── __init__.py
│   ├── extraction.py          Public API: plain_text_output, paginated_plain_text_output,
│   │                            dictionary_output, table_output. Manages pypdfium2 PDF lifecycle and
│   │                            multi-process workers.
│   ├── postprocessing.py      Text cleanup, hyphen joining, ligature replacement, block reading-order sort
│   ├── schema.py              Bbox class + Char / Span / Line / Block / Page / Reference / Link /
│   │                            TableCell / TableInput TypedDicts
│   ├── settings.py            pydantic-settings: WORKER_PAGE_THRESHOLD, RESULTS_FOLDER, BENCH_DATASET_NAME
│   ├── tables.py              table_cell_text — match chars to table cell bboxes, with adaptive gap
│   │                            thresholding (get_dynamic_gap_thresh, is_same_span)
│   ├── pdf/                   pypdfium2 wrappers
│   │   ├── pages.py           get_pages, get_blocks, get_lines, get_spans, assign_scripts, is_math_symbol
│   │   ├── chars.py           get_chars (PdfTextPage char cursor), deduplicate_chars
│   │   ├── links.py           add_links_and_refs — attach link URLs to spans; build Page.refs
│   │   └── utils.py           Geometry helpers, page-rotation transforms, flatten()
│   └── scripts/
│       └── extract_text.py    click CLI entry: extract_text_cli
│
├── benchmark/                 Pytest-style benchmarks against pymupdf / pdfplumber
└── tests/                     Test suite
```

## Module identity

```
name        = "pdftext"
version     = "0.6.3"
license     = "Apache-2.0"
python      = "^3.10"
build-system = poetry-core
```

## Console scripts

```toml
[tool.poetry.scripts]
pdftext = "pdftext.scripts.extract_text:extract_text_cli"
```

Run as `pdftext PDF_PATH ...` after install, or `python -m pdftext.scripts.extract_text` directly, or `python extract_text.py` from the repo root.

## Runtime dependencies

| Module | Pin | Why |
|---|---|---|
| `pypdfium2` | `=4.30.0` (exact) | PDF parsing — char-level cursors, page rendering, form flattening |
| `pydantic` | `^2.7.1` | Validation primitives |
| `pydantic-settings` | `^2.2.1` | `Settings` class in `settings.py` |
| `click` | `^8.1.8` | CLI |

`pypdfium2` is pinned to exactly `4.30.0` because the API of `PdfTextPage` and `init_forms` is sensitive to version: 4.30.0 is the version this code was developed against, and char-position semantics have drifted across versions.

## Dev / benchmark deps

```
pymupdf      ^1.24.2      Comparison baseline (text quality + speed)
pdfplumber   ^0.11.0      Another comparison baseline
datasets     ^2.19.0      For loading vikp/pdf_bench HuggingFace dataset
pillow       ^10.3.0      Image handling in benchmarks
rapidfuzz    ^3.8.1       Fuzzy string-comparison for accuracy scoring
tabulate     ^0.9.0       Pretty benchmark output
pytest       ^8.3.4
```

## Settings ([settings.py](../../repos-folder/pdftext/pdftext/settings.py))

```python
class Settings(BaseSettings):
    WORKER_PAGE_THRESHOLD: int = 10        # min pages per worker before spawning processes
    RESULTS_FOLDER: str        = "results"  # benchmark output dir
    BENCH_DATASET_NAME: str    = "vikp/pdf_bench"
```

Like all pydantic-settings classes, every field is overridable via environment variable of the same name:

```bash
export WORKER_PAGE_THRESHOLD=20
pdftext huge.pdf --workers 8
```

## Why "PyMuPDF without AGPL"?

PyMuPDF / MuPDF is AGPL-licensed; using it in a closed-source / SaaS context requires a commercial license. pdftext targets the same use case (structured text extraction with geometry) on top of pypdfium2 (which uses PDFium, Apache-2.0) so commercial integration is friction-free.

The README notes the trade-off: pdftext is "fast and accurate" relative to other Apache-licensed extractors, but PyMuPDF still leads on some edge cases (annotation rendering, sophisticated text-flow recovery). The benchmark dataset `vikp/pdf_bench` is what the README's accuracy table is computed against.

## Limitations

- No OCR — image-only PDFs produce empty output. Pair with a separate OCR engine if needed.
- No layout model — reading order is recovered heuristically; complex magazines / multi-column papers may need an upstream layout detector to supply block bounding boxes.
- Tables are extracted from caller-supplied bboxes, not detected automatically.
- Single-document API — no built-in batching across documents (use shell-level parallelism for that).
