# Settings

[chandra/settings.py](../../repos-folder/chandra/chandra/settings.py). Pydantic-settings, reads `local.env` from the current working tree via `find_dotenv`. All fields are env-overridable.

| Field | Default | Notes |
|---|---|---|
| `BASE_DIR` | repo root | derived |
| `IMAGE_DPI` | `192` | Target DPI when rasterizing PDFs |
| `MIN_PDF_IMAGE_DIM` | `1024` | Minimum short-side pixel size for PDF page renders — DPI is bumped if needed |
| `MIN_IMAGE_DIM` | `1536` | Minimum short-side pixel size for image inputs (upscaled with LANCZOS) |
| `MODEL_CHECKPOINT` | `datalab-to/chandra-ocr-2` | HF Hub path used by both HF and vLLM backends |
| `TORCH_DEVICE` | `None` | Pin HF model to a device (`cuda:0`, `mps`, …). `None` → `device_map="auto"` |
| `TORCH_ATTN` | `None` | Passed as `attn_implementation` (e.g. `flash_attention_2`) |
| `MAX_OUTPUT_TOKENS` | `12384` | Per-page generation cap |
| `BBOX_SCALE` | `1000` | Normalization range for model-emitted bboxes |
| `VLLM_API_KEY` | `"EMPTY"` | Sent to the OpenAI client; vLLM ignores it by default |
| `VLLM_API_BASE` | `http://localhost:8000/v1` | OpenAI-compatible endpoint |
| `VLLM_MODEL_NAME` | `chandra` | Must match `--served-model-name` on the server |
| `VLLM_GPUS` | `"0"` | Comma-separated device IDs passed to `docker run --gpus` |
| `MAX_VLLM_RETRIES` | `6` | Repeat-detection retry budget per request |

## Example `local.env`

```
MODEL_CHECKPOINT=datalab-to/chandra-ocr-2
MAX_OUTPUT_TOKENS=12384
TORCH_DEVICE=cuda:0
TORCH_ATTN=flash_attention_2

VLLM_API_BASE=http://localhost:8000/v1
VLLM_MODEL_NAME=chandra
VLLM_GPUS=0
```

The file is found by `find_dotenv` walking up from the working directory, so a project-local `local.env` is picked up automatically. `extra = "ignore"` on the `Config` class means unknown keys don't error.
