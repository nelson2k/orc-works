# Settings (`pdftext/settings.py`)

The smallest settings module of all the Datalab repos — three fields, one
of which actually affects extraction.

```python
class Settings(BaseSettings):
    WORKER_PAGE_THRESHOLD: int = 10   # min pages per worker process
    RESULTS_FOLDER: str = "results"   # benchmark only
    BENCH_DATASET_NAME: str = "vikp/pdf_bench"  # benchmark only

settings = Settings()
```

All env-overridable via `pydantic_settings` — pdftext does not look for a
`local.env` file, only `os.environ`.

## `WORKER_PAGE_THRESHOLD`

Caps worker count in `_get_pages`:

```python
workers = min(workers, len(page_range) // settings.WORKER_PAGE_THRESHOLD)
```

So a 100-page PDF with `--workers 16` runs with 10 workers (`100 // 10`), and
a 30-page PDF with `--workers 16` runs with 3.

Bump it down (e.g. `WORKER_PAGE_THRESHOLD=5`) if you have a many-core
machine and lots of short PDFs. Bump it up if process-spawn overhead is
hurting you on short jobs.

```bash
$env:WORKER_PAGE_THRESHOLD=5
pdftext small.pdf --workers 8 --out_path out.txt
```

## What's *not* configurable here

- Image DPI / rendering (pdftext does no rendering).
- OCR (pdftext does no OCR).
- Grouping thresholds — `superscript_height_threshold` (0.7),
  `line_distance_threshold` (0.1), and the `tolerance_factor=1.5` in
  `get_blocks` are all baked into the source. Fork or pass kwargs directly
  to `get_pages` if you need to tune them.
