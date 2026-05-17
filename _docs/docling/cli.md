# CLI

Two entry points, both built on `typer`:

- `docling` — main CLI, in [docling/cli/main.py](../../repos-folder/docling/docling/cli/main.py). Convert files / URLs / folders into one or more output formats.
- `docling-tools` — auxiliary CLI in [docling/cli/tools.py](../../repos-folder/docling/docling/cli/tools.py). Fetch/cache models, list plugins, etc.

Both require the `cli` extra (`typer`, `rich`). If missing, the CLI prints a helpful install hint and exits 1.

## Common shape

```
docling [OPTIONS] SOURCE...
```

`SOURCE` may be a local file, a directory (recursively expanded), or a `http(s)://` URL.

## Output formats

`--to {md,json,yaml,html,html_split_page,text,doctags,vtt}` — repeatable. Default is `md`. Output is written into `--output / -o` (default cwd) as `<stem>.<ext>`.

- `html_split_page` produces a side-by-side page-image / parsed-doc view. Combine with `--show-layout` to overlay layout boxes via `LayoutVisualizer`.
- `doctags` is the lossless tag format Docling models emit (see `docling-core`'s `DocTagsDocument`).
- `vtt` is for ASR output.

## Pipeline selection

`--pipeline {standard,vlm,asr}`. Default chosen per `InputFormat`:

- `pdf` / `image` → `standard` (StandardPdfPipeline)
- `audio` → `asr` (AsrPipeline)
- `docx`, `pptx`, `xlsx`, `md`, `html`, `xml_*`, `csv`, `vtt`, `latex` → `SimplePipeline` (declarative backend does the work)

`--vlm-model NAME` switches to `VlmPipeline` with the named preset (e.g. `granite_docling`, `smoldocling`, `nuextract_2b`).

## Format-specific knobs

PDF / image:

| Flag | Default | Notes |
|---|---|---|
| `--ocr / --no-ocr` | on | Enable OCR for bitmap regions |
| `--force-ocr` | off | OCR the whole page, ignoring native text |
| `--ocr-engine {auto,easyocr,tesseract,tesseract_cli,rapidocr,ocrmac,kserve_v2}` | `auto` | Picks engine based on availability |
| `--ocr-lang LANG,LANG,...` | engine default | e.g. `en,de` for easyocr; `eng+deu` for tesseract |
| `--table-mode {fast,accurate}` | `accurate` | TableFormer mode |
| `--enrich-code` / `--enrich-formula` | off | Run code/formula VLM enrichment |
| `--enrich-picture-classes` / `--enrich-picture-description` | off | Run picture enrichment models |
| `--abort-on-error` | off | Stop the batch on first failure |
| `--page-range A-B` | full doc | 1-indexed inclusive range |
| `--device {auto,cpu,cuda,mps,xpu}` | auto | Sets `AcceleratorOptions.device` |
| `--num-threads N` | 4 | CPU threads for inference |

Audio:

| Flag | Default | Notes |
|---|---|---|
| `--asr-model PRESET` | `whisper_base` | One of `whisper_{tiny,base,small,medium,large,turbo}[_mlx,_native]` |

## Discoverability

- `docling --help` — top-level flags.
- `docling --show-external-plugins` — lists every OCR, layout, and table-structure plugin registered through the entry-point system, with their package source. Useful when integrating third-party engines.
- `docling --logo` — prints the ASCII-art Docling logo.
- `docling --version` — prints versions of `docling`, `docling-core`, `docling-ibm-models`, `docling-parse`, Python, platform.

## Export details

`export_documents` in `cli/main.py` handles writing each output kind. For Markdown it cross-checks for an empty file and downgrades status to FAILURE with a `DOC_ASSEMBLER` error if the export produced nothing.

`--show-layout` together with `--to html_split_page` uses `docling_core.transforms.visualizer.layout_visualizer.LayoutVisualizer` to draw block boxes over the page image.

## `docling-tools`

Smaller utility CLI with subcommands for model artifact management (download, cache inspection). See [docling/cli/tools.py](../../repos-folder/docling/docling/cli/tools.py).
