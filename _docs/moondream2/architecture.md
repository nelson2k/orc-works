# Architecture

moondream2 is a single bf16 model with three sub-networks that share one forward pass:

```
            +-------------------+
  image --->|  vision encoder   |---+
            +-------------------+   |
                                    v
            +-------------------+  +-------------------+
  text  --->|  text encoder     |->|  text decoder     |---> logits ---> token
  tokens    +-------------------+  |  (24 layers,      |
                                   |   prefix-attn)    |     +--> region head --> coord/size logits
                                   +-------------------+     |          ^
                                                  hidden ----+          |
                                                  state           Fourier features
```

The decoder mixes ordinary word-token embeddings with **special-spatial embeddings** produced by the region head — when generation emits a `<coord>` token, the embedding fed back at the next step is the Fourier-encoded coordinate, not the embedding-table row.

## Vision encoder

(Source: `vision.py`, `image_crops.py`. Config: `VisionConfig`.)

A SigLIP-style ViT:

| Property | Value |
|---|---|
| Hidden dim | 1152 |
| Layers | 27 |
| FFN dim | 4304 |
| Heads | 16 |
| Patch size | 14 px |
| Crop size | 378 px (= 27 patches per side, 729 patches per crop) |
| Input channels | 3 |
| Max crops | 12 |
| Patch embedding | linear over `14*14*3 = 588`-dim flat patches |
| Position emb | learned absolute, shape `(1, 729, 1152)` |
| Block | LN → MHA (qkv linear → split → SDPA) → LN → MLP (fc1 → GELU(tanh) → fc2) |

Pre-processing in `image_crops.overlap_crop_image`:

1. Compute an integer tile grid `(h_tiles, w_tiles)` such that `h_tiles * w_tiles ≤ max_crops` and tiles roughly match the image aspect ratio. If `h_tiles * w_tiles == 1`, only the global crop is used.
2. Resize the full image to `378×378` → the **global** crop.
3. For each tile, extract a `378×378` crop with `overlap_margin = 4` patches of overlap to its neighbors → the **local** crops.
4. Return `crops: (1+N, 378, 378, 3)` and `tiling: (h_tiles, w_tiles)`.

`vision_encoder` runs the ViT on the batched `1+N` crops. The global crop becomes the first row of features; the local crops become rows `1:`.

`vision_projection` then:

