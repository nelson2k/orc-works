# CLI (`chandra`)

Defined in `chandra/scripts/cli.py`. Always takes two positional args:

```bash
chandra <input_path> <output_path> [options]
```

`input_path` can be a single file (PDF or image) or a directory — in directory
mode it processes every supported file (`.pdf .png .jpg .jpeg .gif .webp .tiff
.bmp`).

## Options

| Flag                                    | Default       | Notes                                                                        |
|-----------------------------------------|---------------|------------------------------------------------------------------------------|
| `--method [hf\|vllm]`                   | `vllm`        | See [backends.md](backends.md).                                              |
| `--page-range TEXT`                     | all pages     | e.g. `"1-5,7,9-12"`. PDFs only.                                              |
| `--max-output-tokens INT`               | 12384         | Per-page cap.                                                                |
| `--max-workers INT`                     | `min(64,N)`   | Parallel vLLM requests. vLLM only.                                           |
| `--max-retries INT`                     | 6             | vLLM retry budget on repeats / errors.                                       |
| `--include-images/--no-images`          | include       | Save cropped figure JPEGs alongside the markdown.                            |
| `--include-headers-footers/--no-headers-footers` | exclude  | Whether `Page-Header` / `Page-Footer` blocks survive into output.            |
| `--save-html/--no-html`                 | save          | Emit `.html` next to `.md`.                                                  |
| `--batch-size INT`                      | 28 vllm / 1 hf | Pages per batch sent to the model.                                          |
| `--paginate_output`                     | off           | Insert `\n\nN--------…\n\n` separators between pages in the merged output.   |

## Output structure

For input `doc.pdf` and output dir `./out`, you get:

```
out/
└─ doc/
   ├─ doc.md                  # merged markdown across all pages
   ├─ doc.html                # merged HTML
   ├─ doc_metadata.json       # per-page token counts, bboxes, chunk count
   └─ <md5>_<idx>_img.webp    # extracted figure crops
```

Image filenames are deterministic: MD5 of the per-page raw HTML + the block
index, suffix `_img.webp`. So re-running on the same input produces the same
filenames.

Per-page metadata entry shape (see `save_merged_output` in `chandra/scripts/cli.py`):

```json
{
  "page_num": 0,
  "page_box": [0, 0, W, H],
  "token_count": 1234,
  "num_chunks": 17,
  "num_images": 3
}
```
