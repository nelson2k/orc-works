# Overview

PDFText reads a PDF's **embedded text** (it does not OCR), groups characters
into spans → lines → blocks via heuristics, and emits either flat strings or
a `Page` dict tree. It is the text-extraction layer marker uses for
digital PDFs (see [vs_others.md](vs_others.md)).

## Modules (`pdftext/`)

| File                  | Job                                                                |
|-----------------------|--------------------------------------------------------------------|
| `extraction.py`       | Public API: `plain_text_output`, `dictionary_output`, `table_output`. Owns the worker pool for multi-process page extraction. |
| `pdf/chars.py`        | `get_chars(textpage, …)` — pull char + font + bbox from pypdfium2. `deduplicate_chars` strips duplicates from overlapping text objects. |
| `pdf/pages.py`        | `get_pages(...)` — runs `get_spans` → `get_lines` → `assign_scripts` → `get_blocks` per page. |
| `pdf/links.py`        | `add_links_and_refs(pages, pdf)` — read link annotations, split spans on link bboxes, build a `PageReference` for internal goto links. |
| `pdf/utils.py`        | `flatten`, `get_fontname`, `matrix_intersection_area`, whitespace constants. |
| `postprocessing.py`   | Ligature replacement, control-char strip, hyphenation handling, `sort_blocks` reading-order pass, `merge_text` for plain-text output. |
| `tables.py`           | `table_cell_text` — given table bboxes from an external detector (e.g. surya), extract span-bounded cell text. |
| `schema.py`           | `Bbox` class + `TypedDict`s for `Char`, `Span`, `Line`, `Block`, `Page`, `Link`, `Reference`. |
| `settings.py`         | One pydantic setting: `WORKER_PAGE_THRESHOLD`. |
| `scripts/extract_text.py` | `pdftext` Click CLI. |

## Key properties

- **No OCR.** Scanned PDFs return empty/garbage — use chandra or surya for that.
- **No layout model.** Block grouping is a simple median-gap heuristic in
  `get_blocks`, not a neural model.
- **Apache 2.0.** Replacement for PyMuPDF specifically motivated by avoiding
  AGPL.
- **Thin wrapper.** ~1000 LOC total. If a feature isn't here, drop to
  pypdfium2 directly.
- **Multi-process safe.** `ProcessPoolExecutor` ships the PDF path to each
  worker; each worker opens its own `PdfDocument`.

## Bench numbers (from README, single-process on M1)

| Library    | s / page | Alignment vs pymupdf |
|------------|---------:|---------------------:|
| pymupdf    | 0.32     | (ground truth)       |
| pdftext    | 1.36     | 97.78%               |
| pdfplumber | 3.16     | 90.36%               |

pdftext is ~2× slower than calling pypdfium2 alone — the cost of the
deduplication + grouping passes.
