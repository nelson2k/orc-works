# Settings (env / paths)

[marker/settings.py](../../repos-folder/marker/marker/settings.py). Pydantic-settings, reads `local.env` from the current working tree (via `find_dotenv`). Field summary:

| Field | Default | Notes |
|---|---|---|
| `BASE_DIR` | repo root | derived from file location |
| `OUTPUT_DIR` | `BASE_DIR/conversion_results` | where CLI saves output by default |
| `FONT_DIR` | `BASE_DIR/static/fonts` | needs to contain `GoNotoCurrent-Regular.ttf` |
| `DEBUG_DATA_FOLDER` | `BASE_DIR/debug_data` | where debug processor dumps |
| `ARTIFACT_URL` | `https://models.datalab.to/artifacts` | font/weight mirror |
| `FONT_NAME` | `GoNotoCurrent-Regular.ttf` | required for non-PDF providers (weasyprint render) |
| `LOGLEVEL` | `INFO` | |
| `OUTPUT_ENCODING` | `utf-8` | |
| `OUTPUT_IMAGE_FORMAT` | `JPEG` | extracted images saved as JPG |
| `GOOGLE_API_KEY` | empty | mirrored to `gemini_api_key` in `ConfigParser` |
| `TORCH_DEVICE` | auto | override device pinning |
| `TORCH_DEVICE_MODEL` (computed) | `cuda` / `mps` / `cpu` | what surya actually uses |
| `MODEL_DTYPE` (computed) | `bfloat16` on CUDA, else `float32` | |

`download_font` (in [marker/util.py](../../repos-folder/marker/marker/util.py)) fetches `FONT_NAME` from `ARTIFACT_URL` on first run; required by HTML/DOCX/PPTX/etc providers.

## What to set

For a one-off Windows install with a 4070:
- `TORCH_DEVICE=cuda` if surya doesn't autodetect.
- `GOOGLE_API_KEY=…` only if you want `--use_llm`.
- Leave the rest at defaults.

The `local.env` lookup means a project-local env file is picked up without code changes.
