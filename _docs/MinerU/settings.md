# Settings

MinerU has two layers of config:

1. **`mineru.json`** — JSON file at `~/mineru.json` (or path in
   `MINERU_TOOLS_CONFIG_JSON` env var). Read by `mineru/utils/config_reader.py`.
2. **Environment variables** — runtime overrides + flags that don't fit
   in JSON. Lots of them; the important ones below.

## `mineru.json` template

Bundled as `mineru.template.json` in the repo root:

```json
{
    "bucket_info": {
        "bucket-name-1": ["ak", "sk", "endpoint"],
        "bucket-name-2": ["ak", "sk", "endpoint"]
    },
    "latex-delimiter-config": {
        "display": {"left": "$$", "right": "$$"},
        "inline":  {"left": "$",  "right": "$"}
    },
    "llm-aided-config": {
        "title_aided": {
            "api_key": "your_api_key",
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

| Key                         | Purpose                                                                       |
|-----------------------------|-------------------------------------------------------------------------------|
| `bucket_info`               | S3 (or S3-compatible) credentials per bucket name, used by `data/io/s3.py`.  |
| `latex-delimiter-config`    | Choose how math is delimited in markdown output (`$..$` / `\(..\)` etc.).    |
| `llm-aided-config.title_aided` | Optional LLM-aided title cleanup (sends section headers to e.g. Qwen for refinement). Disabled by default. |
| `models-dir`                | Override the model cache directory per backend.                              |

## Env vars

| Var                                  | Default              | Notes                                                                         |
|--------------------------------------|----------------------|-------------------------------------------------------------------------------|
| `MINERU_TOOLS_CONFIG_JSON`           | `mineru.json`        | Override the config file path. Absolute path bypasses the home-dir lookup.    |
| `MINERU_LOG_LEVEL`                   | `INFO`               | Loguru level for the CLI.                                                     |
| `MINERU_DEVICE_MODE`                 | auto (`cuda`/`mps`/`cpu`) | Force a torch device for pipeline / VLM backends.                       |
| `MINERU_FORMULA_CH_SUPPORT`          | `False`              | If `True`, use `pp_formulanet_plus_m` (better Chinese math support) instead of `unimernet_small`. |
| `MINERU_MAX_CONCURRENT_REQUESTS`     | server default       | Per-`mineru-api` concurrency cap.                                             |
| `MINERU_LMDEPLOY_DEVICE`             | unset                | If set to `maca`, disables `cudnn` for compatibility.                         |
| `TORCH_CUDNN_V8_API_DISABLED`        | `1` (set by mineru)  | Cudnn v8 compatibility shim, set automatically.                               |
| `PYTORCH_ENABLE_MPS_FALLBACK`        | `1` (set by mineru)  | Allows MPS to fall back to CPU for unsupported ops.                           |
| `NO_ALBUMENTATIONS_UPDATE`           | `1` (set by mineru)  | Prevents `albumentations` from phoning home.                                  |
| `TOKENIZERS_PARALLELISM`             | `false` (set)        | Quietens HuggingFace tokenizer warning.                                       |

## Devices

`mineru/utils/config_reader.py:get_device()` returns:

1. `MINERU_DEVICE_MODE` if set.
2. `cuda` if `torch.cuda.is_available()`.
3. `npu` if `torch_npu` is importable.
4. `mps` on Apple Silicon.
5. `cpu` otherwise.

## Model caches

Downloads land in:
- HuggingFace: `~/.cache/huggingface/hub/`
- ModelScope: `~/.cache/modelscope/hub/`

Use `models-dir` in `mineru.json` to relocate. Use `mineru-models-download`
to pre-fetch everything in one shot.
