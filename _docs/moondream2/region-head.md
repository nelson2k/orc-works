# Region head — coordinates and sizes

The region head is the part of the model that produces and consumes **spatial values** (coordinates and sizes) directly from / into the text decoder's hidden states, without going through the word-token vocabulary.

This lets `detect` / `point` / `query(reasoning=True)` work with continuous-valued spatial outputs at high precision (1024 bins per dimension) while still living inside the same autoregressive decode loop as the text.

## Why a separate head

A plain LM head would have to spend many vocabulary entries on coordinate tokens (e.g. `<0.42>`, `<0.43>` …) and would force quantization into a coarse grid baked into the vocab. Instead:

- **Output side**: when the decoder emits a special `coord_id` or `size_id` token, the region head reads a 1024-bin distribution from the same hidden state and selects the index.
- **Input side**: at the next decoder step, the chosen coordinate is encoded back into a 2048-dim embedding via Fourier features + a linear, and fed in as if it were a normal token embedding. The LM never has to vocabulary-tokenize the number.

## Components

(Source: `region.py`. Config: `RegionConfig`.)

```python
RegionConfig:
    dim            = 2048   # hidden dim (matches text decoder)
    coord_feat_dim = 256    # cos+sin feature size for coords
    coord_out_dim  = 1024   # # of bins for coord decode
    size_feat_dim  = 512    # cos+sin feature size for sizes
    size_out_dim   = 2048   # 2 × 1024 (one 1024-bin head per axis)
    inner_dim      = 8192   # hidden dim of decoder MLPs
```

The runtime owns these weight tensors (built in `MoondreamModel.__init__`):

| Tensor | Shape | Role |
|---|---|---|
| `coord_features` | `(1, 128)` (transposed view of `(128, 1).T`) | Learned Fourier frequencies for 1-D coord input |
| `coord_encoder` | Linear 256 → 2048 | Project Fourier features into the decoder's embedding space |
| `coord_decoder.fc1` | Linear 2048 → 8192 | First half of coord-decode MLP |
| `coord_decoder.fc2` | Linear 8192 → 1024 | Output 1024 coord logits |
| `size_features` | `(2, 256)` (transposed) | Learned Fourier frequencies for 2-D size input |
| `size_encoder` | Linear 512 → 2048 | Project size Fourier features |
| `size_decoder.fc1` | Linear 2048 → 8192 | |
| `size_decoder.fc2` | Linear 8192 → 2048 | Output reshaped to `(2, 1024)` — width and height logits |

`QuantizedLinear` (4-bit weight quant via torchao) is used if `region.group_size` is set; otherwise plain `nn.Linear`. The shipped checkpoint typically loads as plain bf16.

## Fourier features

```python
def fourier_features(x, w):
    f = 2 * math.pi * x @ w
    return torch.cat([f.cos(), f.sin()], dim=-1)
```

For a single coordinate `x` (a scalar in `[0, 1]`):

1. Project `x` through `w` (a `(1, 128)` learned frequency vector) → `(128,)` frequencies.
2. Multiply by `2π` and take `cos` and `sin` independently.
3. Concatenate → a `(256,)` feature vector.

The intuition (from positional-encoding research): the linear layer that follows can represent high-frequency variations of `x` because the input is now a basis of sinusoids at many learned frequencies. A bare linear over the scalar `x` couldn't.

For sizes, `w` has shape `(2, 256)` so a `(w_norm, h_norm)` 2-vector becomes a `(512,)` feature.

## Encode coordinate

```python
encode_coordinate(coord, w)
# coord: (..., 1) — a normalized x or y
# → Fourier features (256-dim)
# → coord_encoder linear → (..., 2048) — an embedding to feed into the decoder
```

Used in two places:

- Inside `detect` / `point` generation, after decoding `x` (then `y`) from the previous hidden state, the chosen value is re-encoded so the next decoder step "sees" the coordinate it just decided on.
- Inside `query(spatial_refs=...)`, where user-supplied `(x, y)` references replace `coord_id` placeholder embeddings in the prompt.

## Decode coordinate

```python
decode_coordinate(hidden_state, w) -> (..., 1024)
# 2-layer MLP: 2048 → 8192 → 1024 logits
# argmax over 1024 bins, divide by 1024 → normalized coord in [0, 1)
```

The decode picks an argmax (`detect` / `point` are temperature=0). One coord per decoder step.

## Encode / decode size

The size decoder is identical in spirit but outputs `(2, 1024)` — one 1024-bin distribution for width and one for height. The bins are **log-scale**:

```
bin → size = 2^((bin / 1023) * 10 - 10)
size → bin = round((log2(size) + 10) / 10 * 1023)
```

Range: `size = 2^-10 ≈ 1/1024` (smallest representable) to `size = 2^0 = 1.0` (full image side). Log-scaling matters because objects in natural images span many orders of magnitude in size; a linear bin distribution would waste bins on near-1.0 sizes and starve the small end.

## Spatial-references encoder

`encode_spatial_refs(spatial_refs, region_module)`:

```python
spatial_refs = [
    (x, y),                        # a point reference
    (x_min, y_min, x_max, y_max),  # a region reference
    ...
]
```

For each ref:

- Point → append `x` and `y` to `coords`.
- Region → append `(x_center, y_center)` to `coords`, append `(width, height)` to `sizes`.

Then `coords` is encoded as a sequence of 1-D coordinates (`encode_coordinate` per scalar) and `sizes` as 2-D sizes (`encode_size` per pair). The output dict `{"coords": Tensor[..., 2048], "sizes": Tensor[..., 2048] | None}` is what `_prefill_prompt` splices into the prompt embedding stream at every `coord_id` / `size_id` placeholder position.

## Object generation loop

`_generate_points(hidden, next_token, pos, include_size=True | False, max_objects, lora)` is the loop that drives detection / pointing:

```
loop:
    # x coordinate
    x_logits = decode_coordinate(hidden, region)
    x        = argmax(x_logits) / 1024
    next_emb = encode_coordinate(x, region)

    # advance decoder one step
    pos += 1
    hidden = decoder(next_emb, mask, pos_ids)

    # y coordinate
    y_logits = decode_coordinate(hidden, region)
    y        = argmax(y_logits) / 1024
    next_emb = encode_coordinate(y, region)

    if include_size:
        pos += 1
        hidden = decoder(next_emb, mask, pos_ids)
        size_logits = decode_size(hidden, region)
        w = 2^((argmax(size_logits[0]) / 1023) * 10 - 10)
        h = 2^((argmax(size_logits[1]) / 1023) * 10 - 10)
        next_emb = encode_size((w, h), region)
        out.append({x_min: x - w/2, ..., y_max: y + h/2})
    else:
        out.append({x: x, y: y})

    # one more decoder step to read the "next object or EOS" token
    pos += 1
    logits, hidden = decoder(next_emb, mask, pos_ids)
    next_token = argmax(logits)

    if next_token == eos_id or len(out) >= max_objects: break
```

The decoder is doing two distinct jobs in this loop:

- Predicting the **next spatial value** through the region head (no token sampled).
- Predicting whether to **continue or stop** through the LM head (one token sampled per object).

It's still autoregressive — the only thing that's different from text generation is that the inputs and outputs around the spatial steps go through the region head instead of the embedding table and LM head.
