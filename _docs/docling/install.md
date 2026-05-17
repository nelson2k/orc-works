# Installation

The package on PyPI is `docling-slim` (version `2.93.0` in the source). The base install is intentionally small (~50 MB, 8 dependencies); features live behind `pip install docling-slim[<extra>]` extras.

## Base dependencies

From `pyproject.toml`:

```
pydantic >=2.0,<3.0
docling-core >=2.73.0,<3.0
pydantic-settings >=2.3,<3.0
filetype >=1.2,<2.0
requests >=2.32.2,<3.0
certifi >=2024.7.4
pluggy >=1.0,<2.0
tqdm >=4.65,<5.0
```

Python `>=3.10,<4.0`. Python 3.9 was dropped in 2.70.0.

## Extras (selected)

| Extra | Adds |
|---|---|
| `convert-core` | `numpy`, `pillow`, `rtree`, `scipy` — needed by any non-trivial pipeline |
| `extract-core` | `convert-core` + `polyfactory` |
| `format-pdf-pypdfium2` | `pypdfium2` — minimal PDF backend |
| `format-pdf-docling` | `pypdfium2` + `docling-parse` — full Docling PDF backend |
| `format-pdf` | both PDF backends |
| `format-docx` / `format-pptx` / `format-xlsx` | python-docx / python-pptx / openpyxl |
| `format-office` | docx + pptx + xlsx |
| `format-html` / `format-markdown` / `format-web` | beautifulsoup4+lxml / marko / both |
| `format-latex` | `pylatexenc` |
| `format-xml-xbrl` | `arelle-release` |
| `format-html-render` | `playwright` |
| `format-audio` | `openai-whisper`, `numba`, `mlx-whisper` (Apple Silicon) |
| `feat-ocr-rapidocr` | RapidOCR engine |
| `feat-ocr-rapidocr-onnx` | RapidOCR + onnxruntime |
| `feat-ocr-easyocr` | EasyOCR |
| `feat-ocr-tesserocr` | tesserocr |
| `feat-ocr-mac` | ocrmac (macOS Vision framework) |
| `models-local` | torch, torchvision, docling-ibm-models, accelerate, huggingface_hub, defusedxml |
| `models-remote` | tritonclient[grpc] |
| `models-onnxruntime` | onnxruntime / onnxruntime-gpu |
| `models-vlm-inline` | transformers, accelerate, mlx-vlm (Apple), qwen-vl-utils, peft |
| `feat-chunking` | docling-core[chunking] |
| `service-client` | httpx, websockets |
| `cli` | typer, rich |
| `standard` | format-pdf + models-local + rapidocr + office + web + latex + chunking + extract-core + service-client + cli |
| `all` | `standard` + every other extra |

Typical real-world installs:

```
pip install "docling-slim[standard]"    # PDF + Office + Web + LaTeX + local models + RapidOCR + CLI
pip install "docling-slim[all]"         # the kitchen sink
```

The historical `docling` meta-package (in `packages/docling/`) is a thin wrapper that pulls in `docling-slim[standard]`.

## Entry points (from `pyproject.toml`)

```
docling       = docling.cli.main:app           # main CLI (typer)
docling-tools = docling.cli.tools:app          # auxiliary CLI (model fetch etc.)
```

Both require the `cli` extra (`typer`, `rich`). If missing, the CLI prints a friendly install hint and exits.

## Hardware notes

- `models-local` requires PyTorch ≥2.2.2. Apple Silicon, NVIDIA CUDA, Intel XPU, and CPU all supported via `AcceleratorOptions.device`.
- `format-audio` on Apple Silicon uses `mlx-whisper` for speed; elsewhere it uses `openai-whisper` (CPU/CUDA).
- Flash Attention 2 (`cuda_use_flash_attention2=True`) is supported on Ampere+ NVIDIA GPUs and gives a significant speedup; requires `flash-attn` installed separately.

## From source

```
git clone https://github.com/docling-project/docling.git
cd docling
uv sync
```

The repo is a uv workspace; `packages/docling` is a workspace member referencing the slim package.
