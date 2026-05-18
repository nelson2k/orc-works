# LoRA variants

Almost every skill accepts a `settings={"variant": "<id>"}` argument. When supplied, the runtime downloads a per-variant LoRA adapter from the moondream API and applies it on top of the base weights for that single call (no permanent mutation of the loaded model).

This is how moondream ships task-specific fine-tunes — instead of distributing N full checkpoints, they distribute one base model + a small LoRA per task, loaded on demand.

## Cache layout

(Source: `lora.py`.)

```python
def variant_cache_dir():
    if "HF_HUB_CACHE" in env:  return env["HF_HUB_CACHE"]/"md_variants"
    if "HF_HOME"      in env:  return env["HF_HOME"]/"hub"/"md_variants"
    return "~/.cache/huggingface/hub/md_variants"
```

So the LoRA bundles sit alongside the standard Hugging Face hub cache. Within `md_variants/`:

```
md_variants/
└── <variant>/
    ├── final.pt
    ├── 1000.pt          # only present if variant_id was "<variant>/1000"
    ├── 2000.pt
    └── ...
```

`variant_id` syntax: `"<variant>"` (uses `final.pt`) or `"<variant>/<step>"` (uses `<step>.pt`).

## Download

When the cache miss happens:

```python
GET https://api.moondream.ai/v1/variants/<variant_id>/download
Headers:
    User-Agent:       moondream-torch
    X-Moondream-Auth: <env MOONDREAM_API_KEY>  (if set)
```

The response body is streamed to disk (`shutil.copyfileobj`) under the cache path above. The endpoint base can be overridden with the `MOONDREAM_ENDPOINT` env var.

## State-dict format

The downloaded file is a torch state dict (loaded with `weights_only=True`). Keys come from the training code and use older naming conventions; the loader applies a small rewrite table to map them to the runtime's tensor names:

| Old prefix / part | New |
|---|---|
| `text_model.transformer.h` | `text.blocks` |
| `.mixer` | `.attn` |
| `.out_proj` | `.proj` |
| `.Wqkv` | `.qkv` |
| `.parametrizations.weight.0` | (stripped) |

After rewriting, keys are dot-paths like `text.blocks.0.attn.qkv.A`, `text.blocks.0.mlp.fc1.B`, etc. `A` and `B` are the two LoRA factor matrices (low-rank decomposition: `ΔW ≈ B @ A`).

`nest(flat)` then converts the flat dict into a nested tree keyed by path components:

```python
{
  "text": {
    "blocks": {
      "0": {
        "attn": {"qkv": {"A": tensor, "B": tensor},
                 "proj": {"A": tensor, "B": tensor}},
        "mlp":  {"fc1": {"A": tensor, "B": tensor},
                 "fc2": {"A": tensor, "B": tensor}},
      },
      "1": { ... },
      ...
    }
  }
}
```

## Cache decorator

```python
@functools.lru_cache(maxsize=5)
def variant_state_dict(variant_id, device): ...
```

Up to 5 distinct variants are kept in-memory once loaded — switching back and forth between a few task adapters is cheap after the first warm-up.

## Application points

LoRA is applied as an **additive low-rank correction** inside each transformer block. The base weight `W` stays untouched; the linear becomes:

```
y = W @ x + B @ A @ x
```

There's no scale parameter in the application code — the trained `B` already absorbs scale.

Specifically, the LoRA is wired in at four sites per text-decoder block:

- `attn.qkv` — the fused QKV projection.
- `attn.proj` — the attention output projection.
- `mlp.fc1` — first FFN linear.
- `mlp.fc2` — second FFN linear.

`text.attn` and `text.mlp` both look for `lora["qkv"]["A"]`, `lora["proj"]["A"]`, `lora["fc1"]["A"]`, `lora["fc2"]["A"]` (and the matching `B`), and if present, add the `F.linear(F.linear(x, A), B)` correction term to the base linear's output.

LoRA is **not** applied to the vision encoder or the region head. Only the text decoder layers.

## Per-call flow

```python
model.detect(img, "face", settings={"variant": "object-detection-v2"})
```

1. `MoondreamModel.detect` reads `settings["variant"]`, calls `variant_state_dict("object-detection-v2", device=self.device)`.
2. If the variant isn't on disk, `cached_variant_path` downloads it.
3. The state dict is loaded with `torch.load(map_location=device)`, key-rewritten, and nested.
4. The resulting tree is threaded through `_prefill_prompt` → `text_decoder` → per-block `attn` / `mlp`, which apply the additive LoRA term.
5. Other concurrent calls without `settings["variant"]` see the base model unchanged.

## Authorization

If `MOONDREAM_API_KEY` is set in the environment, it's sent as `X-Moondream-Auth`. Public variants may not require auth; gated / paid variants do.

## Practical notes

- Variants used to be relatively small (a few MB) since they only cover four matrices per layer × 24 layers, often with rank ≤ 16.
- The `lru_cache(maxsize=5)` means the loaded variants share GPU memory with the base model and each other — they're tiny so this is rarely a problem.
- LoRA adapters are **per-revision**: a variant trained against the `2025-04-15` checkpoint may not work cleanly against `2025-06-21` if the tokenizer or layer dims changed. Pin both: `revision=...` for the base model, and pick variants released against that same revision.
