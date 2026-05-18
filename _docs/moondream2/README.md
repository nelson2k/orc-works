# moondream2

A small (~1.9 B parameter) vision-language model published by `vikhyatk` and the team behind moondream.ai. License: Apache-2.0 (code; weights are also Apache-2.0 per the HF model card).

This particular repository (`repos-folder/moondream2`) is the **Hugging Face checkpoint snapshot** — model files, tokenizer, and a small Python package that's loaded via `trust_remote_code=True`. It's the v2 generation; a `moondream3-preview` is what the upstream points at as the successor.

## What the model does

One model handles four image-grounded tasks, exposed as Python methods on the loaded `AutoModelForCausalLM`:

- **`caption(image, length="short" | "normal" | "long")`** — generate a caption. Streaming supported.
- **`query(image, question, reasoning=False, spatial_refs=None, stream=False)`** — visual question answering. With `reasoning=True` the model emits a step-by-step grounded reasoning trace before answering. `spatial_refs` lets you point the question at specific (x,y) coordinates or (x_min,y_min,x_max,y_max) regions inside the image.
- **`detect(image, object)`** — open-vocabulary object detection. Returns a list of `{x_min, y_min, x_max, y_max}` boxes in normalized 0–1 coordinates.
- **`point(image, object)`** — open-vocabulary "pointing". Returns a list of `{x, y}` points in normalized 0–1 coordinates.

Plus a more experimental **`detect_gaze`** for gaze estimation given an eye point or face bounding box.

## How it's put together

Three sub-networks share a single forward pass:

1. **Vision encoder** — a ~400 M parameter ViT (SigLIP-style: 27 layers, 1152-dim, 16 heads, patch 14, 378×378 crops). It encodes the full image plus up to 12 overlapping crops, then reassembles a feature grid and projects to 2048-dim through a 2-layer MLP.
2. **Text decoder** — a 24-layer, 2048-dim, 32-head Phi-style decoder (8192 FFN, RoPE, GQA — though `n_kv_heads == n_heads == 32` here, so it's effectively MHA). Vocab 51200 with a 2048-token max context. The "prefix attention" trick: the first `1 + (378/14)^2 = 730` positions (BOS + image patches) all see each other bi-directionally; positions after 730 are causal.
3. **Region head** — small per-task projections that turn the decoder's hidden state into coordinate logits (1024 bins) and size logits (1024 log-scale bins, range `2^-10 … 2^0` of the image). Used during `detect` / `point` / grounded reasoning to read out spatial outputs without going through the text tokenizer.

A single decode loop alternates between **text tokens** and **special-spatial tokens** (`<coord>`, `<size>`). When the model emits a coord token, the next embedding fed back into the decoder comes from the region head (Fourier features → coord encoder) instead of the word embedding table.

## What's in the checkpoint

- `model.safetensors` — all weights in bf16 (the text decoder may be int4-packed via torchao when loaded with `group_size=128`; the code paths exist).
- `config.json` — minimal HF config pointing at the in-repo classes via `auto_map`.
- `configuration_moondream.py` — `PhiConfig` + `MoondreamConfig` (kept for HF compatibility; the actual runtime config is the dataclass set in `config.py`).
- `tokenizer.json` / `vocab.json` / `merges.txt` / `tokenizer_config.json` / `added_tokens.json` / `special_tokens_map.json` — the "superword" BPE tokenizer (51200 entries). The model fetches its tokenizer from `moondream/starmie-v1` on the Hub at init time.
- `versions.txt` — list of all dated releases (2024-03-04 through 2025-06-21). The HF `revision=` argument pins to a specific date.
- `requirements.txt` — runtime deps that aren't already implied by transformers (`einops`, `pyvips`, `pyvips-binary`).

## Loading

```python
from transformers import AutoModelForCausalLM
from PIL import Image

model = AutoModelForCausalLM.from_pretrained(
    "vikhyatk/moondream2",
    revision="2025-06-21",
    trust_remote_code=True,
    device_map={"": "cuda"},  # or "mps" on Apple Silicon
)

img = Image.open("photo.jpg")
print(model.caption(img, length="normal")["caption"])
print(model.query(img, "How many people are in this image?")["answer"])
for box in model.detect(img, "face")["objects"]:
    print(box)
for point in model.point(img, "person")["points"]:
    print(point)
```

## Notes

- `revision="..."` is **strongly recommended** in production — the model is updated frequently, and changes (new tokenizer, new template ids, new region head) can break callers that pin only the package name.
- Streaming works for `caption()` and `query()` via `stream=True` — the result `["caption"]` / `["answer"]` becomes a generator yielding string chunks (word-boundary aware, with special handling for CJK and multi-byte unicode).
- A `compile()` method on the underlying `MoondreamModel` enables `torch.compile`-based fast paths for `_vis_enc`, `_prefill`, and `_decode_one_tok` (gpt-fast-style). Vision projection isn't compiled yet.
- The default sampler is multinomial with `temperature=0.5`, `top_p=0.3`, `max_tokens=768`. `max_objects` for detection/pointing defaults to 50.
- Reasoning mode (`query(..., reasoning=True)`) trades latency for accuracy on tasks like chart median calculation and precise counting.

## Variants (LoRA adapters)

Many calls accept `settings={"variant": "<id>"}`. The `lora.py` module downloads the named adapter from `https://api.moondream.ai/v1/variants/<id>/download` (auth via `MOONDREAM_API_KEY` env var) and caches it under `~/.cache/huggingface/hub/md_variants/<variant>/<step>.pt`. The adapter is applied as a low-rank addition to `qkv`, `proj`, `fc1`, `fc2` weights in each text-decoder layer.
