# Settings (`chandra/settings.py`)

Single `pydantic_settings.BaseSettings` instance loaded from a `local.env` via
`find_dotenv("local.env")` plus normal environment overrides. Unknown keys are
ignored (`extra = "ignore"`).

| Field                | Default                                    | Notes |
|----------------------|--------------------------------------------|-------|
| `BASE_DIR`           | repo root                                  | Computed from file location. |
| `IMAGE_DPI`          | 192                                        | PDF render DPI floor. |
| `MIN_PDF_IMAGE_DIM`  | 1024                                       | Shortest side floor (px) when rendering PDF pages. |
| `MIN_IMAGE_DIM`      | 1536                                       | Shortest side floor for image inputs; upscaled with LANCZOS if smaller. |
| `MODEL_CHECKPOINT`   | `datalab-to/chandra-ocr-2`                 | HuggingFace model ID. |
| `TORCH_DEVICE`       | `None`                                     | If set (e.g. `cuda:0`), forces `device_map={"": <val>}`; else `"auto"`. |
| `MAX_OUTPUT_TOKENS`  | 12384                                      | Per-page cap on generated tokens. |
| `TORCH_ATTN`         | `None`                                     | Optional `attn_implementation`, e.g. `flash_attention_2`. |
| `BBOX_SCALE`         | 1000                                       | The 0..1000 grid the model emits bboxes in. |
| `VLLM_API_KEY`       | `"EMPTY"`                                  | vLLM ignores this but the OpenAI client wants a value. |
| `VLLM_API_BASE`      | `http://localhost:8000/v1`                 | Where the vLLM server is. |
| `VLLM_MODEL_NAME`    | `chandra`                                  | `served-model-name` exposed by the vLLM server. |
| `VLLM_GPUS`          | `"0"`                                      | Device-IDs string passed to `docker --gpus device=…`. |
| `MAX_VLLM_RETRIES`   | 6                                          | Per-request retry budget on repeat-token / transient errors. |

## Example `local.env`

```bash
# HF mode on a 12 GB GPU
TORCH_DEVICE=cuda:0
TORCH_ATTN=flash_attention_2
MAX_OUTPUT_TOKENS=12384

# vLLM client pointing at a remote server
VLLM_API_BASE=https://chandra.example.com/v1
VLLM_MODEL_NAME=chandra-ocr-2
```

`local.env` is searched relative to the working directory tree, so per-project
configs work without editing source. Env vars always win over `local.env`.
