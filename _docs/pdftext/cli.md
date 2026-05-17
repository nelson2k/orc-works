# CLI (`pdftext`)

Defined in `pdftext/scripts/extract_text.py`. Always takes one positional
arg: the PDF path.

```bash
pdftext <PDF_PATH> [options]
```

If `--out_path` is omitted, output goes to stdout.

## Options

| Flag                    | Default | Notes                                                                  |
|-------------------------|---------|------------------------------------------------------------------------|
| `--out_path PATH`       | stdout  | Write to file instead of stdout.                                        |
| `--json`                | off     | Emit structured JSON instead of plain text. See [schema.md](schema.md). |
| `--sort`                | off     | Sort blocks into best-guess reading order (see [postprocessing.md](postprocessing.md)). |
| `--keep_hyphens`        | off     | Preserve `-\n` line-end hyphens; default joins the word across the linebreak. |
| `--page_range "0,5-10"` | all     | Comma-separated single pages / ranges (inclusive).                     |
| `--flatten_pdf`         | off     | `init_forms()` + `FPDFPage_Flatten` before extraction (merges form fields into the page content stream). |
| `--keep_chars`          | off     | JSON only — keep per-character `bbox`/`font`/`char_idx`.                |
| `--workers N`           | 1       | Parallel pages across processes. Capped at `len(pages) // WORKER_PAGE_THRESHOLD` (default `10`) internally — small books stay single-process. |

## Examples

```bash
# Plain text, all pages, default order
pdftext input.pdf --out_path out.txt

# Reading-order-sorted, hyphens removed
pdftext input.pdf --out_path out.txt --sort

# JSON with chars + font info (big files!)
pdftext input.pdf --out_path out.json --json --keep_chars

# First 10 pages only, parallel 4-way
pdftext input.pdf --out_path out.txt --page_range "0-9" --workers 4

# Form-heavy PDF: flatten first so form-field text comes through
pdftext form.pdf --flatten_pdf --json
```

## JSON output shape (high level)

```json
[
  {
    "page": 0,
    "bbox": [0, 0, 612, 792],
    "width": 612,
    "height": 792,
    "rotation": 0,
    "blocks": [
      {
        "bbox": [...],
        "lines": [
          {
            "bbox": [...],
            "spans": [
              {
                "text": "Hello",
                "bbox": [...],
                "char_start_idx": 0,
                "char_end_idx": 4,
                "rotation": 0,
                "font": {"size": 12, "weight": 400, "name": "Times", "flags": 4},
                "url": "",
                "superscript": false,
                "subscript": false,
                "chars": [/* only with --keep_chars */]
              }
            ]
          }
        ]
      }
    ],
    "refs": [{"idx": 0, "page": 1, "coord": [0.0, 0.0]}]
  }
]
```

Full field reference: [schema.md](schema.md).
