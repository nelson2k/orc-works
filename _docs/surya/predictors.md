# Predictors

Every surya capability is exposed as a `*Predictor` class. They all share a common shape defined in [common/predictor.py](../../repos-folder/surya/surya/common/predictor.py): `BasePredictor`.

## `BasePredictor`

```python
class BasePredictor:
    model_loader_cls = ModelLoader
    batch_size: Optional[int] = None
    default_batch_sizes = {"cpu": 1, "mps": 1, "cuda": 1}
    torch_dtype = settings.MODEL_DTYPE
```

Construction:

```python
predictor = SomePredictor(
    checkpoint=None,                          # override settings.<TYPE>_MODEL_CHECKPOINT
    device=settings.TORCH_DEVICE_MODEL,       # cpu / cuda / mps / xla
    dtype=None,                               # float32 on cpu, bfloat16 on xla, float16 elsewhere
    attention_implementation=None,            # e.g. "flash_attention_2"
)
```

The constructor:

1. Builds a `model_loader_cls(checkpoint)` and asks it for the torch model (`.model(device, dtype, attention_implementation)`) and the processor (`.processor()`).
2. Stores `self.model`, `self.processor`, `self._disable_tqdm`.

Each subclass overrides `model_loader_cls` and `default_batch_sizes`, then implements `__call__(images, batch_size=None, ...) -> List[Result]`.

## The five concrete predictors

### `DetectionPredictor` ([detection/__init__.py](../../repos-folder/surya/surya/detection/__init__.py))

- `default_batch_sizes = {"cpu": 8, "mps": 8, "cuda": 36, "xla": 18}`.
- Single-purpose semantic-segmentation model that emits a heatmap. `parallel_get_boxes` (in `detection/heatmap.py`) converts heatmaps to `PolygonBox`es using thresholds `DETECTOR_TEXT_THRESHOLD=0.6` and `DETECTOR_BLANK_THRESHOLD=0.35`.
- Postprocessing runs in a `ThreadPoolExecutor` (`DETECTOR_POSTPROCESSING_CPU_WORKERS`, max 8) when the batch has ≥`DETECTOR_MIN_PARALLEL_THRESH=3` images.
- Large images are vertically sliced at `DETECTOR_IMAGE_CHUNK_HEIGHT=1400` and the heatmaps stitched back.
- Returns `List[TextDetectionResult]` (`bboxes`, `heatmap`, `affinity_map`, `image_bbox`). Heatmaps are only included when `include_maps=True`.

### `FoundationPredictor` ([foundation/__init__.py](../../repos-folder/surya/surya/foundation/__init__.py))

The shared backbone for both recognition and layout. Not usually called directly — you hand it to `RecognitionPredictor` and/or `LayoutPredictor`.

- `default_batch_sizes = {"cpu": 32, "mps": 64, "cuda": 256, "xla": 64}`.
- `encoder_chunk_sizes = {"cpu": 4096, "mps": 4096, "cuda": 32768, "xla": 32768}`.
- Knows about five tasks (`TaskNames` in `common/surya/schema.py`): `ocr_with_boxes`, `ocr_without_boxes`, `block_without_boxes`, `layout`, `table_structure`. Each task has its own image-size, max-token, and bbox-presence settings.
- Implements continuous batching for autoregressive decoding: maintains a `prompt_queue`, KV cache (dynamic or static depending on `FOUNDATION_STATIC_CACHE`), and a `batch_prompt_mapping`.
- Repeat-token detection (`foundation/util.py: detect_repeat_token`) guards against degenerate "the the the" loops.

### `RecognitionPredictor` ([recognition/__init__.py](../../repos-folder/surya/surya/recognition/__init__.py))

- `default_batch_sizes = {"cpu": 32, "mps": 64, "cuda": 256, "xla": 128}`.
- Constructed with a `FoundationPredictor` — does *not* load its own model. Shares `processor`, `bbox_size`, `tasks` from the foundation.
- Three entry shapes:
  1. `predictor(images, det_predictor=...)` — runs detection first, slices the lines, OCR each.
  2. `predictor(images, bboxes=[[...]])` — caller provides bboxes per image.
  3. `predictor(images, polygons=[[...]])` — caller provides 4-corner polygons.
