# Package layout

Contents of `repos-folder/surya/`:

```
surya/                          main Python package
├── __init__.py                 (empty)
├── settings.py                 pydantic-settings: device, DPI, batch sizes, checkpoints
├── logging.py                  configure_logging() + get_logger()
├── models.py                   load_predictors() one-shot factory
├── common/                     shared abstractions
│   ├── load.py                 ModelLoader base
│   ├── predictor.py            BasePredictor base
│   ├── pretrained.py           HF Hub helpers
│   ├── polygon.py              PolygonBox
│   ├── util.py                 misc helpers (clean_boxes, ...)
│   ├── s3.py                   parallel S3 downloader for s3:// checkpoints
│   ├── xla.py                  mark_step() shim for XLA
│   ├── adetr/                  additive-DETR decoder (used by table rec)
│   ├── donut/                  Donut encoder + processor
│   └── surya/                  the custom Surya backbone
│       ├── config.py
│       ├── encoder/, decoder/, embedder/, processor/
│       ├── flash_attn_utils.py
│       └── schema.py           TaskNames
├── detection/                  text-line detection
│   ├── __init__.py             DetectionPredictor
│   ├── loader.py               DetectionModelLoader
│   ├── model/                  the segmentation model
│   ├── processor.py
│   ├── heatmap.py              parallel_get_boxes
│   ├── util.py                 split_image / get_total_splits
│   ├── parallel.py             FakeExecutor (drop-in single-thread ThreadPoolExecutor)
│   └── schema.py               TextDetectionResult
├── foundation/                 the shared backbone
│   ├── __init__.py             FoundationPredictor (continuous batching)
│   ├── loader.py
│   ├── util.py                 detect_repeat_token, prediction_to_polygon_batch
│   └── cache/                  dynamic_ops / static_ops KV cache implementations
├── recognition/                OCR
│   ├── __init__.py             RecognitionPredictor
│   ├── postprocessing.py       fix_unbalanced_tags
│   ├── util.py                 sort_text_lines / clean_close_polygons / words_from_chars / ...
│   ├── languages.py            CODE_TO_LANGUAGE table (90+ languages)
│   └── schema.py               TextChar, TextWord, TextLine, OCRResult
├── layout/                     layout + reading order
│   ├── __init__.py             LayoutPredictor (wraps FoundationPredictor)
│   ├── label.py                LAYOUT_PRED_RELABEL mapping
│   └── schema.py               LayoutBox, LayoutResult
├── ocr_error/                  OCR-quality classifier
│   ├── __init__.py             OCRErrorPredictor
│   ├── loader.py
│   ├── tokenizer.py
│   ├── model/                  small classifier
│   └── schema.py               OCRErrorDetectionResult
├── table_rec/                  table structure
│   ├── __init__.py             TableRecPredictor
│   ├── loader.py
│   ├── processor.py
│   ├── shaper.py               LabelShaper: tokens ↔ structured cells
│   ├── model/                  table-rec encoder-decoder
│   └── schema.py               TableCell, TableRow, TableCol, TableResult
├── input/
│   ├── load.py                 load_pdf / load_image / load_from_file / load_from_folder
│   └── processing.py           open_pdf, get_page_images, slice_polys_from_image, ...
├── scripts/                    CLI entry points
│   ├── config.py               CLILoader + common_options
│   ├── detect_text.py          surya_detect
│   ├── ocr_text.py             surya_ocr
│   ├── detect_layout.py        surya_layout
│   ├── table_recognition.py    surya_table
│   ├── ocr_latex.py            surya_latex_ocr
│   ├── run_streamlit_app.py    surya_gui launcher
│   ├── streamlit_app.py        the streamlit app body
│   ├── run_texify_app.py       texify_gui launcher
│   ├── texify_app.py           texify streamlit app
│   ├── finetune_ocr.py         HF Trainer-based finetune script
│   └── hf_to_s3.py             maintainer mirror tool
└── debug/                      draw_text_on_image and other rendering helpers
```

## Call graph (`surya_ocr` end to end)

```
surya_ocr CLI (scripts/ocr_text.py)
  ├─ CLILoader.load_from_file → input/load.py → pypdfium2 / PIL
  ├─ FoundationPredictor()                             # loads recognition checkpoint
  ├─ DetectionPredictor()
  ├─ RecognitionPredictor(foundation_predictor)
  └─ recognition(images, det_predictor=detection, highres_images=..., math_mode=...)
       ├─ detection(images) → List[TextDetectionResult]  (heatmap → parallel_get_boxes)
       ├─ slice_polys_from_image(highres, scaled_polys)
       └─ foundation.prediction_loop(slices, task_names=ocr_with_boxes)
            ├─ continuous-batch decode with KV cache
            ├─ detect_repeat_token guards against degeneration
            └─ prediction_to_polygon_batch turns bbox tokens back into PolygonBoxes
       → OCRResult per image with TextLine / TextChar / TextWord
```

## Other top-level files in `repos-folder/surya/`

- `README.md`, `CITATION.cff`, `LICENSE`, `MODEL_LICENSE`, `CLA.md`
- `pyproject.toml` / `poetry.lock`
- `pytest.ini` / `tests/`
- `benchmark/` — `detection.py`, `recognition.py`, `layout.py`, `ordering.py`, `table_recognition.py`, `texify.py` — each measures one capability against an open dataset
- `static/` — fonts (`GoNotoCurrent-Regular.ttf`, `GoNotoCJKCore.ttf`) plus example images for the README
- `signatures/` — CLA signatures
- `detect_layout.py`, `detect_text.py`, `ocr_app.py`, `ocr_latex.py`, `ocr_text.py`, `table_recognition.py`, `texify_app.py` — repo-root shim scripts that just re-run the corresponding entry points (useful when running from a clone without installing)
