# Python usage

Every predictor is callable on a list of PIL `Image` instances. Load the model dict once and reuse it.

## Load everything at once

```python
from surya.models import load_predictors

predictors = load_predictors(device="cuda")    # or "mps", "cpu", "xla"

detection      = predictors["detection"]        # DetectionPredictor
recognition    = predictors["recognition"]      # RecognitionPredictor (wraps a FoundationPredictor)
layout         = predictors["layout"]           # LayoutPredictor (wraps a FoundationPredictor)
table_rec      = predictors["table_rec"]        # TableRecPredictor
ocr_error      = predictors["ocr_error"]        # OCRErrorPredictor
```

Note: `load_predictors` creates *two* `FoundationPredictor` instances (one for recognition, one for layout) so each can pin its own checkpoint. If you want them to share, instantiate manually as below.

## OCR — full pipeline (detect + recognize)

```python
from PIL import Image
from surya.foundation import FoundationPredictor
from surya.recognition import RecognitionPredictor
from surya.detection import DetectionPredictor
from surya.settings import settings

foundation = FoundationPredictor(checkpoint=settings.RECOGNITION_MODEL_CHECKPOINT)
recognition = RecognitionPredictor(foundation)
detection = DetectionPredictor()

image = Image.open("page.png")
predictions = recognition([image], det_predictor=detection)
# predictions: List[OCRResult]
for line in predictions[0].text_lines:
    print(line.text, line.confidence, line.bbox)
```

For higher-resolution OCR, render the page twice — low-res (96 DPI) for detection, high-res (192 DPI) for recognition — and pass both:

```python
predictions = recognition([lowres_image], det_predictor=detection,
                          highres_images=[highres_image])
```

## OCR — bring your own boxes

```python
predictions = recognition(
    [image],
    task_names=["ocr_with_boxes"],
    bboxes=[[[100, 50, 800, 90], [100, 100, 800, 140]]],   # per-image list of [x0,y0,x1,y1]
)
```

Or polygons:

```python
predictions = recognition(
    [image],
    task_names=["ocr_with_boxes"],
    polygons=[[ [[100,50],[800,50],[800,90],[100,90]] ]],
)
```

## Text-line detection only

```python
from surya.detection import DetectionPredictor

detection = DetectionPredictor()
results = detection([image])
# results: List[TextDetectionResult]
for box in results[0].bboxes:
    print(box.polygon, box.confidence)
```

## Layout + reading order

```python
from PIL import Image
from surya.foundation import FoundationPredictor
from surya.layout import LayoutPredictor
from surya.settings import settings

layout = LayoutPredictor(FoundationPredictor(checkpoint=settings.LAYOUT_MODEL_CHECKPOINT))

image = Image.open("page.png")
results = layout([image])
# results: List[LayoutResult]
for box in sorted(results[0].bboxes, key=lambda b: b.position):
    print(box.position, box.label, box.bbox, box.top_k)
```

`box.position` is the reading-order index. `box.top_k` is a `{label: confidence}` dict for alternative labels.

## Table recognition

Crop each table to its own image (e.g. using a `LayoutPredictor` first), then:

```python
from surya.table_rec import TableRecPredictor

table_rec = TableRecPredictor()
results = table_rec([table_crop])
# results: List[TableResult]
for cell in results[0].cells:
    print(cell.row_id, cell.col_id, cell.colspan, cell.rowspan,
          cell.is_header, cell.bbox)
```

## LaTeX OCR

```python
from surya.texify import TexifyPredictor

texify = TexifyPredictor()
latex_results = texify([equation_crop])
```

## OCR-error detection

```python
from surya.ocr_error import OCRErrorPredictor

ocr_error = OCRErrorPredictor()
res = ocr_error(["the joined text of one page", "another page text"])
# res.labels -> ["good", "bad"]
```

## Sharing the foundation model

If you want layout and recognition to share weights — they're the same model class, but the README defaults pin them to different checkpoints. If you do choose to share:

```python
from surya.foundation import FoundationPredictor
from surya.layout import LayoutPredictor
from surya.recognition import RecognitionPredictor
from surya.settings import settings

# both layout and recognition currently default to "s3://text_recognition/2025_09_23"
# and "s3://layout/2025_09_23" — sharing only makes sense if you've trained
# a single multi-task checkpoint that supports both tasks.
shared = FoundationPredictor(checkpoint=settings.RECOGNITION_MODEL_CHECKPOINT)
recognition = RecognitionPredictor(shared)
layout = LayoutPredictor(shared)
```

## Disable progress bars / debug

Either set `DISABLE_TQDM=true` in `local.env` / env, or per-instance:

```python
detection.disable_tqdm = True
```

`RecognitionPredictor` and `LayoutPredictor` forward `disable_tqdm` to their wrapped `FoundationPredictor` automatically.

## Force a device

Each predictor accepts `device=` and `dtype=`:

```python
DetectionPredictor(device="cuda:1", dtype=torch.float16)
```

Or set `TORCH_DEVICE=cuda:1` in env and let `settings.TORCH_DEVICE_MODEL` route it for you.
