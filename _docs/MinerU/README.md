# MinerU

High-accuracy document parsing engine that converts PDF / DOCX / PPTX / XLSX / image / web inputs into structured Markdown and JSON. Open-sourced by OpenDataLab.

Three backends share a common pipeline:

- **pipeline** — CPU- or GPU-friendly layout + OCR + formula/table parsers (PyTorch-Paddle, UnimerNet, FormulaNet, etc.). Lowest dependency surface, no hallucinations.
- **vlm** — runs a vision-language model (`MinerU2.5-Pro-2604-1.2B`) over rasterized pages. Higher accuracy on hard layouts.
- **hybrid** — combines native text extraction with the VLM for accuracy without hallucinations.

The Python package is `mineru` and ships eight console scripts.

## Top-level layout

| Path | Purpose |
|---|---|
| `mineru/cli/` | Click CLIs + FastAPI server + Gradio UI + router |
| `mineru/backend/pipeline/` | Pipeline analyze pass (layout → OCR → formula → table) |
| `mineru/backend/vlm/` | VLM analyze pass (engine-agnostic, sglang / vLLM / LMDeploy / MLX / HTTP client) |
| `mineru/backend/hybrid/` | Hybrid analyze pass (text + VLM verification) |
| `mineru/backend/office/` | Native DOCX / PPTX / XLSX parsing (no VLM, no rasterization) |
| `mineru/model/` | Model implementations: VLM servers, OCR, layout (PP-DocLayout v2), MFR (UnimerNet, FormulaNet), table (cls + rec) |
| `mineru/data/` | `DataReader` / `DataWriter` abstractions: file / S3 / multi-bucket S3 / HTTP / dummy |
| `mineru/utils/` | PDF tools, classification, drawing, language guessing, config reader, OS env, model utils |
| `mineru/resources/` | Shipped assets (fonts, configs) |
| `projects/` | Auxiliary projects (currently mostly Chinese README placeholders) |
| `tests/` | Pytest suite |
| `docker/` | Reference Dockerfiles |
| `docs/` | MkDocs site sources |

## Outputs

Each parsed document produces (by default under `<output>/<stem>/<backend>/<method>/`):

- `<stem>.md` — Markdown render with reading order, tables as HTML, formulas as LaTeX, image refs to `images/`
- `<stem>_content_list.json` — flat list of blocks `[{type, text|html|latex|img_path, page_idx, bbox}, ...]`
- `<stem>_middle.json` — intermediate page-structured representation, what the renderer consumes
- `<stem>_model.json` — raw model outputs (per-page layout boxes, OCR spans, etc.)
- `images/` — extracted figures, charts, table crops
- `<stem>_layout.pdf` / `<stem>_span.pdf` — debug visualizations
- Optional original input copy

## Key concepts

- **Middle JSON** — internal page-structured intermediate. Pipeline produces it via `model_json_to_middle_json`; VLM via `model_output_to_middle_json`; office via `office_middle_json_mkcontent`. The "make content" stage walks middle JSON and emits Markdown / JSON / content-list.
- **Methods** (`-m` / `--method`) — `auto` (decide per page), `txt` (text extraction), `ocr` (force OCR). Only meaningful for `pipeline` and `hybrid-*` backends.
- **Processing window** — for the pipeline backend, multiple short documents are bin-packed up to `MINERU_PROCESSING_WINDOW_SIZE` pages (default 64) and run as one batch.
- **API-mediated CLI** — since 3.0.0 the `mineru` CLI orchestrates an HTTP client against `mineru-api`. If `--api-url` is omitted it spawns a temporary local server.

## Entry points

| Surface | Entry point |
|---|---|
| CLI | `mineru.cli.client:main` (`mineru`) |
| Sync FastAPI server | `mineru.cli.fast_api:main` (`mineru-api`) |
| Multi-server router | `mineru.cli.router:main` (`mineru-router`) |
| Gradio UI | `mineru.cli.gradio_app:main` (`mineru-gradio`) |
| Model downloader | `mineru.cli.models_download:download_models` (`mineru-models-download`) |
| VLM servers | `mineru.cli.vlm_server` → `vllm_server` / `lmdeploy_server` / `openai_server` |

