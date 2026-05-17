# Models

Surya ships five trainable models plus a small fast text-classifier. Weights live in `s3://<bucket>/<area>/<date>` URLs that resolve against `S3_BASE_URL=https://models.datalab.to`.

## Checkpoints (from `settings.py`)

| Setting | Default | Used by |
|---|---|---|
| `DETECTOR_MODEL_CHECKPOINT` | `s3://text_detection/2025_05_07` | `DetectionPredictor` |
| `FOUNDATION_MODEL_CHECKPOINT` | `s3://text_recognition/2025_09_23` | `FoundationPredictor` (when no override is passed) |
| `RECOGNITION_MODEL_CHECKPOINT` | `s3://text_recognition/2025_09_23` | passed to `FoundationPredictor` for `RecognitionPredictor` |
| `LAYOUT_MODEL_CHECKPOINT` | `s3://layout/2025_09_23` | passed to `FoundationPredictor` for `LayoutPredictor` |
| `TABLE_REC_MODEL_CHECKPOINT` | `s3://table_recognition/2025_02_18` | `TableRecPredictor` |
| `OCR_ERROR_MODEL_CHECKPOINT` | `s3://ocr_error_detection/2025_02_18` | `OCRErrorPredictor` |

The dates in the paths are version pins — surya bumps these when a new training run is promoted. Override via env (e.g. `LAYOUT_MODEL_CHECKPOINT=hf:hub/path`) or by passing `checkpoint=...` to a predictor.

`s3:` URLs are downloaded once per host into `MODEL_CACHE_DIR`. `hf:`-prefixed strings are fetched directly from the HuggingFace Hub.

## Architecture summary (from README)

### Text detection

Trained from scratch on a modified [EfficientViT](https://github.com/mit-han-lab/efficientvit) for semantic segmentation. Outputs a heatmap which is thresholded and polygon-extracted by `parallel_get_boxes` in [detection/heatmap.py](../../repos-folder/surya/surya/detection/heatmap.py). Trained 3 days on 4× A6000 across a diverse image set.

### Text recognition (foundation)

Modified [Donut](https://github.com/clovaai/donut) decoder with:

- GQA (grouped-query attention),
- a sparse MoE layer,
- UTF-16 decoding (lets a single token cover most Unicode planes efficiently),
- layer-config changes.

Plus a [Segformer](https://arxiv.org/pdf/2105.15203.pdf)-derived vision encoder. Trained 2 weeks on 4× A6000. Source for the encoder lives at [common/donut/encoder.py](../../repos-folder/surya/surya/common/donut/encoder.py); the decoder + custom Surya model code lives under [common/surya/](../../repos-folder/surya/surya/common/surya/) (`encoder/`, `decoder/`, `embedder/`, `processor/`, `flash_attn_utils.py`, `config.py`, `schema.py`).

### Layout

Shares the recognition backbone. Layout is just `TaskNames.layout` dispatched through `FoundationPredictor` — same weights as recognition for the encoder, different decoder head and prompt. The output tokens (`<section-header>`, `<table>`, …) are remapped to friendly labels in [layout/label.py](../../repos-folder/surya/surya/layout/label.py).

### Reading order

Folded into layout. Each `LayoutBox` carries a `position` integer — the order in which the model emitted the box, which is the predicted reading order.

### Table recognition

Stand-alone encoder-decoder model. Encoder is a Donut-style vision encoder (`common/donut/encoder.py`); decoder is the additive-DETR-style decoder in [common/adetr/decoder.py](../../repos-folder/surya/surya/common/adetr/decoder.py). Box-property tokens (cell, row_id, col_id, span, header, merge) are decoded via `BOX_PROPERTIES` in `table_rec/model/config.py` and reshaped by `LabelShaper` in [table_rec/shaper.py](../../repos-folder/surya/surya/table_rec/shaper.py).

### OCR-error detection

Small DistilBERT-style classifier. Input is the joined text of a page; output is a single label per page. Used by downstream pipelines to decide whether native PDF text is trustworthy or whether OCR is needed.

### Texify (LaTeX OCR)

VLM trained on equation crops. Returns LaTeX. Lives in the `surya.texify` namespace.

## Loaders

Each predictor uses its own `ModelLoader` subclass (`detection/loader.py`, `foundation/loader.py`, `table_rec/loader.py`, `ocr_error/loader.py`). They all extend [common/load.py: ModelLoader](../../repos-folder/surya/surya/common/load.py) and implement `model(device, dtype, attention_implementation)` and `processor()`. The base class is intentionally minimal — checkpoint is the only ctor arg.

S3 downloading is handled by [common/s3.py](../../repos-folder/surya/surya/common/s3.py): parallel chunked download with `PARALLEL_DOWNLOAD_WORKERS=10`.

## Compilation

torch.compile can be enabled per model via env:

| Env var | Predictor it affects |
|---|---|
| `COMPILE_DETECTOR=true` | DetectionPredictor |
| `COMPILE_LAYOUT=true` | LayoutPredictor (foundation) |
| `COMPILE_FOUNDATION=true` | FoundationPredictor / RecognitionPredictor |
| `COMPILE_TABLE_REC=true` | TableRecPredictor |
| `COMPILE_OCR_ERROR=true` | OCRErrorPredictor |
| `COMPILE_ALL=true` | everything |

When compilation is on for an area, that area uses a static KV cache (`<area>_STATIC_CACHE` computed in `settings.py`) and pads batches to `batch_size`. Static cache is also forced for XLA regardless of compile flags.

README-reported speedups on an A10: detection +3.3%, layout +0.9%, table rec +11.5%. Recognition isn't in the table (compile path is supported via `COMPILE_FOUNDATION` but speedup wasn't published).
