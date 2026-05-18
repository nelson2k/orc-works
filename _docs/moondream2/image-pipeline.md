# Image pipeline

The vision side has two stages: an **overlap-crop** preprocessor that turns the input image into a small batch of 378×378 tiles, and a ViT encoder that emits feature tokens for the text decoder to attend over.

## Stage 1 — overlap crop

(Source: `image_crops.py`.)

The model wants a fixed `378×378` input. To preserve high-resolution detail on larger images, the preprocessor produces **1 global crop + N overlapping local crops**, with `N ≤ max_crops = 12`.

```
overlap_crop_image(np_image, overlap_margin=4, max_crops=12, base_size=(378,378), patch_size=14)
  ↓
{
  "crops":  np.ndarray of shape (1+N, 378, 378, 3) — uint8 RGB
  "tiling": (h_tiles, w_tiles)                     — int tile-grid shape
}
```

### Tiling decision (`select_tiling`)

Given image `(H, W)` and target crop side `S = 378`:

1. If `H ≤ S` or `W ≤ S`, return `(1, 1)` — only the global crop is used.
2. Otherwise compute the minimum-required tiles to cover the image at full resolution: `min_h = ceil(H / S)`, `min_w = ceil(W / S)`.
3. If `min_h * min_w > max_crops`, scale proportionally to fit the budget.
4. Otherwise pick `h_tiles = floor(sqrt(max_crops * H/W))`, `w_tiles = floor(sqrt(max_crops * W/H))` — the closest aspect-ratio-matching tile grid that still fits in `max_crops`.
5. Clamp to `(max(1, h_tiles), max(1, w_tiles))`.

For a 1920×1080 input (16:9), this produces a 3×4 grid (12 tiles, exactly the cap).

### Overlap

`overlap_margin = 4` (in patch units = 56 px) means each local crop's footprint into the source image overlaps its neighbors by 4 patches' worth on every shared edge. The overlap exists for two reasons:

- Smooth feature continuity across crop boundaries.
- Objects spanning two tiles still see themselves wholly in at least one crop.

After encoding, the overlapping regions are dropped during reconstruction (the central patches of each crop are kept; margin patches are discarded) — so the final reassembled grid is a clean non-overlapping tiling.

### Optional pyvips backend

The crop code prefers `pyvips` (libvips bindings) when available — it's faster for large images. If not installed (or the import fails for any reason) it falls back to plain PIL. The HF runtime requirements file lists `pyvips==2.2.3` + `pyvips-binary==8.16.0` to ship the backend.

## Stage 2 — vision encoder

(Source: `vision.py`.)

The crops are converted to a bf16 tensor on the model's device, normalized to `[-1, 1]` via `(x/255 - 0.5) / 0.5`, and permuted to `(B, C, H, W)`. `B = 1 + N`.

```python
all_crops, tiling = prepare_crops(image, config, device)
outputs = vision_encoder(all_crops, self.vision, config)
```

Inside `vision_encoder`:

1. `create_patches(x, 14)` reshapes the `(B, 3, 378, 378)` input into `(B, 729, 588)` — each of `27×27 = 729` positions carries a flat `14*14*3 = 588`-dim patch.
2. `patch_emb` projects `588 → 1152`, then add `pos_emb` (learned absolute, shape `(1, 729, 1152)`).
3. Run 27 transformer blocks (`LN → MHA(qkv → SDPA → proj) → LN → MLP(fc1 → GELU(tanh) → fc2)`, each with a residual).
4. Final `post_ln`.

Output shape: `(B, 729, 1152)` where row 0 is the global crop's features and rows `1..N` are the local crops' features.

## Stage 3 — projection

(Source: `MoondreamModel._run_vision_encoder` and `vision.vision_projection`.)

Now we need to combine the global features and the local features into a single `(729, 2048)` grid that the text decoder can attend over.

1. Take the local features `(N, 729, 1152)` and reshape into a 4-D grid using `tiling`: this becomes a `(grid_h, grid_w, 1152)` feature map where `grid_h` and `grid_w` depend on the tile count and the per-tile retained patch count (`reconstruct_from_crops` drops overlap margins).
2. `adaptive_avg_pool2d` the grid down to `(27, 27, 1152)` — same spatial resolution as the global crop's feature map.
3. Concatenate with the global crop's features along the channel dim → `(729, 2304)` where 2304 = 2 × 1152.
4. Apply a 2-layer projection MLP: `fc1: 2304 → 8192`, GELU(tanh), `fc2: 8192 → 2048`.

Result: `(729, 2048)` — exactly 729 token embeddings ready to be prepended to the text decoder's input.

> Note: a small quirk in `_run_vision_encoder` — it reshapes the local features using `enc_n_layers` twice (`-1, enc_n_layers, enc_n_layers, enc_dim`). The constant `enc_n_layers = 27` happens to coincide with the spatial side of the 27×27 patch grid in this config, so the code reads slightly oddly but is correct for this particular `crop_size / patch_size = 378/14 = 27`. If you ever change `crop_size`, this line needs to change too.

## Stage 4 — feeding the decoder

`encode_image` then:

1. Tokenizes a BOS into a single embedding via `text_encoder`.
2. Concatenates `[BOS_emb, image_embeds]` → `(1, 1 + 729, 2048) = (1, 730, 2048)`.
3. Runs `_prefill` (`text_decoder`) over that entire prefix with the **prefix-attention** mask: full bi-directional attention within the 730-token prefix, causal everywhere else.
4. Snapshots each block's K/V cache slice `[..., :730, :]` and packages it into an `EncodedImage(pos=730, caches=[(k, v) per block])`.

That `EncodedImage` is what the four skills consume. It's also reusable: keep it around if you plan to run multiple skills against the same image to skip the vision encoder cost on subsequent calls.

```python
enc = model.encode_image(img)
print(model.caption(enc, length="short")["caption"])
print(model.query(enc, "What is happening?")["answer"])
print(model.detect(enc, "person")["objects"])
```

## Performance notes

- Vision encoding dominates first-token latency for image-grounded skills; on RTX-class GPUs it's well under a second for typical inputs but can grow for very large images that hit the `max_crops=12` cap.
- The bf16 dtype is hard-coded for the vision encoder path (`prepare_crops` casts to `torch.bfloat16` explicitly). If the rest of the model is loaded in a different dtype the multiplication paths still work because `F.scaled_dot_product_attention` and the linears tolerate dtype mismatch on most accelerators, but it's cleanest to keep everything bf16.
- `torch.compile` accelerates the encoder when you call `model.compile()` on the underlying `MoondreamModel`. The `_vis_proj` step is currently *not* compiled (TODO in the source).
