# Installation

From `repos-folder/chandra/pyproject.toml`:

```
pip install chandra-ocr              # base (vLLM client only)
pip install chandra-ocr[hf]          # + torch, torchvision, transformers, accelerate
pip install chandra-ocr[app]         # + streamlit
pip install chandra-ocr[all]         # everything
```

Base dependencies: `beautifulsoup4`, `click`, `filetype`, `markdownify==1.1.0`, `openai>=2.2.0`, `pillow`, `pydantic`, `pydantic-settings`, `pypdfium2`, `python-dotenv`, `six`.

HF extra adds: `torch>=2.8.0`, `torchvision>=0.23.0`, `transformers>=5.2.0`, `accelerate>=1.11.0`. Python `>=3.10`.

For the HF backend, [flash-attention](https://github.com/Dao-AILab/flash-attention) is recommended for performance.

## From source

```
git clone https://github.com/datalab-to/chandra.git
cd chandra
uv sync
source .venv/bin/activate
```

## Entry points (from `pyproject.toml`)

| Command | Module |
|---|---|
| `chandra` | `chandra.scripts.cli:main` — file/folder CLI |
| `chandra_app` | `chandra.scripts.run_app:main` — Streamlit playground |
| `chandra_screenshot` | `chandra.scripts.screenshot_app:main` — screenshot helper |
| `chandra_vllm` | `chandra.scripts.vllm:main` — launches the vLLM Docker container |
