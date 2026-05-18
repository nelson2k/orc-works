# Long context

The model is shipped configured for **32 768-token effective context** and can be extended to **128 K** with YaRN.

## Config values that matter

From `config.json`:

```json
"max_position_embeddings": 128000,
"sliding_window": 32768,
"use_sliding_window": false,
"max_window_layers": 70,
"rope_theta": 1000000.0,
"rope_scaling": {
  "mrope_section": [16, 24, 24],
  "rope_type": "default",
  "type": "default"
}
```

And from `tokenizer_config.json`: `model_max_length: 131072`.

Two seeming inconsistencies that are intentional:

- `max_position_embeddings = 128 000` but the upstream README says "32 768 by default" — because `rope_scaling.type` is `default` (no scaling applied), the model has only been trained / aligned to behave reliably out to 32 K, even though positional ids up to 128 K are technically valid.
- `sliding_window = 32 768` with `use_sliding_window = false` — the parameter is there in case you want to **opt into** local-window attention for very long contexts, but full attention is on by default.

## Extending to 128 K with YaRN

Per the upstream README, patch `config.json` like this **after download**:

```json
{
  "...": "...",
  "type": "yarn",
  "mrope_section": [16, 24, 24],
  "factor": 4,
  "original_max_position_embeddings": 32768
}
```

`factor: 4` means stretch the rope frequencies by 4× to cover `4 × 32 768 = 131 072` positions, with YaRN's frequency-aware interpolation that preserves short-range fidelity.

### When NOT to enable YaRN

The upstream README explicitly warns:

> However, it should be noted that this method has a significant impact on the performance of temporal and spatial localization tasks, and is therefore not recommended for use.

YaRN scaling rotates the RoPE frequency grid, which the multimodal RoPE (mRoPE) sections rely on for **spatial** localization (the `[16, 24, 24]` axis split — see [architecture.md](architecture.md#multimodal-rope-mrope)). After YaRN, bounding-box and pointing accuracy degrade noticeably. If your workload needs precise localization, stay at 32 K or shorten inputs.

## Long video alternative

For **long video** workloads specifically, the upstream README recommends a different approach:

> for long video inputs, since MRoPE itself is more economical with ids, the max_position_embeddings can be directly modified to a larger value, such as 64k.

Because mRoPE assigns positions on a `(t, h, w)` grid rather than a flat 1D axis, a 1-hour video doesn't consume 1-hour-of-tokens worth of position ids. Simply bumping `max_position_embeddings` (without YaRN, without rescaling RoPE) often suffices and keeps spatial precision intact.

## Memory implications

GQA helps a lot here: `num_key_value_heads = 2` with `head_dim = 128` means the KV cache per token is `2 * 128 * 2 layers_of_kv_per_head` × `num_hidden_layers = 36` × bf16 (2 bytes):

```
kv_bytes_per_token = 2 (kv heads) * 128 (head dim) * 2 (k+v) * 36 (layers) * 2 (bf16)
                  = 36 864 bytes ≈ 36 KB per token
```

At 32 K tokens that's ~1.2 GB of KV cache; at 128 K, ~4.5 GB. Plan VRAM accordingly — on top of the ~3.4 GB of weights and the vision-tower activations.
