# surya — notes

Source: `repos-folder/surya` (VikParuchuri / datalab-to). Package: `surya-ocr` on PyPI, version `0.17.1`. License: GPL-3.0 code, OpenRAIL-M weights. Named after the Hindu sun god.

What it is: a document-OCR toolkit assembled from five PyTorch models, plus the glue around them. Each model is exposed as a stateful `*Predictor` you instantiate once and call repeatedly with PIL images.

The five capabilities:

| Capability | Predictor | Model file |
|---|---|---|
| Text-line detection | `DetectionPredictor` | semantic-segmentation model (modified EfficientViT) trained from scratch |
| Text recognition (OCR) | `RecognitionPredictor` (wraps `FoundationPredictor`) | modified Donut: GQA, MoE layer, UTF-16 decoding |
| Document layout | `LayoutPredictor` (wraps `FoundationPredictor`) | the foundation model run with the `layout` task |
| Reading order | (folded into layout output — `position` field on each box) | same foundation model |
| Table recognition | `TableRecPredictor` | dedicated encoder-decoder model |
| LaTeX OCR (texify) | `TexifyPredictor` | dedicated VLM trained on equations |
| OCR-error detection | `OCRErrorPredictor` | small text classifier — labels page text as "good" or "bad" |

The foundation model (`FoundationPredictor`) is a shared backbone — both recognition and layout are tasks dispatched into it via `TaskNames`.

The package ships seven CLI commands (`surya_ocr`, `surya_detect`, `surya_layout`, `surya_table`, `surya_latex_ocr`, `surya_gui`, `texify_gui`) and corresponding Python predictors. Models auto-download from `https://models.datalab.to` on first use into `~/.cache/datalab/models/`.
