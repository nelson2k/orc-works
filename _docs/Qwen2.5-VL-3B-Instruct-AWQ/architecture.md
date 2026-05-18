# Architecture

Qwen2.5-VL-3B is two sub-networks tied through a projection:

```
            +-------------------+
  image --->|  vision encoder   |--+   (bf16, NOT AWQ-quantized)
  / video   |  (ViT, dyn. res.) |  |
            +-------------------+  v
                                  +-------------------+
  text  -----------------------> |  language decoder | --> logits --> token
  tokens                         |  (Qwen2.5 LM,     |
                                 |   AWQ 4-bit)      |
                                 +-------------------+
```

A single forward pass: image patches are projected into the decoder's embedding space (2048-dim) and **spliced into the text token stream** at `<|image_pad|>` / `<|video_pad|>` positions, framed by `<|vision_start|>` / `<|vision_end|>` markers. The decoder then operates on a flat sequence of mixed image-patch and text embeddings.

## Language decoder

Values from `config.json`.

| Property | Value |
|---|---|
| `model_type` | `qwen2_5_vl` |
| `hidden_size` | 2048 |
| `intermediate_size` | 11008 |
| `num_hidden_layers` | 36 |
| `num_attention_heads` | 16 |
| `num_key_value_heads` | 2 (**GQA**, 8:1 ratio) |
| Head dim | 128 (= 2048 / 16) |
| `vocab_size` | 151 936 |
| `hidden_act` | `silu` (SwiGLU FFN) |
| `rms_norm_eps` | 1e-6 |
| `tie_word_embeddings` | **true** (input embed reused as LM head) |
| `max_position_embeddings` | 128 000 |
| `max_window_layers` | 70 |
| `sliding_window` | 32 768 |
| `use_sliding_window` | **false** (full attention) |
| Quantization | AWQ 4-bit, see [awq-quantization.md](awq-quantization.md) |

The eight-to-one GQA ratio (16 query heads, 2 KV heads) keeps the KV cache tiny — critical for 128K-context inference.

## Vision tower

From `config.json` → `vision_config`.

| Property | Value |
|---|---|
| `model_type` | `qwen2_5_vl` (vision config) |
| `hidden_size` | 1280 |
| `out_hidden_size` | 2048 (matches decoder) |
| `in_chans` | 3 |
| `spatial_patch_size` | 14 |
| `tokens_per_second` | 2 (video FPS sampling) |
| `torch_dtype` | bf16 |

Notable: it stays bf16 — see [awq-quantization.md](awq-quantization.md). For preprocessing details (patch merging, dynamic resolution) see [image-pipeline.md](image-pipeline.md).

The ViT here uses:

- **Window attention** in most layers (full attention only in a few "anchor" layers) — major speedup over Qwen2-VL.
- **SwiGLU + RMSNorm** — aligned with the Qwen2.5 LLM (was GELU+LN in Qwen2-VL).

## Multimodal RoPE (mRoPE)

`rope_scaling` in the config:

```json
{
  "mrope_section": [16, 24, 24],
  "rope_type": "default",
  "type": "default"
}
```

`rope_theta = 1_000_000`.

The head dim (128) is split into three rotary sections:

- **16 dims** for the temporal axis (frame index / time)
- **24 dims** for the height axis (row)
- **24 dims** for the width axis (column)
- (Remaining 64 dims left non-rotated.)

Each visual token gets a `(t, h, w)` triple position, and the rotary frequencies are applied along each axis independently. For text tokens, all three axes share the same linear position id, so the same rotary code falls back to standard 1D RoPE. The model card's "dynamic FPS + absolute time alignment" comes from how `t` is assigned during video sampling — `tokens_per_second = 2` means the temporal id advances at a fixed wall-clock rate regardless of how many frames the dynamic sampler picked.

## Token IDs to know

(From `config.json`.)

- `bos_token_id`: 151 643 (`<|endoftext|>`)
- `eos_token_id`: 151 645 (`<|im_end|>`)
- `vision_start_token_id`: 151 652
- `vision_end_token_id`: 151 653
- `vision_token_id`: 151 654 (`<|vision_pad|>`)
- `image_token_id`: 151 655 (`<|image_pad|>`)
- `video_token_id`: 151 656 (`<|video_pad|>`)

Image/video pad tokens are placeholders inserted by the processor; each one is replaced at forward time by an image-patch embedding from the vision tower.
