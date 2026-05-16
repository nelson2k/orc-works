# Settings (`marker/settings.py`)

Single `pydantic_settings.BaseSettings` instance (`settings`) loaded from a
`local.env` (via `find_dotenv("local.env")`) plus environment overrides.

| Field | Default | Notes |
|-------|---------|-------|
| `BASE_DIR` | repo root | Computed from this file's location. |
| `OUTPUT_DIR` | `<repo>/conversion_results` | Default for `--output_dir`. |
| `FONT_DIR` | `<repo>/static/fonts` | Where `GoNotoCurrent-Regular.ttf` ends up after the auto-download. |
| `DEBUG_DATA_FOLDER` | `<repo>/debug_data` | Default debug dump location. |
| `ARTIFACT_URL` | `https://models.datalab.to/artifacts` | Where the font is fetched from. |
| `FONT_NAME` / `FONT_PATH` | `GoNotoCurrent-Regular.ttf` | Used by WeasyPrint-backed providers. |
| `LOGLEVEL` | `INFO` | Used by `logger.py`. |
| `OUTPUT_ENCODING` | `utf-8` | For text writes. |
| `OUTPUT_IMAGE_FORMAT` | `JPEG` | Used when saving images and base64 encoding. |
| `GOOGLE_API_KEY` | `""` | Mirrored as `gemini_api_key` by `ConfigParser`. |
| `TORCH_DEVICE` | `None` | Override device picking. |
| `TORCH_DEVICE_MODEL` *(computed)* | `cuda` / `mps` / `cpu` | What the builders actually check. MPS is downgraded to CPU for text detection (kept here for everything else). |
| `MODEL_DTYPE` *(computed)* | `torch.bfloat16` on CUDA, else `float32` | Used by `create_model_dict`. |

Useful env overrides:

```bash
TORCH_DEVICE=cuda            # force device
GOOGLE_API_KEY=...           # gemini key
```

`local.env` is searched relative to the working directory tree, so per-project
configs work without editing source.
