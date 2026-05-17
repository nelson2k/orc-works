# Settings

[surya/settings.py](../../repos-folder/surya/surya/settings.py). Pydantic-settings, `extra="ignore"`, reads `local.env` from the current working tree via `find_dotenv`. Every field is env-overridable (no prefix — the env var name *is* the field name).

## General

| Field | Default | Notes |
|---|---|---|
| `TORCH_DEVICE` | `None` | Override device pinning. Otherwise computed: cuda → mps → xla → cpu. |
| `IMAGE_DPI` | `96` | Rendering DPI for detection, layout, reading order |
| `IMAGE_DPI_HIGHRES` | `192` | Rendering DPI for OCR and table recognition |
| `IN_STREAMLIT` | `False` | Set true to disable threadpool postprocessing |
| `FLATTEN_PDF` | `True` | Flatten form fields before rendering |
| `DISABLE_TQDM` | `False` | Hide progress bars |
| `S3_BASE_URL` | `https://models.datalab.to` | Where `s3://...` checkpoints resolve |
| `PARALLEL_DOWNLOAD_WORKERS` | `10` | Concurrency of initial model fetch |
| `MODEL_CACHE_DIR` | `~/.cache/datalab/models` (via `platformdirs`) | Where downloaded weights are cached |
| `LOGLEVEL` | `INFO` | |
| `DATA_DIR` | `data` | Default input dir for CLI examples |
| `RESULT_DIR` | `results` | Default output dir for CLI |
| `FONT_DIR` | `<repo>/static/fonts` | Where Go Noto fonts live |

## Computed fields

- `TORCH_DEVICE_MODEL` — resolves auto-detection. Tries `cuda`, `mps`, `xla` (via `import torch_xla`), then `cpu`.
- `MODEL_DTYPE` — `float32` on CPU, `bfloat16` on XLA, `float16` everywhere else.
- `MODEL_DTYPE_BFLOAT` — `float32` on CPU, `bfloat16` on MPS / CUDA / XLA.
- `INFERENCE_MODE` — `torch.no_grad` on XLA, `torch.inference_mode` elsewhere.
- `*_STATIC_CACHE` (per area) — true if `COMPILE_ALL`, the area's `COMPILE_*` flag, or running on XLA.

## Detection

| Field | Default |
|---|---|
| `DETECTOR_BATCH_SIZE` | `None` → 8 (CPU/MPS), 36 (CUDA), 18 (XLA) |
| `DETECTOR_IMAGE_CHUNK_HEIGHT` | `1400` — vertical slicing threshold |
| `DETECTOR_TEXT_THRESHOLD` | `0.6` — heatmap > this is text |
| `DETECTOR_BLANK_THRESHOLD` | `0.35` — heatmap < this is blank |
| `DETECTOR_POSTPROCESSING_CPU_WORKERS` | `min(8, cpu_count)` |
| `DETECTOR_MIN_PARALLEL_THRESH` | `3` — minimum images before threading kicks in |
| `DETECTOR_BOX_Y_EXPAND_MARGIN` | `0.05` — vertical expansion of detected boxes |
| `COMPILE_DETECTOR` | `False` |

## Recognition / foundation

| Field | Default |
|---|---|
| `RECOGNITION_BATCH_SIZE` | `None` → 32 (CPU), 64 (MPS), 256 (CUDA), 128 (XLA) |
| `RECOGNITION_PAD_VALUE` | `255` (white) |
| `FOUNDATION_MODEL_QUANTIZE` | `False` |
| `FOUNDATION_MAX_TOKENS` | `None` |
| `FOUNDATION_CHUNK_SIZE` | `None` |
| `FOUNDATION_PAD_TO_NEAREST` | `256` |
| `FOUNDATION_MULTI_TOKEN_MIN_CONFIDENCE` | `0.9` |
| `COMPILE_FOUNDATION` | `False` |
| `RECOGNITION_RENDER_FONTS` | Go Noto Regular by default, CJK Core for zh/ja/ko |
| `RECOGNITION_FONT_DL_BASE` | go-noto-universal v7.0 GitHub release |

Each recognition batch item uses ~40 MB VRAM (per the README), so 256 ≈ 10 GB.

## Layout

| Field | Default |
|---|---|
| `LAYOUT_BATCH_SIZE` | `None` → 4 (CPU/MPS), 32 (CUDA), 16 (XLA) |
| `LAYOUT_IMAGE_SIZE` | `{height: 768, width: 768}` |
| `LAYOUT_SLICE_MIN` | `{height: 1500, width: 1500}` — slice above this |
| `LAYOUT_SLICE_SIZE` | `{height: 1200, width: 1200}` — slice dimensions |
| `LAYOUT_MAX_BOXES` | `100` |
| `COMPILE_LAYOUT` | `False` |

Each layout batch item ≈ 220 MB VRAM (32 → ~7 GB).

## Table recognition

| Field | Default |
|---|---|
| `TABLE_REC_BATCH_SIZE` | `None` → 8 (CPU/MPS), 32 (CUDA), 16 (XLA) |
| `TABLE_REC_IMAGE_SIZE` | `{height: 768, width: 768}` |
| `TABLE_REC_MAX_BOXES` | `150` |
| `COMPILE_TABLE_REC` | `False` |

Each table-rec batch item ≈ 150 MB VRAM (64 → ~10 GB).

## OCR-error detection

| Field | Default |
|---|---|
| `OCR_ERROR_BATCH_SIZE` | `None` → 8 (CPU/MPS), 64 (CUDA), 32 (XLA) |
| `COMPILE_OCR_ERROR` | `False` |

## Compilation flags

| Env | Effect |
|---|---|
| `COMPILE_DETECTOR=true` | torch.compile detection model |
| `COMPILE_FOUNDATION=true` | torch.compile the foundation model |
| `COMPILE_LAYOUT=true` | (uses foundation cache statically) |
| `COMPILE_TABLE_REC=true` | torch.compile table-rec model |
| `COMPILE_OCR_ERROR=true` | torch.compile classifier |
| `COMPILE_ALL=true` | all of the above |

Compilation flips that area to a static KV cache + batch padding.

## Example `local.env`

```
TORCH_DEVICE=cuda:0
RECOGNITION_BATCH_SIZE=256
DETECTOR_BATCH_SIZE=36
LAYOUT_BATCH_SIZE=32
COMPILE_DETECTOR=true
COMPILE_LAYOUT=true
COMPILE_TABLE_REC=true
DISABLE_TQDM=false
```
