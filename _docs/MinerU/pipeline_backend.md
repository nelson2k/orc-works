# Pipeline backend (`mineru/backend/pipeline/`)

The "classic" multi-model parser. Lives in `mineru.backend.pipeline` and
wires together five specialised models. CPU-capable, deterministic, low
VRAM. Scores 86.2 on OmniDocBench v1.5 — competitive with the previous-gen
VLM `MinerU2.0-2505-0.9B`.

## Models loaded

Constructed once per `(lang, formula_enable, table_enable)` tuple by
`ModelSingleton` in `pipeline_analyze.py`. The actual builder is
`MineruPipelineModel` in `model_init.py`.

| Stage             | Model class                                       | Notes                                              |
|-------------------|---------------------------------------------------|----------------------------------------------------|
| Layout            | `PPDocLayoutV2LayoutModel`                        | PaddlePaddle DocLayoutV2 (replaces older yolov8 / doclayoutyolo, dropped in 3.0 for AGPL). |
| MFR (math)        | `UnimernetModel` (default) or `FormulaRecognizer` (PP-FormulaNet+) | Toggle with env `MINERU_FORMULA_CH_SUPPORT=True`. Default model only handles English/Latin math. |
| OCR det+rec       | `PytorchPaddleOCR`                                | 109-language PaddleOCR ported to pure PyTorch. Det DB-thresh `0.3`, unclip ratio `1.8` by default. |
| Seal text         | `seal_crop` + `seal_det_warp` (in `model/ocr/`)   | Recognizes stamps / circular seals.                |
| Table cls         | `PaddleTableClsModel`                             | Classifies tables as wired/wireless to pick recognizer. |
| Table cls ori     | `MineruTableOrientationClsModel`                  | Detects 90/180/270° rotated tables.                |
| Table rec (wired) | `UnetTableModel`                                  | For ruled tables. Re-uses OCR engine internally.   |
| Table rec (wireless) | `PaddleTableModel` (SLANet+)                   | For unruled / borderless tables.                   |

## Per-document flow

1. `pdf_classify.classify(pdf_bytes)` decides OCR vs txt when method=auto.
2. `load_images_from_pdf_doc` (in `utils/pdf_image_tools.py`) renders pages
   with pypdfium2 at the configured DPI.
3. `batch_analyze.py` runs all model stages across a "processing window"
   of pages (default from `get_processing_window_size`). Sliding window so
   peak memory stays bounded — this is the optimisation 3.0 introduced for
   tens-of-thousands-of-pages docs.
4. `pipeline_magic_model.py` post-processes raw model output into
   `middle_json` blocks (the canonical intermediate representation — same
   schema as VLM backend, see [output.md](output.md)).
5. `para_split.py` groups detected text into paragraphs.
6. `pipeline_middle_json_mkcontent.py` renders middle JSON → markdown /
   content list.

## Long-document optimisations

Three things keep long PDFs tractable on commodity GPUs:

- **Sliding-window batch.** Pages are processed in groups; intermediate
  results are written out per group instead of all-at-end.
- **Streaming writes.** `batch_analyze.py` streams completed pages to disk
  via `pipeline_middle_json_mkcontent` rather than holding them all.
- **Thread-safety.** Singletons use `PIPELINE_MODEL_INIT_LOCK`; concurrent
  inference is supported, enabling one-process multi-thread serving (and
  enabling `mineru-router` to fan out across GPUs).

## When to pick this backend

- CPU-only deployments.
- 6 GB or smaller GPU — VLM doesn't fit, this does.
- Want zero hallucinations on text content.
- Documents with simple/no math — this backend's MFR is weaker than the VLM.
- Need 109-language OCR explicitly (the VLM backends are powerful but the
  pipeline gives you fine-grained PaddleOCR language packs).
