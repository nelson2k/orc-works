# Package layout

The repo is a single flat folder — a Hugging Face model checkpoint plus the Python modules that implement the runtime. Everything is loaded via `trust_remote_code=True` from a single `model_dir`.

```
moondream2/
├── README.md                       Upstream HF model card (released change-log)
├── config.json                     HF config: model_type=moondream1, auto_map → hf_moondream classes
├── configuration_moondream.py      Legacy PretrainedConfig (PhiConfig + MoondreamConfig)
├── generation_config.json          Minimal HF generation config (not really used at runtime)
├── special_tokens_map.json         BOS/EOS/PAD ids for the tokenizer
├── tokenizer.json                  HF tokenizers serialization (BPE)
├── tokenizer_config.json           tokenizer.json companion
├── vocab.json / merges.txt         BPE pieces + merge rules (51 200 entries)
├── added_tokens.json               Extra tokens defined on top of the BPE
├── versions.txt                    List of dated revisions (pin with revision=...)
├── requirements.txt                Extra runtime deps: einops, pyvips, pyvips-binary
├── model.safetensors               Single-file weight bundle (bf16)
│
├── handler.py                      Inference Endpoints handler: __call__(data)→answer
├── hf_moondream.py                 HF wrapper: HfConfig + HfMoondream(PreTrainedModel)
│
├── config.py                       Frozen dataclasses: TextConfig, VisionConfig, RegionConfig,
│                                   TokenizerConfig, MoondreamConfig (runtime source of truth)
├── moondream.py                    MoondreamModel: top-level skills (caption, query, detect,
│                                   point, detect_gaze) + KVCache + generation loops
│
├── vision.py                       Vision-encoder forward + builder
├── vision_encoder.py               Pure-tensor variant of the vision encoder (older API)
├── image_crops.py                  overlap_crop_image + select_tiling + reconstruct_from_crops
│                                   (uses pyvips when available, falls back to PIL)
│
├── text.py                         Text-decoder forward + builder; attn / lm_head / encoder
│
├── region.py                       Coord/size Fourier encoders + decoders, encode_spatial_refs
├── region_model.py                 Legacy region-head wrapper (older API)
├── fourier_features.py             Fourier-feature helper module
│
├── layers.py                       attn / mlp / layer_norm + QuantizedLinear (4-bit weights
│                                   unpacked via torchao at load time)
├── lora.py                         variant_state_dict: downloads + caches LoRA variants from
│                                   api.moondream.ai, applied per-layer
├── rope.py                         precompute_freqs_cis + apply_rotary_emb (partial-rotary,
│                                   rot_dim=32)
├── utils.py                        Misc helpers (remove_outlier_points used by detect_gaze)
└── weights.py                      safetensors loader + dataclass wrappers (legacy entry path)
```

## Roles at a glance

- **HF entry path** — `hf_moondream.HfMoondream(config)` → instantiates `moondream.MoondreamModel(MoondreamConfig.from_dict(config.config))`. The `HfMoondream` class exposes thin properties (`caption`, `query`, `detect`, `point`, `detect_gaze`) that forward to `self.model.<skill>`, plus an `_setup_caches()` lazy hook so the KV cache is only allocated once you actually use the model.
- **Runtime model** — `MoondreamModel` owns the three sub-networks (`self.vision`, `self.text`, `self.region`) as `nn.ModuleDict`s. It builds the prefix-attention mask once (`attn_mask`) and stores it as a non-persistent buffer.
- **Vision pipeline** — `image_crops.overlap_crop_image` slices the source image into a 1+N tile set (1 global crop + up to 12 overlapping local crops), each 378×378. `vision.vision_encoder` runs the ViT on the batched crops; `vision.vision_projection` reassembles the local crops into a feature grid via adaptive-avg-pool and concatenates with the global features before projecting to 2048-dim.
- **Text decoder** — `text.text_decoder` runs the 24-layer Phi-style block stack with per-block KV caches (`KVCache` from `moondream.py`). `text.attn` uses `F.scaled_dot_product_attention` with the prefix-attention mask, RoPE on the first 32 head dims (`rope.apply_rotary_emb`), and GQA support (currently `n_heads == n_kv_heads`).
- **Region head** — `region.py` implements Fourier-feature encoders for normalized `(x,y)` coordinates and `(w,h)` sizes, plus matching decoders that go from the decoder hidden state to coord/size logits. The size head outputs **two** 1024-bin distributions (one per dimension), with bins distributed log-scale via `size = 2^((bin/1023) * 10 - 10)` so the range covers ~1/1024 to 1.0.
- **Sampling** — multinomial with top-p (`_apply_top_p`), or argmax when `temperature=0`. Sampler logic is inline inside `_prefill_prompt`, `_generate_answer`, `_generate_reasoning`, and `_generate_points`.

## Legacy / alternative paths

A few files exist for historical reasons:

- `weights.py` — older dataclass-based safetensors loader that builds a separate `MoondreamModel` (different from the one in `moondream.py`!). The HF flow does **not** use this; it's kept for direct-checkpoint loading outside transformers.
- `region_model.py` — earlier region-head module wrapper.
- `vision_encoder.py` — earlier pure-function vision encoder. `vision.py` replaces it.
- `configuration_moondream.py` — older `PretrainedConfig` shape (with the explicit `PhiConfig`). The actual runtime config now flows through `MoondreamConfig.from_dict` in `config.py`.

When in doubt, the live entry point is `hf_moondream.HfMoondream` → `moondream.MoondreamModel`, using the dataclass `MoondreamConfig` from `config.py`.
