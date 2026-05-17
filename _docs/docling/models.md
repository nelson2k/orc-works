# Models

[docling/models/](../../repos-folder/docling/docling/models/). Holds every ML model used by the pipelines plus the plugin / factory machinery that lets users swap engines.

## Base abstractions

[models/base_model.py](../../repos-folder/docling/docling/models/base_model.py):

- `BaseModelWithOptions` — Protocol requiring `get_options_type()` + `__init__(*, options, **kwargs)`. Anything created via a factory must satisfy this.
- `BasePageModel` — abstract `__call__(conv_res, page_batch) -> Iterable[Page]`. Every per-page model (OCR, layout, table, etc.) implements this.
- `BaseVlmModel` — adds `process_images(image_batch, prompt) -> Iterable[VlmPrediction]`.
- `BaseVlmPageModel` — combines the two; default `__call__` extracts page images, runs `process_images`, attaches results.
- `GenericEnrichmentModel[T]` — used for doc-level enrichment (picture description, classifier, chart extraction). Implements `prepare_element` / `__call__(doc, element_batch)`.

[models/base_layout_model.py](../../repos-folder/docling/docling/models/base_layout_model.py), [base_ocr_model.py](../../repos-folder/docling/docling/models/base_ocr_model.py), [base_table_model.py](../../repos-folder/docling/docling/models/base_table_model.py), and [picture_description_base_model.py](../../repos-folder/docling/docling/models/picture_description_base_model.py) refine these for each stage.

## `stages/` — pipeline stage implementations

```
stages/
├── chart_extraction/     GraniteVision chart-extraction models (v1, v4)
├── code_formula/         CodeFormulaVlmModel for code/formula enrichment
├── layout/               LayoutModel (Heron / Egret) + LayoutObjectDetectionModel
├── ocr/                  EasyOCR / RapidOCR / TesseractOCR / OCRmac / KServe v2 / auto
├── page_assemble/        PageAssembleModel — glues per-page predictions into AssembledUnit
├── page_preprocessing/   PagePreprocessingModel — page-image scaling, lazy backend loading
├── picture_classifier/   DocumentPictureClassifier
├── picture_description/  PictureDescription{Api,Vlm,VlmEngine}Model
├── reading_order/        ReadingOrderModel
├── table_structure/      TableStructureModel (TableFormer v1), TableStructureModelV2, GraniteVisionTableStructureModel
└── vlm_convert/          VlmConvertModel (new VLM-as-converter runtime)
```

Each `stages/<area>/` package generally has one concrete model per engine plus a shared `auto_*` model for engine auto-selection.

## OCR engines (`stages/ocr/`)

| Engine | Module | Backend |
|---|---|---|
| auto | `auto_ocr_model.py` | picks the first available from the others |
| EasyOCR | `easyocr_model.py` | torch |
| RapidOCR | `rapid_ocr_model.py` | onnxruntime / openvino / paddle / torch (configurable) |
| Tesseract (Python bindings) | `tesseract_ocr_model.py` | tesserocr |
| Tesseract (CLI) | `tesseract_ocr_cli_model.py` | shells out to `tesseract` |
| OCRmac | `ocr_mac_model.py` | macOS Vision framework (Darwin only) |
| KServe v2 | `kserve_v2_ocr_model.py` | gRPC client for remote KServe v2 inference servers |

Each is selected via `pipeline_options.ocr_options.kind`. Engine-specific option classes (`EasyOcrOptions`, `RapidOcrOptions`, `TesseractOcrOptions`, `TesseractCliOcrOptions`, `OcrMacOptions`, `KserveV2OcrOptions`) all extend `OcrOptions`.

## Layout engines (`stages/layout/`)

`LayoutModel` — the production model, instantiated with a `LayoutModelConfig` preset (`DOCLING_LAYOUT_HERON` default, `DOCLING_LAYOUT_HERON_101`, `DOCLING_LAYOUT_EGRET_{LARGE,MEDIUM,XLARGE}`, `DOCLING_LAYOUT_V2` legacy). Weights come from `docling-ibm-models` / Hugging Face Hub.

`LayoutObjectDetectionModel` — generic object-detection wrapper used by experimental layouts and the `TableCropsLayoutModel` in `docling/experimental/`.

## Table structure engines (`stages/table_structure/`)

| Engine | Kind | Notes |
|---|---|---|
| `TableStructureModel` | `docling_tableformer` | TableFormer V1 (default). `mode = fast \| accurate`. |
| `TableStructureModelV2` | `docling_tableformer_v2` | TableFormer V2 — same kind of model, improved structure prediction. |
| `GraniteVisionTableStructureModel` | `granite_vision_table` | VLM-based table-structure model. |

## VLM models (`vlm_pipeline_models/`)

`api_vlm_model.py`, `hf_transformers_model.py`, `mlx_model.py`, `vllm_model.py` — the four ways VlmPipeline talks to a VLM. Selected by `InferenceFramework` in `vlm_model_specs.py` (`TRANSFORMERS`, `MLX`, `VLLM`, `API`).

Presets in `datamodel/vlm_model_specs.py`: `GRANITE_VISION_*`, `SMOLDOCLING_*`, `NU_EXTRACT_*`, and more. Each preset carries the `repo_id`, prompt style, response format, and the pipeline framework default.

## Extraction models (`extraction/`)

For `DocumentExtractor` (not the converter). `nuextract_transformers_model.py` runs the NuExtract VLM; `transformers_extraction_model.py` is the legacy general wrapper. `prompt_utils.py` builds extraction prompts from JSON schemas.

## Inference engines (`inference_engines/`)

Shared lower-level engines pluggable into models:

```
inference_engines/
├── common/             Cross-cutting helpers
├── image_classification/
├── object_detection/   Used by LayoutObjectDetectionModel
└── vlm/                Used by PictureDescriptionVlmEngineModel
```

## Factory + plugin system

[models/factories/](../../repos-folder/docling/docling/models/factories/) holds one factory per swappable area (`OcrFactory`, `LayoutFactory`, `TableStructureFactory`, `PictureDescriptionFactory`). They all extend `BaseFactory` and discover registered engines via `pluggy` entry-points.

The built-in registrations live in [models/plugins/defaults.py](../../repos-folder/docling/docling/models/plugins/defaults.py) — four functions (`ocr_engines`, `layout_engines`, `table_structure_engines`, `picture_description`) each return a dict that maps the factory's plugin slot to a list of classes. They're registered through the `pyproject.toml` entry point `[project.entry-points.docling] docling_defaults = "docling.models.plugins.defaults"`.

Third-party packages can ship their own `docling`-namespaced entry points to plug in new engines. The CLI flag `--show-external-plugins` lists everything discovered.

## `experimental/`

[docling/experimental/](../../repos-folder/docling/docling/experimental/) — table-crops layout model and other in-progress work. Plugged in through the same plugin system but not part of the stable surface.
