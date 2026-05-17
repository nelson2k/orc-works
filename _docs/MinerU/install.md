# Install

Python 3.10–3.13 (`requires-python = ">=3.10,<3.14"`). MinerU pulls in a
*lot* — the base wheel alone has ~30 runtime deps including `boto3`,
`fastapi`, `magika`, `pandas`, `lxml`, `mammoth`, `qwen-vl-utils`,
`mineru-vl-utils`, `pdftext`, `pypdfium2`, `pypdf`, `openai`, `loguru`,
plus office libraries (`python-docx`, `pypptx-with-oxml`, `openpyxl`).

## Pip extras

```bash
pip install mineru                # base (no torch, no models, mostly CLI plumbing)
pip install mineru[pipeline]      # +torch, transformers, onnxruntime, albumentations, omegaconf
pip install mineru[vlm]           # +torch, transformers, accelerate  (lightweight VLM HF runner)
pip install mineru[vllm]          # +vllm (Linux only)
pip install mineru[lmdeploy]      # +lmdeploy (Windows recommended)
pip install mineru[mlx]           # +mlx-vlm (macOS only)
pip install mineru[gradio]        # +gradio + gradio-pdf
pip install mineru[core]          # = vlm + pipeline + gradio
pip install mineru[all]           # = core + platform-appropriate VLM engine
pip install mineru[test]          # = core + pytest, fuzzywuzzy
```

`[all]` chooses the right engine per OS automatically:

```toml
all = [
    "mineru[core]",
    "mineru[mlx]    ; sys_platform == 'darwin'",   # macOS → mlx
    "mineru[vllm]   ; sys_platform == 'linux'",    # Linux → vLLM
    "mineru[lmdeploy] ; sys_platform == 'win32'",  # Windows → LMDeploy
]
```

## Models (separate from pip install)

After the pip install, fetch model weights with:

```bash
mineru-models-download
```

This pulls from ModelScope or HuggingFace into the cache dir. You can also
let MinerU download lazily on first parse — same end state. Size is
roughly:
- Pipeline models: ~2 GB total
- VLM (`MinerU2.5-Pro-2604-1.2B`): ~2.5 GB

## Docker

```bash
# Global (HuggingFace, English-first)
docker pull opendatalab/mineru:latest

# China mirror (ModelScope)
docker pull opendatalab/mineru:china-latest
```

Two folders in the repo (`docker/global/`, `docker/china/`) hold the
Dockerfiles. Base image is upgraded in 3.0.0 to `vllm0.11.2 +
torch2.9.0`, unifying paths across CUDA compute capabilities.

## Entry points

From `pyproject.toml`:

| Command                    | Module                                       |
|----------------------------|----------------------------------------------|
| `mineru`                   | `mineru.cli.client:main`                     |
| `mineru-api`               | `mineru.cli.fast_api:main`                   |
| `mineru-router`            | `mineru.cli.router:main`                     |
| `mineru-gradio`            | `mineru.cli.gradio_app:main`                 |
| `mineru-vllm-server`       | `mineru.cli.vlm_server:vllm_server`          |
| `mineru-lmdeploy-server`   | `mineru.cli.vlm_server:lmdeploy_server`      |
| `mineru-openai-server`     | `mineru.cli.vlm_server:openai_server`        |
| `mineru-models-download`   | `mineru.cli.models_download:download_models` |

See [cli.md](cli.md) and [server.md](server.md) for each.
