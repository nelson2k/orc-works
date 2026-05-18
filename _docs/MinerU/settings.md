# Configuration

Two layers:

1. **`~/mineru.json`** — user-level static config (model paths, S3 buckets, LaTeX delimiters, LLM-aided post-processing). Path override: `MINERU_TOOLS_CONFIG_JSON`.
2. **Environment variables** — runtime flags consulted by [mineru/utils/config_reader.py](../../repos-folder/MinerU/mineru/utils/config_reader.py), the CLIs, and the backends.

## `~/mineru.json` schema

Template at [mineru.template.json](../../repos-folder/MinerU/mineru.template.json):

```json
{
  "bucket_info": {
    "bucket-name-1": ["access_key", "secret_key", "endpoint"],
    "bucket-name-2": ["access_key", "secret_key", "endpoint"]
  },
  "latex-delimiter-config": {
    "display": { "left": "$$", "right": "$$" },
    "inline": { "left": "$",  "right": "$"  }
  },
  "llm-aided-config": {
    "title_aided": {
      "api_key": "...",
      "base_url": "https://dashscope.aliyuncs.com/compatible-mode/v1",
      "model": "qwen3.5-plus",
      "enable_thinking": false,
      "enable": false
    }
  },
  "models-dir": {
    "pipeline": "",
    "vlm": ""
  },
  "config_version": "1.3.1"
}
```

- `bucket_info` — keyed by bucket name; the special key `[default]` is used as a fallback for unknown buckets. Used by S3 `DataReader`/`DataWriter` (see `mineru/data/data_reader_writer/s3.py`).
- `latex-delimiter-config` — wraps math expressions in Markdown output.
- `llm-aided-config.title_aided` — when `enable: true`, the renderer rewrites headings using the given OpenAI-compatible model.
- `models-dir` — explicit path overrides; usually populated by `mineru-models-download`.

## Environment variables

### Device / engines

| Var | Default | Purpose |
|---|---|---|
| `MINERU_DEVICE_MODE` | auto-detect | Force device: `cuda`, `mps`, `npu`, `gcu`, `musa`, `mlu`, `sdaa`, `cpu` |
| `MINERU_VLLM_DEVICE` | auto | Override vLLM device |
| `MINERU_LMDEPLOY_DEVICE` | auto | Override LMDeploy device (`maca` is special-cased: disables cuDNN) |
| `MINERU_LMDEPLOY_BACKEND` | auto | LMDeploy backend name |

### Feature toggles

| Var | Default | Purpose |
|---|---|---|
| `MINERU_FORMULA_ENABLE` | true | Override `--formula` CLI flag |
| `MINERU_TABLE_ENABLE` | true | Override `--table` CLI flag |
| `MINERU_FORMULA_CH_SUPPORT` | false | Enable Chinese formula support in pipeline model init |
| `MINERU_VLM_FORMULA_ENABLE` | true | VLM-only formula toggle |
| `MINERU_VLM_TABLE_ENABLE` | true | VLM-only table toggle |
| `MINERU_OCR_DET_MASK_INLINE_FORMULA_ENABLE` | true | Mask inline-formula bboxes during OCR det |
| `MINERU_HYBRID_BATCH_RATIO` | tuned | Pipeline:VLM batch ratio inside hybrid |
| `MINERU_FORCE_VLM_OCR_ENABLE` | false | Force VLM to do OCR in hybrid path |
| `MINERU_HYBRID_FORCE_PIPELINE_ENABLE` | false | Force pipeline path in hybrid |

### API server

| Var | Default | Purpose |
|---|---|---|
| `MINERU_API_MAX_CONCURRENT_REQUESTS` | 3 | Max in-flight tasks |
| `MINERU_PROCESSING_WINDOW_SIZE` | 64 | Pages per pipeline batch |
| `MINERU_LOG_LEVEL` | `INFO` | Server / client logging |
| `MINERU_API_PUBLIC_BIND_EXPOSED` | unset | Allow `*-http-client` backends when bound publicly |
| `MINERU_API_ALLOW_PUBLIC_HTTP_CLIENT` | unset | Same, finer-grained variant |
| `MINERU_API_SHUTDOWN_ON_STDIN_EOF` | unset | Treat stdin EOF as a shutdown signal (used by the embedded local API) |
| `MINERU_API_ENABLE_FASTAPI_DOCS` | unset | Expose `/docs` and `/openapi.json` |

### Misc

| Var | Default | Purpose |
|---|---|---|
| `MINERU_TOOLS_CONFIG_JSON` | `~/mineru.json` | Override config-file path |
| `MINERU_DONOT_CLEAN_MEM` | unset | Skip `torch.cuda.empty_cache()` between batches |
| `MINERU_PDF_RENDER_TIMEOUT` | none | Per-page render timeout (in `os_env_config.py`) |
| `MINERU_PDF_RENDER_THREADS` | auto | Render-pool size |
| `MINERU_SEAL_OCR_DEBUG` | false | Save seal-OCR debug crops |
| `MINERU_SEAL_OCR_DEBUG_DIR` | none | Output dir for the above |

### External pinned vars

- `TORCH_CUDNN_V8_API_DISABLED=1` — forced inside `cli/client.py` and `cli/fast_api.py` for compatibility.
- `PYTORCH_ENABLE_MPS_FALLBACK=1` — forced inside `backend/pipeline/pipeline_analyze.py`.
- `NO_ALBUMENTATIONS_UPDATE=1` — same place; prevents update check noise.
- `TOKENIZERS_PARALLELISM=false` — set in `cli/common.py`.

## Config-reader API

[config_reader.py](../../repos-folder/MinerU/mineru/utils/config_reader.py) exposes:

```python
read_config()                       # returns parsed mineru.json or None
get_s3_config(bucket)               # (ak, sk, endpoint) tuple
get_device()                        # current device, env-overridable
get_formula_enable(bool)            # CLI value overridden by MINERU_FORMULA_ENABLE
get_table_enable(bool)
get_ocr_det_mask_inline_formula_enable(bool)
get_processing_window_size(default)
get_max_concurrent_requests(default)
get_latex_delimiter_config()
get_llm_aided_config()
get_local_models_dir()
```
