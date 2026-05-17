# Overview

MinerU is one of the more *engineering-heavy* document parsers — it ships a
full client/server/router architecture, multiple deployable inference
backends, and native parsing for the four mainstream office formats. It
came out of OpenDataLab's InternLM pre-training pipeline (the public version
was formerly called "magic-pdf"; the keyword is still in `pyproject.toml`).

## Module map (`mineru/`)

| Subpackage           | Job                                                                              |
|----------------------|----------------------------------------------------------------------------------|
| `cli/`               | All CLI entry points: `client`, `fast_api`, `router`, `vlm_server`, `gradio_app`, `models_download`. |
| `backend/pipeline/`  | Multi-model pipeline backend. `pipeline_analyze.py`, `model_init.py`, table/MFR/OCR wiring. |
| `backend/vlm/`       | VLM backend. `vlm_analyze.py` calls `mineru_vl_utils.MinerUClient`. |
| `backend/hybrid/`    | Hybrid backend mixing VLM with the pipeline OCR for low-hallucination text.     |
| `backend/office/`    | DOCX/PPTX/XLSX native parsers (analyze + middle-JSON + mkcontent).               |
| `backend/utils/`     | Markdown/HTML helpers shared across backends.                                    |
| `model/layout/`      | `PPDocLayoutV2LayoutModel` — page layout detection.                              |
| `model/mfr/`         | Math Formula Recognition: `unimernet` (default) or `pp_formulanet_plus_m`.       |
| `model/ocr/`         | `PytorchPaddleOCR` + seal text crop/warp.                                        |
| `model/table/`       | Table classifier (`PaddleTableClsModel`) + recognizers (`PaddleTableModel` SLANet+, `UnetTableModel`). |
| `model/vlm/`         | `vllm_server.py`, `lmdeploy_server.py` shims.                                   |
| `model/{docx,pptx,xlsx}/` | Office package normalizers + main converters.                              |
| `data/`              | Reader/writer abstractions: file-based, S3, multi-bucket S3, dummy, HTTP.       |
| `utils/`             | 30+ helpers: bbox, language, pdf classify, model download, draw bbox, llm-aided, OCR utils, … |
| `resources/`         | Bundled fasttext-langdetect model + other static assets.                         |

## What runs at runtime

- A configurable subset of models is downloaded on first use from
  ModelScope or HuggingFace into `~/.cache/` (see [settings.md](settings.md)
  and `mineru-models-download` CLI).
- Models are wrapped in singletons per (lang, formula_enable, table_enable)
  tuple — switching languages forces re-init.
- For VLM backends, MinerU prefers vLLM > LMDeploy > mlx > transformers
  depending on platform.

## Lineage / naming

MinerU 2.5 was the previous VLM. 3.1.0 introduces **MinerU2.5-Pro-2604-1.2B**
(1.2B params, supports image/chart parsing, truncated-paragraph merge,
cross-page tables, in-table image recognition). The older AGPLv3 models
`doclayoutyolo` and `mfd_yolov8`, and the CC-BY-NC-SA `layoutreader`, were
removed in 3.0.0 specifically to clear the licensing path.
