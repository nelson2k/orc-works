# Backends

MinerU 3.x has four logical analyze passes, organized under [mineru/backend/](../../repos-folder/MinerU/mineru/backend/). All four end in the same "middle JSON" representation, after which a renderer emits Markdown / JSON.

## pipeline ([backend/pipeline/](../../repos-folder/MinerU/mineru/backend/pipeline/))

Classic vision-pipeline approach. Per-page:

1. **Classify** PDF (`utils/pdf_classify.py`) — decide "scanned" vs "text-based".
2. **Layout** (`model/layout/pp_doclayoutv2.py`) — PP-DocLayout v2 detects blocks (title, paragraph, table, figure, formula, ...).
3. **OCR** (`model/ocr/pytorch_paddle.py`) — text-line det → rec on raster crops. PaddleOCR ports re-implemented in PyTorch.
4. **MFR** (`model/mfr/`) — UnimerNet or FormulaNet+ for math regions.
5. **Table** (`model/table/`) — `cls/` decides table style, `rec/` reconstructs structure as HTML.

Driver: `pipeline_analyze.py` runs `MineruPipelineModel` (cached in `ModelSingleton`) for each page bitmap. Output goes through `model_json_to_middle_json.py` which slides a window across pages and writes batch-completed pages to disk early.

Outputs:

- `pipeline_magic_model.py` — strips/normalizes raw model outputs.
- `pipeline_middle_json_mkcontent.py` — middle JSON → Markdown/content list.
- `para_split.py` — paragraph reconstruction with reading-order.

Strengths: no hallucination, works on CPU. Weaknesses: lower accuracy on heavily designed layouts.

OmniDocBench v1.5 score in 3.0.0: 86.2.

## VLM ([backend/vlm/](../../repos-folder/MinerU/mineru/backend/vlm/))

Vision-language model approach: render each page as an image and prompt a fine-tuned VLM (`MinerU2.5-Pro-2604-1.2B` as of 3.1.0) to output a structured representation of the whole page.

Driver: `vlm_analyze.py`. Engine-agnostic — chooses one of:

| Engine | When |
|---|---|
| `transformers` | Local PyTorch path (CUDA / MPS / NPU / ...) |
| `vllm` | Linux + CUDA, high throughput |
| `lmdeploy` | Windows + CUDA |
| `mlx-vlm` | Apple Silicon |
| HTTP client | Remote OpenAI-compatible server (`vlm-http-client`) |

Engine pick logic is in `utils/engine_utils.py`. Models cached via `shutdown_cached_models()` for clean exit.

Output goes through:

- `vlm_magic_model.py` — parses the VLM's structured text into typed items.
- `model_output_to_middle_json.py` — converts those into middle JSON.
- `vlm_middle_json_mkcontent.py` — middle JSON → Markdown.

Strengths: high accuracy on complex layouts, charts, image-inside-table. Weaknesses: VRAM-hungry; occasional hallucination on dense numeric tables (mitigated in hybrid).

## Hybrid ([backend/hybrid/](../../repos-folder/MinerU/mineru/backend/hybrid/))

Combines native text extraction (anchored to true PDF text positions) with the VLM. The VLM identifies layout & semantics; the native extractor supplies the text — eliminating most VLM hallucinations.

Driver: `hybrid_analyze.py`. Calls into pipeline components for layout and into VLM components for semantic structure, then `hybrid_magic_model.py` reconciles. `hybrid_model_output_to_middle_json.py` builds the middle JSON.

The pipeline batch ratio is tuned via `MINERU_HYBRID_BATCH_RATIO`. `MINERU_FORCE_VLM_OCR_ENABLE` / `MINERU_HYBRID_FORCE_PIPELINE_ENABLE` override the auto fall-through.

Hybrid backends require the `pipeline` extra (`torch`). The CLI raises `HybridDependencyError` from `cli/common.py:ensure_backend_dependencies` if `torch` isn't importable.

## Office ([backend/office/](../../repos-folder/MinerU/mineru/backend/office/))

Native parsers — no rasterization, no VLM. Added in 3.0 for DOCX, extended in 3.1 to PPTX and XLSX.

- `docx_analyze.py` — uses `python-docx` + `mammoth` + `lxml`.
- `pptx_analyze.py` — uses `pypptx-with-oxml`.
- `xlsx_analyze.py` — uses `openpyxl` + `pandas`.
- `mkcontent/` — emits middle-JSON-shaped output.
- `office_middle_json_mkcontent.py` — final Markdown rendering.

The office parsers run *very* fast (no models) and lossless w.r.t. tables / lists / heading hierarchy. Selected by file suffix at CLI level, not by the `--backend` flag.

## What the orchestrator picks

The `--backend` flag selects pipeline / vlm / hybrid only for PDF and image inputs. For DOCX / PPTX / XLSX, the office parsers are always used.
