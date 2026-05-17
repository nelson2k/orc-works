# CLI

Defined in [chandra/scripts/cli.py](../../repos-folder/chandra/chandra/scripts/cli.py).

```
chandra INPUT_PATH OUTPUT_PATH [options]
```

`INPUT_PATH` may be a single file or a directory. Supported extensions: `.pdf`, `.png`, `.jpg`, `.jpeg`, `.gif`, `.webp`, `.tiff`, `.bmp`.

## Options

| Flag | Default | Notes |
|---|---|---|
| `--method [hf\|vllm]` | `vllm` | Inference backend |
| `--page-range TEXT` | none | PDF page range, e.g. `1-5,7,9-12` |
| `--max-output-tokens INT` | 12384 (settings) | Per-page generation cap |
| `--max-workers INT` | min(64, batch size) | vLLM concurrency |
| `--max-retries INT` | 6 (settings) | vLLM repeat-detection retry budget |
| `--include-images / --no-images` | include | Save and embed cropped images |
| `--include-headers-footers / --no-headers-footers` | exclude | Keep `Page-Header` / `Page-Footer` blocks |
| `--save-html / --no-html` | save | Write `*.html` |
| `--batch-size INT` | 28 vllm, 1 hf | Pages per batch |
| `--paginate_output` | false | Inserts `{n}` + 48 dashes between pages in markdown |

## Output layout

For input `foo.pdf`, output goes to `OUTPUT_PATH/foo/`:

```
foo/
  foo.md                        merged markdown for all pages
  foo.html                      merged HTML (when --save-html)
  foo_metadata.json             page count, token totals, per-page chunk/image counts
  <hash>_<idx>_img.webp         each extracted image
```

The hashed image names come from `output.get_image_name(html, div_idx)` — md5 of the page HTML plus the div index, so identical pages produce stable names.

## vLLM server launcher

`chandra_vllm` ([chandra/scripts/vllm.py](../../repos-folder/chandra/chandra/scripts/vllm.py)) shells out to `docker run` against `vllm/vllm-openai:v0.17.0`, loading `settings.MODEL_CHECKPOINT` (default `datalab-to/chandra-ocr-2`) and exposing port 8000.

```
chandra_vllm [--gpu h100|a100|a100-40|l40s|a10|l4|4090|3090|t4] [--mtp]
```

Per-GPU presets pick `max-num-batched-tokens` and `max-num-seqs` by scaling against an H100/80GB baseline (8192 tokens, 64 sequences). `--mtp` enables MTP speculative decoding (off by default — flagged as unstable). Other knobs are hardcoded: `--max-model-len 18000`, `--gpu-memory-utilization .85`, `--enable-prefix-caching`, mm-processor pixel range `3136..6291456`.

`VLLM_GPUS` env var selects device IDs; `VLLM_MODEL_NAME` controls the `--served-model-name` flag.
