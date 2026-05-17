# Programmatic API (`pdftext.extraction`)

Three top-level functions. All thread- and process-safe (each call opens its
own `PdfDocument`).

## `plain_text_output(pdf_path, *, sort=False, hyphens=False, page_range=None, flatten_pdf=False, workers=None) → str`

One big string. Internally calls `paginated_plain_text_output` and joins with
`"\n"`.

```python
from pdftext.extraction import plain_text_output

text = plain_text_output("doc.pdf", sort=True, page_range=[0, 1, 2])
```

## `paginated_plain_text_output(...) → list[str]`

Same args, but returns one string per page (handy when you want explicit page
boundaries).

```python
from pdftext.extraction import paginated_plain_text_output

pages = paginated_plain_text_output("doc.pdf")
for i, page_text in enumerate(pages):
    print(f"=== page {i} ===\n{page_text}")
```

## `dictionary_output(pdf_path, *, sort=False, page_range=None, keep_chars=False, flatten_pdf=False, quote_loosebox=True, disable_links=False, workers=None) → list[Page]`

Returns the structured `Pages` tree (`list[Page]`). Each `Page` has `blocks`
→ `lines` → `spans`, with bboxes flattened to plain `[x1, y1, x2, y2]` lists
(`Bbox` objects are internal-only by the time you get the result).

```python
from pdftext.extraction import dictionary_output

pages = dictionary_output("doc.pdf", keep_chars=True, disable_links=False)
for page in pages:
    for block in page["blocks"]:
        for line in block["lines"]:
            for span in line["spans"]:
                print(span["text"], span["font"]["size"])
```

Flags:

- `keep_chars=True` → preserves per-char `bbox` / `font` / `char_idx`. Needed
  for [tables.md](tables.md).
- `quote_loosebox=True` → uses pypdfium2's loose-bbox mode for `'` characters
  (small accuracy win; can disable for raw pypdfium2 parity).
- `disable_links=True` → skips the link-merging pass, saves a second
  PDF open. `refs` will be missing from the per-page output.

## `table_output(pdf_path, table_inputs, *, page_range=None, flatten_pdf=False, quote_loosebox=True, workers=None, pages=None) → list[Tables]`

Given externally-detected table bboxes (e.g. from a layout model), extract
the text inside each table cell-by-cell. See [tables.md](tables.md).

```python
from pdftext.extraction import table_output

table_inputs = [
    {"tables": [[100, 100, 500, 400]], "img_size": [612, 792]},  # page 0
    {"tables": [], "img_size": [612, 792]},                       # page 1
]
tables = table_output("doc.pdf", table_inputs, pages=existing_pages)
```

Pass `pages=` if you already extracted them via `dictionary_output` — saves
re-running the whole pipeline.

## Worker model

- `workers > 1` spawns a `ProcessPoolExecutor`; each worker keeps its own
  `PdfDocument` open via the `worker_init` initializer.
- The actual worker count is `min(workers, len(page_range) //
  WORKER_PAGE_THRESHOLD)`. Default `WORKER_PAGE_THRESHOLD=10` — so a 20-page
  PDF with `workers=4` still runs with 2 workers.
- `atexit.register` ensures each worker closes its `PdfDocument`.

## Custom flow

If you need something the public API doesn't expose, start from
`pdftext.extraction._get_pages` — it's just `_load_pdf` + `get_pages` +
optional `ProcessPoolExecutor`. The pipeline functions in
`pdftext/pdf/pages.py` are all module-level and reusable.
