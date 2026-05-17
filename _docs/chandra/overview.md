# Overview

Chandra is a **single vision-language model** (`datalab-to/chandra-ocr-2`,
based on Qwen 3.5 VL) prompted to emit a constrained HTML representation of a
page. Everything after that — markdown, chunks, image extraction — is
deterministic post-processing on that one HTML string.

This is the inverse of marker's design: marker is a long pipeline of small
models + heuristics + optional LLM tweaks; chandra is *one* VLM call per page.

## Core modules (`chandra/`)

| Module                 | Job                                                              |
|------------------------|------------------------------------------------------------------|
| `input.py`             | Read PDF/image → list of `PIL.Image`s at the right DPI.          |
| `prompts.py`           | The big `OCR_LAYOUT_PROMPT` + allowed HTML tags / block labels.  |
| `model/__init__.py`    | `InferenceManager` — picks the backend and orchestrates a batch. |
| `model/hf.py`          | Local inference via `transformers` (`generate_hf`).              |
| `model/vllm.py`        | Remote inference via vLLM server (OpenAI-compatible).            |
| `model/util.py`        | `scale_to_fit` (28-px grid), repeat-token detector for retries.  |
| `output.py`            | Parse model HTML → markdown / layout / chunks / cropped images.  |
| `util.py`              | `draw_layout` — debug overlay of block bboxes.                   |
| `settings.py`          | `pydantic_settings.BaseSettings` reading `local.env`.            |
| `scripts/cli.py`       | `chandra` CLI entry point.                                       |
| `scripts/app.py`       | Streamlit single-page demo.                                      |
| `scripts/vllm.py`      | `chandra_vllm` — Docker-based vLLM server launcher.              |
| `scripts/screenshot_app.py` | `chandra_screenshot` — Flask UI for layout-overlay screenshots. |

## Inputs and outputs

- **Inputs:** any PDF (rendered to PNGs via `pypdfium2`), or any image
  (`.png .jpg .jpeg .gif .webp .tiff .bmp`).
- **Per-page output:** a `BatchOutputItem` carrying `markdown`, `html`,
  `chunks` (layout blocks with bboxes + labels), `raw` (the model's exact
  string), `images` (dict of cropped figures), `token_count`.

See [pipeline.md](pipeline.md) for what happens in a single call.
