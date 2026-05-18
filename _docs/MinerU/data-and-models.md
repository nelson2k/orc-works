# Data layer and models

## Data readers and writers

[mineru/data/data_reader_writer/](../../repos-folder/MinerU/mineru/data/data_reader_writer/)

```python
from mineru.data.data_reader_writer import (
    DataReader, DataWriter,
    FileBasedDataReader, FileBasedDataWriter,
    S3DataReader, S3DataWriter,
    MultiBucketS3DataReader, MultiBucketS3DataWriter,
    DummyDataWriter,
)
```

| Class | Reads / writes from |
|---|---|
| `FileBasedDataReader/Writer` | Local filesystem |
| `S3DataReader/Writer` | A single S3 bucket (creds + endpoint from `~/mineru.json`) |
| `MultiBucketS3DataReader/Writer` | Cross-bucket transfers, resolved from `bucket_info` map |
| `DummyDataWriter` | No-op sink (used for dry runs / tests) |

[mineru/data/io/](../../repos-folder/MinerU/mineru/data/io/) hosts lower-level transport classes — `http.py` (range-aware HTTP fetch), `s3.py` — used by the readers/writers.

## Models

### Pipeline models

[mineru/model/](../../repos-folder/MinerU/mineru/model/) holds the model implementations used by the pipeline backend.

- **layout/** — `pp_doclayoutv2.py`: PP-DocLayout v2. Detects block types per page (title, paragraph, table, figure, formula, ...).
- **ocr/** — `pytorch_paddle.py`: PaddleOCR ports running in pure PyTorch (det + rec). `seal_crop.py` / `seal_det_warp.py` handle round seals (Chinese documents).
- **mfr/** — math formula recognition.
  - `unimernet/` — UnimerNet
  - `pp_formulanet_plus_m/` — FormulaNet+
- **table/** — table cls (`cls/`) decides table type; rec (`rec/`) emits HTML.
- **utils/pytorchocr/** — PaddleOCR-derived modeling code (backbones, heads, postprocess) reimplemented for PyTorch.
- **utils/tools/** — checkpoint conversion utilities.

`ModelSingleton` in `backend/pipeline/pipeline_analyze.py` caches a built `MineruPipelineModel(lang, formula_enable, table_enable)` per language so subsequent calls reuse weights.

### VLM models

[mineru/model/vlm/](../../repos-folder/MinerU/mineru/model/vlm/)

- `vllm_server.py` — wraps `vllm.LLM` and exposes an OpenAI-style endpoint.
- `lmdeploy_server.py` — same for LMDeploy.

The VLM model file itself (`MinerU2.5-Pro-2604-1.2B`) is downloaded via `mineru-models-download` from HuggingFace or ModelScope. Cache path stored in `~/mineru.json` → `models-dir.vlm`.

## Utilities

[mineru/utils/](../../repos-folder/MinerU/mineru/utils/) is the grab-bag layer:

| Module | Purpose |
|---|---|
| `bbox_utils.py` / `boxbase.py` | Geometry on bounding boxes (intersect, IoU, expand, ...) |
| `char_utils.py` | CJK width handling, special-character normalization |
| `check_sys_env.py` | Detect macOS, Windows, CUDA presence |
| `cli_parser.py` | Shared CLI arg-parse helpers |
| `config_reader.py` | Read `~/mineru.json`, env vars (see settings.md) |
| `cut_image.py` | Crop helpers used by table/figure extraction |
| `draw_bbox.py` | Renders layout/span debug PDFs |
| `engine_utils.py` | Picks VLM engine (vllm / lmdeploy / mlx / transformers / http) |
| `enum_class.py` | Enums (`MakeMode`, `ImageType`, ...) |
| `guess_suffix_or_lang.py` | File-type sniffing (uses `magika`) and language sniffing (`fast-langdetect`) |
| `hash_utils.py` | Stable hashing for dedupe |
| `language.py` | Language-name normalization |
| `llm_aided.py` | LLM-aided post-processing (heading rewrite) |
| `magic_model_utils.py` | Shared "magic model" plumbing (the post-model normalization layer) |
| `model_utils.py` | VRAM measurement, `clean_memory`, etc. |
| `models_download_utils.py` | HuggingFace / ModelScope download helpers |
| `ocr_utils.py` | OCR-specific helpers (sort spans, merge lines, ...) |
| `office_rich_text.py` | Office rich-text normalization |
| `os_env_config.py` | Reads `MINERU_PDF_RENDER_*` env vars |
| `pdf_classify.py` | Classify a PDF as text-based vs scanned |
| `pdf_image_tools.py` | Rasterize PDF → PIL Images via pdfium |
| `pdf_page_id.py` | Compute effective page ranges |
| `pdf_reader.py` | High-level PDF iteration |
| `pdf_text_tool.py` | Pulls native text positions from PDFs (anchor for hybrid backend) |
| `pdfium_guard.py` | Thread-safe `pdfium` wrapping (open/close/page-count) |
| `span_block_fix.py`, `span_pre_proc.py` | Post-OCR cleanup |
| `table_continuation.py`, `table_merge.py` | Multi-page table stitching |
| `visual_magic_model_utils.py` | Visual post-processing of model outputs |