- For high-resolution OCR, pass `highres_images=...` (rendered at `IMAGE_DPI_HIGHRES=192`) alongside the low-res images used for detection.
- Post-OCR fixes: `fix_unbalanced_tags`, `clean_math_tags`, `filter_blacklist_tags`, `words_from_chars`, `clean_close_polygons` (in `recognition/postprocessing.py` and `recognition/util.py`).
- Returns `List[OCRResult]` — `text_lines` of `TextLine` with `chars`, `words`, `bbox`, `polygon`, `confidence`, `original_text_good`.

### `LayoutPredictor` ([layout/__init__.py](../../repos-folder/surya/surya/layout/__init__.py))

- `default_batch_sizes = {"cpu": 4, "mps": 4, "cuda": 32, "xla": 16}`.
- Also wraps a `FoundationPredictor`. Runs the foundation model with `TaskNames.layout`, `max_tokens=500`, `top_k=5`.
- Each emitted token decodes to a `<tag>` (e.g. `<section-header>`) which is mapped via `LAYOUT_PRED_RELABEL` in `layout/label.py` to a friendly label (`SectionHeader`, `Text`, `Picture`, …).
- `top_k` predictions are kept so downstream code can do its own label arbitration.
- Returns `List[LayoutResult]` — `bboxes: List[LayoutBox]` with `polygon`, `label`, `position` (reading order), `top_k`, `confidence`; plus `image_bbox` and a `sliced` flag.

### `TableRecPredictor` ([table_rec/__init__.py](../../repos-folder/surya/surya/table_rec/__init__.py))

- `default_batch_sizes = {"cpu": 8, "mps": 8, "cuda": 32, "xla": 16}`.
- Standalone model (own loader, own weights). Inputs are images already cropped to a single table; `TABLE_REC_MAX_BOXES=150` caps decoder length.
- Returns `List[TableResult]` — `cells`, `unmerged_cells`, `rows`, `cols`, `image_bbox`. Cells carry `row_id`, `col_id`, `cell_id`, `colspan`, `rowspan`, `is_header`, `merge_up`, `merge_down`, optional `text_lines`.
- Box property encoding is defined in `table_rec/model/config.py` (`BOX_PROPERTIES`, `BOX_DIM`, `CATEGORY_TO_ID`); `LabelShaper` (`table_rec/shaper.py`) translates between decoder logits and structured cells.

### `OCRErrorPredictor` ([ocr_error/__init__.py](../../repos-folder/surya/surya/ocr_error/__init__.py))

- `default_batch_sizes = {"cpu": 8, "mps": 8, "cuda": 64, "xla": 32}`.
- Small text classifier. Input: page text (joined from a detector / native PDF text). Output: per-page label `"good"` or `"bad"` (`ID2LABEL` in `ocr_error/model/config.py`).
- Returns `OCRErrorDetectionResult(texts, labels)`.

## `load_predictors()` convenience

[surya/models.py](../../repos-folder/surya/surya/models.py) exports a one-call factory:

```python
from surya.models import load_predictors

predictors = load_predictors(device="cuda")
# -> {"layout": LayoutPredictor, "ocr_error": OCRErrorPredictor,
#     "recognition": RecognitionPredictor, "detection": DetectionPredictor,
#     "table_rec": TableRecPredictor}
```

Note layout and recognition each get their own `FoundationPredictor` instance — different checkpoints (`LAYOUT_MODEL_CHECKPOINT` vs `RECOGNITION_MODEL_CHECKPOINT`).

## Texify (LaTeX OCR)

`TexifyPredictor` (imported as `from surya.texify import TexifyPredictor`) wraps a dedicated equation-OCR VLM. Same shape as the others — `predictor([image])` returns LaTeX strings. Input images should already be cropped to an equation.
