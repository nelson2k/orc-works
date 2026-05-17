# Install

Python `>=3.10`. Defined in `pyproject.toml`.

## Pip extras

```bash
pip install chandra-ocr           # base — vLLM client only, no torch
pip install chandra-ocr[hf]       # +torch, torchvision, transformers, accelerate
pip install chandra-ocr[app]      # +streamlit
pip install chandra-ocr[all]      # = hf + app
```

The base install is intentionally light — for the vLLM workflow you don't need
torch at all on the client side. `[hf]` is what pulls in the heavy ML deps.

For HF inference Datalab also recommends [flash attention](https://github.com/Dao-AILab/flash-attention)
for speed; not installed by default.

## From source (uv)

```bash
git clone https://github.com/datalab-to/chandra.git
cd chandra
uv sync
source .venv/bin/activate     # Linux/Mac
.venv\Scripts\Activate.ps1    # Windows
```

`uv.lock` is checked in.

## Entry points (from `pyproject.toml`)

| Command              | Module                                          |
|----------------------|-------------------------------------------------|
| `chandra`            | `chandra.scripts.cli:main`                      |
| `chandra_app`        | `chandra.scripts.run_app:main` → streamlit      |
| `chandra_screenshot` | `chandra.scripts.screenshot_app:main` → flask   |
| `chandra_vllm`       | `chandra.scripts.vllm:main` → docker            |

## Model weights

Pulled from HuggingFace on first run as `datalab-to/chandra-ocr-2`
(configurable via `MODEL_CHECKPOINT`). The model is ~7B parameters
(Qwen 3.5 VL family).