1. Reshapes local features into a grid using `tiling` (`reconstruct_from_crops`, in `image_crops.py`).
2. Adaptive-avg-pools the grid down to `27×27 = 729` positions (matches the global crop's patch count).
3. Concatenates the pooled local features with the global features along the channel dim → `(729, 2 * 1152) = (729, 2304)`.
4. Runs a 2-layer MLP (`fc1` 2304→8192, GELU, `fc2` 8192→2048) → final image embeddings `(729, 2048)`.

This gives 729 image-token embeddings that get prepended to the prompt token stream.

## Text decoder

(Source: `text.py`, `rope.py`, `layers.py`. Config: `TextConfig`.)

A Phi-style decoder:

| Property | Value |
|---|---|
| Embedding dim | 2048 |
| Layers | 24 |
| FFN dim | 8192 (≈ 4× hidden) |
| Heads | 32 |
| KV heads | 32 (GQA path exists; equal in this checkpoint) |
| Head dim | 64 |
| Vocab | 51 200 |
| Max context | 2048 |
| Activation | GELU (tanh approx) |
| Norm | LayerNorm (post-block style w/ residual + attn + mlp) |
| Position emb | RoPE on first 32 head dims (partial rotary, `theta=10000.0`) |
| Token embedding | `wte: (vocab, dim)` plain `nn.Parameter` (not `nn.Embedding`) |

Each block computes (logically):

```
ln    = LayerNorm(x)
attn  = MHA(ln)        # qkv linear → split → RoPE → SDPA → out proj
mlp   = MLP(ln)        # fc1 → GELU(tanh) → fc2
x     = x + attn + mlp
```

Note that attn and mlp share the **same** `ln` input — this is the "parallel" Phi style, not pre-norm sequential. The block then has a single residual that accumulates both contributions.

The QKV projection is fused: a single `qkv` linear of shape `(dim, n_heads*head_dim + 2 * n_kv_heads*head_dim)`, then `.split([q_dim, kv_dim, kv_dim])` after projecting.

### Prefix attention

`MoondreamModel.__init__` builds `attn_mask` as a lower-triangular `(max_context, max_context)` mask, then explicitly sets the top-left `730 × 730` block to all-True. The constant `730 = 1 (BOS) + 27*27 (image patches)` is exactly the position range covered by `[BOS] + image_embeddings`. Effect: **the image tokens see each other bi-directionally** (full attention), but later text tokens only see causal history. This makes the image side of the prefix behave like an encoder while still letting the decoder be causal.

### KV cache

`KVCache` (in `moondream.py`) is an `nn.Module` that holds two persistent buffers `k_cache`, `v_cache` of shape `(1, n_kv_heads, max_context, head_dim)`. Each block has its own cache. `update(pos_ids, k, v)` scatters the new K/V into the cache at the given positions and returns the full cache slices for SDPA.

After `encode_image`, a snapshot of the (per-block) KV cache is taken and returned in the `EncodedImage` dataclass. Subsequent `caption` / `query` / `detect` / `point` calls **reload** that snapshot into the block KV caches before resuming generation, avoiding re-running the vision encoder for the same image across multiple skills.

## Region head

(Source: `region.py`, `fourier_features.py`. Config: `RegionConfig`.)

Used during `detect`, `point`, and grounded `query(reasoning=True)`. Two coupled paths:

### Coordinate path

- `encode_coordinate(x)` — `x: (..., 1)` normalized → Fourier features (256-dim) → `coord_encoder: Linear(256, 2048)` → embedding ready to feed into the decoder.
- `decode_coordinate(h)` — `h: (..., 2048)` decoder hidden state → 2-layer MLP (`fc1: 2048→8192`, GELU, `fc2: 8192→1024`) → 1024 logits over discrete coordinate bins. Picked via `argmax`; bin index divided by 1024 gives a normalized coordinate.

### Size path

- `encode_size(s)` — `s: (..., 2)` (width, height) normalized → Fourier features (512-dim) → `size_encoder: Linear(512, 2048)` → embedding.
- `decode_size(h)` — same shape MLP as coord, but `fc2: 8192→2048` reshaped to `(2, 1024)` — two 1024-bin distributions, one per dimension.
- Bins are **log-scale**: `bin → size = 2^((bin/1023) * 10 - 10)`, covering ~1/1024 to 1.0 of the image.

### Fourier features

```python
def fourier_features(x, w):
    f = 2 * math.pi * x @ w     # x: (..., d_in)  w: (d_in, d_feat/2)
    return torch.cat([f.cos(), f.sin()], dim=-1)
```

`w` is a learned frequency matrix (`coord_features: (1, 128)` for coords, `size_features: (2, 256)` for sizes). The cos/sin pair produces a `d_feat`-dim feature that the linear encoder then projects into the decoder's embedding space.

This is the standard "Fourier feature mapping" trick from positional-encoding research — by passing continuous coordinates through sinusoids at learned frequencies, the linear layer above can represent high-frequency variations that a plain linear layer over scalar coords cannot.

## Putting it all together

A typical `query(image, "what color is the car?")` call:

1. **`encode_image(image)`** — vision encoder + projection produces 729 image embeddings; `_prefill` runs the text decoder over `[BOS] + 729 image_embeds` with the prefix-attention mask. The resulting KV caches (per block) are snapshotted into an `EncodedImage`.
2. **`load_encoded_image(image)`** — copies the snapshot back into the block KV caches.
3. Build prompt token list: `[query_prefix] + tokenizer.encode(question) + [query_suffix]` (plus an optional `thinking_id` if `reasoning=True`).
4. **`_prefill_prompt`** — run the decoder over the prompt tokens, get the first sampled token.
5. **`_generate_answer`** — loop: feed last token through the decoder, sample next, until EOS or `max_tokens`.
6. Return a string (or a streaming generator if `stream=True`).

For `detect(image, "face")`:

1. Encode + reload as above.
2. Prefill the detect prompt (`[detect_prefix] + " face" tokens + [detect_suffix]`) with `temperature=0`.
3. **`_generate_points`** — special loop: read x via `decode_coordinate` from the last hidden state, encode it back via `encode_coordinate`, feed the embedding to the decoder; read y similarly; read width and height via `decode_size`. Repeat until EOS or `max_objects`. Outputs `{x_min, y_min, x_max, y_max}` per object.

For `point(image, "...")` the same loop runs but with `include_size=False`, yielding `{x, y}` per match.
