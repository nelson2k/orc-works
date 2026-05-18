# Image pipeline

From `preprocessor_config.json`:

```json
{
  "image_processor_type": "Qwen2VLImageProcessor",
  "processor_class": "Qwen2_5_VLProcessor",
  "patch_size": 14,
  "merge_size": 2,
  "temporal_patch_size": 2,
  "min_pixels": 3136,
  "max_pixels": 12845056,
  "size": {"shortest_edge": 3136, "longest_edge": 12845056},
  "resample": 3,
  "do_resize": true,
  "do_rescale": true,
  "rescale_factor": 0.00392156862745098,
  "do_normalize": true,
  "do_convert_rgb": true,
  "image_mean": [0.48145466, 0.4578275, 0.40821073],
  "image_std":  [0.26862954, 0.26130258, 0.27577711]
}
```

## Dynamic resolution

Unlike fixed-crop ViTs, the Qwen2-VL/2.5-VL image processor **resizes each image to a multiple of 28×28** preserving aspect ratio, then patches at 14 px and merges 2×2 patches. The "28" comes from `patch_size * merge_size = 14 * 2`.

A given image with `H × W` pixels (after resize) becomes:

```
n_image_tokens = (H / 28) * (W / 28)
```

i.e. one decoder token per 28×28 area.

## Bounds on size

- `min_pixels = 3136 = 56 × 56 = 4 × 28²` — the smallest accepted area, i.e. 4 image tokens minimum.
- `max_pixels = 12 845 056 = 16384 × 28²` — the largest accepted area, i.e. 16 384 image tokens maximum.

The resize step picks the largest `H × W` that fits in `[min_pixels, max_pixels]` while keeping aspect ratio and rounding each dim to a multiple of 28. So a 4032 × 3024 phone photo at default settings becomes roughly 4032 × 3024 directly (≈12.2 M pixels, within the cap), tokenized to ~15 552 image tokens.

Pass explicit bounds to the processor to clamp tighter:

```python
processor = AutoProcessor.from_pretrained(
    "Qwen/Qwen2.5-VL-3B-Instruct-AWQ",
    min_pixels=256 * 28 * 28,    # ≈200 K pixels → ≥256 tokens
    max_pixels=1280 * 28 * 28,   # ≈1 M pixels → ≤1 280 tokens
)
```

Or per-message:

```python
{"type": "image", "image": "...", "min_pixels": 50176, "max_pixels": 50176}
# or
{"type": "image", "image": "...", "resized_height": 280, "resized_width": 420}
```

`resized_height` / `resized_width` are rounded to the nearest multiple of 28.

## Normalization constants

The `image_mean` / `image_std` values are the **CLIP / SigLIP statistics**, not the ImageNet ones:

```
mean = (0.48145466, 0.4578275,  0.40821073)
std  = (0.26862954, 0.26130258, 0.27577711)
```

If you ever preprocess outside the HF processor (e.g. ONNX export), use these — don't substitute ImageNet `(0.485, 0.456, 0.406) / (0.229, 0.224, 0.225)`.

Pipeline order (per HF flags):

1. `do_convert_rgb`: alpha or grayscale → RGB.
2. `do_resize` with `resample=3` (PIL's `BICUBIC`).
3. `do_rescale`: `pixel / 255` (`rescale_factor = 1/255`).
4. `do_normalize`: `(pixel - mean) / std`.
5. Tile into 14×14 patches; merge 2×2 → one token per 28×28.

## Videos

`temporal_patch_size: 2` — videos are sampled into frame pairs, each pair becoming one temporal patch. Combined with `tokens_per_second = 2` from the vision config (see [architecture.md](architecture.md#multimodal-rope-mrope)), the model's mRoPE temporal axis advances 2 ids per real-world second regardless of source FPS.

For longer videos than ~1 h, increase `max_pixels` carefully — token count grows linearly with frame count.
