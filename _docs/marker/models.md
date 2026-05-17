# Models

[marker/models.py](../../repos-folder/marker/marker/models.py). One factory:

```python
from marker.models import create_model_dict

models = create_model_dict(device=None, dtype=None, attention_implementation=None)
```

Returns a dict with five surya predictors:

| Key | Class | Used by |
|---|---|---|
| `layout_model` | `surya.layout.LayoutPredictor` (wraps `FoundationPredictor`) | `LayoutBuilder` |
| `recognition_model` | `surya.recognition.RecognitionPredictor` (wraps `FoundationPredictor`) | `OcrBuilder`, `TableProcessor`, `EquationProcessor` |
| `table_rec_model` | `surya.table_rec.TableRecPredictor` | `TableProcessor` |
| `detection_model` | `surya.detection.DetectionPredictor` | `LineBuilder` |
| `ocr_error_model` | `surya.ocr_error.OCRErrorPredictor` | `LineBuilder` |

These are passed as `artifact_dict` to `PdfConverter`. `BaseConverter.resolve_dependencies` inspects each builder/processor's `__init__` signature and pulls the matching key out automatically.

## Device

`device=None` → uses surya defaults, which read `TORCH_DEVICE` from settings (`cuda` if available, else `mps`, else `cpu`). `dtype=None` → `bfloat16` on CUDA, `float32` elsewhere.

## Weight downloads

First call downloads the surya checkpoints from `https://models.datalab.to/artifacts` (and HF) into `~/.cache/datalab` or the surya equivalent. Subsequent runs are offline-capable.

## Memory notes (from README benchmarks)

- ~3.17 GB VRAM average per worker, 5 GB peak.
- On an H100 marker reports ~25 pages/s sustained in batch, 122 pages/s projected with parallel workers.
- On a 4070 (12 GB) you can comfortably run one worker.
