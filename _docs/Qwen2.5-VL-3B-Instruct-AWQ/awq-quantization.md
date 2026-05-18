# AWQ quantization

From `config.json`:

```json
"quantization_config": {
  "bits": 4,
  "group_size": 128,
  "modules_to_not_convert": ["visual"],
  "quant_method": "awq",
  "version": "gemm",
  "zero_point": true
}
```

## What's actually quantized

Only the **language decoder** weights. The `visual` (ViT) sub-module is excluded by `modules_to_not_convert` and runs in bf16. That's why the on-disk size is **~3.4 GB** instead of the ~1.7 GB you'd expect if everything were 4-bit — the unquantized vision tower (~600 MB), the bf16 token embedding (`vocab_size × hidden_size × 2 = 151 936 × 2048 × 2 ≈ 600 MB`, which is **tied** as the LM head so it's only stored once thanks to `tie_word_embeddings: true`), plus AWQ scales/zeros pad the file.

For comparison, the BF16 base (`Qwen2.5-VL-3B-Instruct`) is ~7 GB on disk.

## What AWQ does

AWQ (Activation-aware Weight Quantization, [arXiv:2306.00978](https://arxiv.org/abs/2306.00978)) is a **post-training, weight-only** quantizer:

1. For each linear weight matrix, look at calibration-data activation magnitudes per input channel.
2. Pick per-channel scales that move the "salient" (high-activation) channels into a range where 4-bit rounding error is minimized.
3. Quantize: `w_q = round((w * scale) / group_step)`, store scales and zero-points alongside.
4. At inference, weights are dequantized on the fly inside fused GEMM kernels.

Activations stay in higher precision (typically fp16/bf16). This makes AWQ "weight-only" — distinct from W8A8/W4A8 schemes that also quantize activations.

## Parameters here

| Setting | Value | Effect |
|---|---|---|
| `bits` | 4 | 4-bit weights, ~4× compression for quantized layers |
| `group_size` | 128 | Scales/zeros shared across each contiguous run of 128 input channels — finer than per-tensor, coarser than per-channel |
| `version` | `gemm` | Use the GEMM kernel path (good for batched / longer prompt inference). The alternative is `gemv` (single-token decode) |
| `zero_point` | `true` | Asymmetric quant (each group has its own zero) — handles non-zero-centered weight distributions better than symmetric |
| `modules_to_not_convert` | `["visual"]` | Leave the ViT in bf16 |

## Quality cost

From the upstream evals (3B AWQ vs BF16):

| Benchmark | BF16 | AWQ | Δ |
|---|---|---|---|
| MMMU_VAL | 51.7 | 49.1 | -2.6 |
| DocVQA_VAL | 93.0 | 91.8 | -1.2 |
| MMBench_DEV_EN | 79.8 | 78.0 | -1.8 |
| MathVista_MINI | 61.4 | 58.8 | -2.6 |

Worth the trade for fitting in low-VRAM GPUs (3B AWQ runs comfortably on 8 GB; BF16 needs ~12 GB for the same context).

## Loading

Requires the **AWQ kernel package** for runtime dequant:

```bash
pip install autoawq
```

The HF Transformers integration is transparent — `Qwen2_5_VLForConditionalGeneration.from_pretrained(...)` picks up `quantization_config` automatically and routes through `autoawq`'s kernels. No `quantization_config=...` argument needed at load time.

## Caveats

- **Inference only**. AWQ checkpoints are not designed for fine-tuning; if you need LoRA/SFT, start from the BF16 base.
- **Hardware**: AWQ GEMM kernels target NVIDIA SM ≥ 70 (Volta+). Older cards or non-CUDA backends may fall back to slower paths or fail outright.
- **flash-attn 2** is still recommended on top of AWQ — they're orthogonal optimizations (AWQ = weight memory + matmul throughput; FA2 = attention compute + memory).
