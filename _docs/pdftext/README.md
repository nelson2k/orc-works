# pdftext

[github.com/VikParuchuri/pdftext](https://github.com/VikParuchuri/pdftext) — a Python library that extracts structured text from PDFs using [pypdfium2](https://github.com/pypdfium2-team/pypdfium2). Apache-2.0 licensed (the "PyMuPDF without AGPL" angle in the README).

The package extracts:

- Plain text per page (with optional reading-order sort and hyphen handling)
- Structured pages → blocks → lines → spans → chars, each with bounding boxes and font info
- Table cell text given a list of table bounding boxes per page

It uses Pdfium's character-level cursor (`PdfTextPage.get_charbox` / `.get_char_box4`) to extract every character with its position, then groups characters into spans (by font and proximity), spans into lines, and lines into blocks.

## Module identity

```
[tool.poetry]
name        = "pdftext"
version     = "0.6.3"
license     = "Apache-2.0"
python      = "^3.10"

dependencies:
  pypdfium2      = 4.30.0
  pydantic       = ^2.7.1
  pydantic-settings = ^2.2.1
  click          = ^8.1.8

dev/bench:
  pymupdf, pdfplumber, pillow, datasets, rapidfuzz, tabulate, pytest
```

CLI script: `pdftext = "pdftext.scripts.extract_text:extract_text_cli"`.

## Top-level layout

```
pdftext/
├── pyproject.toml
├── poetry.lock
├── pytest.ini
├── README.md / LICENSE
├── extract_text.py            Trivial shim that calls scripts.extract_text:extract_text_cli (for `python -m`)
│
├── pdftext/                   The package
│   ├── extraction.py          Public API: plain_text_output, dictionary_output, table_output, paginated_plain_text_output
│   ├── postprocessing.py      Text cleanup: hyphens, ligatures, control chars, block sorting
│   ├── schema.py              Bbox class + TypedDicts: Char, Span, Line, Block, Page, TableCell, TableInput, Link, Reference
│   ├── settings.py            Pydantic-settings: WORKER_PAGE_THRESHOLD, RESULTS_FOLDER, BENCH_DATASET_NAME
│   ├── tables.py              table_cell_text — match characters to table cell bboxes
│   ├── pdf/
│   │   ├── pages.py           get_pages, get_blocks, get_lines, get_spans, assign_scripts, is_math_symbol
│   │   ├── chars.py           get_chars (pdfium character cursor), deduplicate_chars
│   │   ├── links.py           add_links_and_refs — link annotations → in-text URLs
│   │   └── utils.py           Geometry / page-rotation helpers
│   └── scripts/
│       └── extract_text.py    Click CLI
│
├── benchmark/                 Quality + speed benchmarks (against pymupdf / pdfplumber)
└── tests/                     Pytest suite
```

## CLI

```bash
pdftext PDF_PATH --out_path output.txt
pdftext PDF_PATH --json --out_path output.json
pdftext PDF_PATH --sort --keep_hyphens
pdftext PDF_PATH --page_range 0,5-10,12 --workers 4 --flatten_pdf --keep_chars
```

Options:

| Flag | Default | Notes |
|---|---|---|
| `PDF_PATH` (positional) | required | Single PDF file |
| `--out_path` | stdout | Output file path |
| `--json` | off | Emit structured JSON instead of plain text |
| `--sort` | off | Sort blocks in reading order using `postprocessing.sort_blocks` |
| `--keep_hyphens` | off | Keep end-of-line hyphens; otherwise join words |
| `--page_range` | all | Comma-separated `1,2-4,10` — 0-indexed |
| `--flatten_pdf` | off | Run `pdf.init_forms()` so form fields are merged into the content |
| `--keep_chars` | off | (JSON only) include per-character data inside spans |
| `--workers` | none | Parallelize across processes; effective only if ≥ `WORKER_PAGE_THRESHOLD` (default 10) pages per worker |

## Python API

```python
from pdftext.extraction import (
    plain_text_output,
    paginated_plain_text_output,
    dictionary_output,
    table_output,
)

# Plain text, joined by newline
text = plain_text_output("in.pdf", sort=True, hyphens=False)

# List of pages, each a string
pages = paginated_plain_text_output("in.pdf", sort=True)

# Structured output (matches the JSON CLI shape)
pages = dictionary_output("in.pdf", sort=True, keep_chars=False, page_range=[0,1,2])

# Extract text from rectangles per page (e.g. table cells)
tables = table_output(
    "in.pdf",
    table_inputs=[
        {"tables": [[100, 100, 500, 300], [100, 320, 500, 500]], "img_size": [800, 1000]},
        ...
    ],
    pages=pages,   # optional: pass pre-extracted pages to skip re-parsing
)
```

## Output schema (JSON / `dictionary_output`)

```jsonc
[
  {
    "page": 0,
    "bbox": [x1, y1, x2, y2],
    "width": 612, "height": 792,
    "rotation": 0,
    "refs": [...],          // links / references resolved into the body
    "blocks": [
      {
        "bbox": [...],
        "lines": [
          {
            "bbox": [...],
            "spans": [
              {
                "bbox": [...],
                "text": "Hello, world",
                "font": { "name": "...", "size": 12, "weight": 400, "flags": ... },
                "rotation": 0,
                "url": "https://...",       // when add_links_and_refs found a match
                "superscript": false,
                "subscript": false,
                "char_start_idx": 0,
                "char_end_idx": 12,
                "chars": [                  // only if keep_chars=True
                  {"bbox": [...], "char": "H", "rotation": 0.0,
                   "font": {...}, "char_idx": 0},
                  ...
                ]
              }
            ]
          }
        ]
      }
    ]
  },
  ...
]
```

