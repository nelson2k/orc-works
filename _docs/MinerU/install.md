# Install

`pyproject.toml` defines the build system (`setuptools>=77.0.3`) and a layered extras matrix. Pick an extras set that matches the backend you'll run.

## Python version

`requires-python = ">=3.10,<3.14"`.

## Base install (CLI client only — no local inference)

```bash
pip install mineru
```

That gives you the `mineru` CLI, the FastAPI server skeleton, the data-reader/writer layer, and the `vlm-http-client` backend (which talks to a remote OpenAI-compatible VLM server). It does NOT include `torch`, `transformers`, or any GPU stack.

## Extras

| Extra | What it adds | When you need it |
|---|---|---|
| `vlm` | `torch>=2.6`, `transformers>=4.57.3`, `accelerate` | Local VLM inference (PyTorch path) |
| `vllm` | `vllm>=0.10.1.1` | Run `mineru-vllm-server` |
| `lmdeploy` | `lmdeploy>=0.10.2` | Run `mineru-lmdeploy-server` (Windows-only default in `all`) |
| `mlx` | `mlx-vlm`, `mlx` | macOS Apple Silicon VLM inference |
| `pipeline` | `torch`, `torchvision`, `transformers`, `onnxruntime`, `albumentations`, `shapely`, `pyclipper`, `omegaconf`, `dill`, `PyYAML`, `ftfy` | The pipeline backend (layout + OCR + formula + table) |
| `gradio` | `gradio>=5.49.1`, `gradio-pdf` | Run `mineru-gradio` UI |
| `core` | `vlm` + `pipeline` + `gradio` | Full local stack |
| `all` | `core` + platform-specific server (`mlx` on macOS, `vllm` on Linux, `lmdeploy` on Windows) | Convenience pin |
| `test` | `core` + pytest, pytest-cov, coverage, fuzzywuzzy | Running the test suite |

Typical desktop install:

```bash
pip install "mineru[core]"        # everything local-inference-capable
pip install "mineru[pipeline]"    # just pipeline backend, no VLM
pip install "mineru[vlm,vllm]"    # VLM via vLLM
```

Hybrid backends (`hybrid-auto-engine`, `hybrid-http-client`) require the `pipeline` extras to be present (they import `torch`). The CLI raises `HybridDependencyError` from `mineru/cli/common.py` if you ask for `hybrid-*` without it.

## Configuration file

A user-level `~/mineru.json` (path override: env `MINERU_TOOLS_CONFIG_JSON`) holds optional settings. Template is shipped as [mineru.template.json](../../repos-folder/MinerU/mineru.template.json):

```json
{
  "bucket_info": { "bucket-name-1": ["ak", "sk", "endpoint"] },
  "latex-delimiter-config": { "display": {"left": "$$", "right": "$$"}, "inline": {"left": "$", "right": "$"} },
  "llm-aided-config": { "title_aided": { "api_key": "...", "base_url": "...", "model": "...", "enable": false } },
  "models-dir": { "pipeline": "", "vlm": "" },
  "config_version": "1.3.1"
}
```

If absent, defaults are used (models downloaded to the HuggingFace/ModelScope cache; no S3; no LLM-aided post-processing).

## Models

The pipeline and VLM model files are pulled at first use from HuggingFace or ModelScope via `mineru-models-download`. To pre-fetch:

```bash
mineru-models-download             # interactive prompt
mineru-models-download -s modelscope -m pipeline
mineru-models-download -s huggingface -m vlm
```

The download utility resolves to a writable cache and updates the `models-dir` entry in `~/mineru.json`.

## Docker

`docker/` ships reference Dockerfiles per backend. The official image upgraded in 3.0.0 to `vllm0.11.2 + torch2.9.0`.

## Hardware

| Backend | Min HW | Recommended |
|---|---|---|
| `pipeline` | CPU works | Any CUDA GPU; Apple MPS supported |
| `vlm-auto-engine` (PyTorch) | CUDA GPU, ~6 GB VRAM | 12 GB+ |
| `vlm-auto-engine` (vLLM) | CUDA GPU, ≥10 GB | More VRAM = larger batches |
| `vlm-auto-engine` (MLX) | Apple Silicon | M-series with 16 GB+ unified memory |
| `vlm-http-client` | Any CPU | Remote GPU does the work |
| `hybrid-*` | Same as pipeline + same as vlm (chosen variant) | |

The auto-engine variants detect the available accelerator in this order: CUDA → MPS → NPU → GCU → MUSA → MLU → SDAA → CPU. Override with env `MINERU_DEVICE_MODE`.

## Domestic AI chips

3.x added support for: Ascend, Cambricon, Enflame, MetaX, Moore Threads, Kunlunxin, Iluvatar, Hygon, Biren, T-Head. These are wired through `MINERU_DEVICE_MODE`, `MINERU_LMDEPLOY_DEVICE`, and `MINERU_VLLM_DEVICE` overrides.
