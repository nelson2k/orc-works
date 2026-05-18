# Qwen2.5-VL-3B-Instruct-AWQ

A vision-language model from Alibaba's Qwen team, released January 2025. This repo (`repos-folder/Qwen2.5-VL-3B-Instruct-AWQ`) is the **Hugging Face checkpoint snapshot** of the 3-billion-parameter instruction-tuned variant, quantized to 4-bit weights with **AWQ** (Activation-aware Weight Quantization).

License: `qwen-research` (research / non-commercial — see `LICENSE` in the snapshot).
Base model: `Qwen/Qwen2.5-VL-3B-Instruct`.
Architecture: `Qwen2_5_VLForConditionalGeneration` (in HF Transformers ≥ 4.49).

## What the model does

A single multimodal model that takes interleaved text + images + video as input and produces text output. Strengths advertised by the model card:

- **Visual understanding** — objects, text, charts, icons, layouts.
- **Agentic** — drive tool use, computer use, phone use given screenshots.
- **Long video** — over 1 hour, with event localization via dynamic-FPS mRoPE.
- **Visual localization** — bounding boxes and points, with stable JSON-coordinate output.
- **Structured output** — invoices, forms, tables → structured text.

The 3B is the smallest of three sizes (3B / 7B / 72B).

## Why "AWQ"?

AWQ is a post-training, weight-only 4-bit quantizer. It picks per-channel scales that protect the activation-salient weights from rounding error, leaving inference fast (no per-token dequant on the activation side) and accuracy close to the bf16 baseline. The HF snapshot here uses:

- 4-bit weights, group size 128, GEMM kernels, zero-point quant.
- `modules_to_not_convert: ["visual"]` — **only the language decoder is quantized**; the vision tower stays in bf16.

Reported impact on this model (from the upstream README evals):

| Bench | BF16 | AWQ |
|---|---|---|
| MMMU_VAL | 51.7 | 49.1 |
| DocVQA_VAL | 93.0 | 91.8 |
| MMBench_DEV_EN | 79.8 | 78.0 |
| MathVista_MINI | 61.4 | 58.8 |

See [awq-quantization.md](awq-quantization.md) for details.

## Sibling docs in this folder

- [architecture.md](architecture.md) — sub-networks, dimensions, multimodal RoPE.
- [awq-quantization.md](awq-quantization.md) — what's quantized, how, and why.
- [image-pipeline.md](image-pipeline.md) — patch size, dynamic resolution, `min_pixels`/`max_pixels`.
- [tokenizer-and-templates.md](tokenizer-and-templates.md) — vocab, special tokens, chat template.
- [files.md](files.md) — every file in the snapshot, what it's for.
- [usage.md](usage.md) — loading + a minimal inference example.
- [long-context.md](long-context.md) — 128K context, YaRN, and the caveats.
