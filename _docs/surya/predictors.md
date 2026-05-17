# Surya Predictors

Surya exposes predictor classes for each model family.

## `surya.models.load_predictors`

`load_predictors(device=None, dtype=None)` returns:

```python
{
    "layout": LayoutPredictor(...),
    "ocr_error": OCRErrorPredictor(...),
    "recognition": RecognitionPredictor(...),
    "detection": DetectionPredictor(...),
    "table_rec": TableRecPredictor(...),
}
```

The layout and recognition predictors share a `FoundationPredictor` checkpoint.
Detection, OCR-error, and table-recognition predictors can be created with a
device and dtype.

## How Marker uses this idea

Marker's `marker.models.create_model_dict(...)` creates a similar dict with:

- `layout_model`
- `recognition_model`
- `table_rec_model`
- `detection_model`
- `ocr_error_model`

Those are injected into Marker builders and processors.

## Overlay relevance

Each predictor produces geometric outputs that can be drawn over a PDF page:

- detection: text-line polygons
- recognition: line/word/char polygons with text
- layout: labeled layout polygons
- table-rec: table cells, rows, columns
