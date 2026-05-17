# Inference

Top-level entry point is `InferenceManager` in [chandra/model/__init__.py](../../repos-folder/chandra/chandra/model/__init__.py).

```python
from chandra.model import InferenceManager
from chandra.model.schema import BatchInputItem
from PIL import Image

mgr = InferenceManager(method="vllm")   # or "hf"
batch = [BatchInputItem(image=Image.open("page.png"), prompt_type="ocr_layout")]
results = mgr.generate(batch, include_images=True, include_headers_footers=False)
```

Each result is a `BatchOutputItem` (`chandra/model/schema.py`) with `markdown`, `html`, `chunks`, `raw`, `page_box`, `token_count`, `images`, `error`.

## HuggingFace backend

[chandra/model/hf.py](../../repos-folder/chandra/chandra/model/hf.py).

- `load_model()` calls `AutoModelForImageTextToText.from_pretrained(settings.MODEL_CHECKPOINT, dtype=bfloat16, device_map=device_map)` plus `AutoProcessor.from_pretrained(...)`. Padding side set to `left` for batched generation.
- `settings.TORCH_DEVICE` overrides `device_map` to `{"": device}`. `settings.TORCH_ATTN` sets `attn_implementation` (e.g. `flash_attention_2`).
- `generate_hf(batch, model, …)` builds a chat-template payload per item, runs `model.generate(..., max_new_tokens=max_output_tokens, eos_token_id=[…, <|im_end|>])`, batch-decodes, returns one `GenerationResult` per item.
- The model emits `<|im_end|>` at turn boundaries; the code explicitly adds it to `eos_token_id` even though `generation_config` only has `<|endoftext|>`.

Requires `[hf]` extras (torch, transformers, accelerate). Raises a clear `ImportError` if missing.

## vLLM backend

[chandra/model/vllm.py](../../repos-folder/chandra/chandra/model/vllm.py). Uses the `openai` SDK against a vLLM OpenAI-compatible server.

- Images are PNG-encoded and base64-embedded into a `chat.completions.create` call with `image_url` parts.
- Concurrency: `ThreadPoolExecutor` with `max_workers = min(64, len(batch))` by default.
- Retry: `_should_retry` triggers on (a) `detect_repeat_token` matches on the output tail (`chandra/model/util.py`), (b) explicit API errors. Each retry bumps `temperature` by `0.2 * (retries+1)` capped at `0.8`, with `top_p=0.95`. `max_retries` defaults from `settings.MAX_VLLM_RETRIES` (6). API errors also sleep `2 * (retries+1)` seconds.
- Reads `VLLM_API_BASE`, `VLLM_API_KEY` (default `"EMPTY"`), `VLLM_MODEL_NAME`.

## Image scaling before inference

Both backends call `scale_to_fit` from [chandra/model/util.py](../../repos-folder/chandra/chandra/model/util.py) before sending. The image is resized to fit between `min_size=(1792,28)` and `max_size=(3072,2048)` pixels, in 28-pixel-grid blocks (the model's patch grid). Aspect ratio is preserved as closely as possible; the refinement loop steps down whichever axis distorts the aspect ratio less.

## Repeat-token detection

[chandra/model/util.py: detect_repeat_token](../../repos-folder/chandra/chandra/model/util.py). Scans suffix lengths 1..250 and counts how many times each candidate sequence repeats at the end of the output. Shorter sequences require more repeats (`base_max_repeats=4`, scaling factor 3.0). This is what flags degenerate VLM "the the the the…" loops and triggers a higher-temperature retry.
