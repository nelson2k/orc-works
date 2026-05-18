# Quantization

Quantization shrinks model weights from F32 or F16 down to integer or low-bit float formats. llama.cpp supports a wide range, all the way from 1.5 bits per weight up to 8-bit integers, plus the new MXFP4 4-bit float used by `gpt-oss`. Quantization runs offline on a GGUF file; the resulting GGUF is then loaded by libllama at inference time.

## The quant types

### Block-quantized integer (`Q*_0` / `Q*_1`)

Earliest formats. A block of 32 weights shares one (or two) F16 scaling parameters; the weights themselves are stored as `n`-bit unsigned integers.

| Type | Bits | Block | Notes |
|---|---|---|---|
| `Q4_0` | 4 | 32 | single scale per block |
| `Q4_1` | 4 | 32 | scale + min per block |
| `Q5_0` | 5 | 32 | |
| `Q5_1` | 5 | 32 | |
| `Q8_0` | 8 | 32 | usually negligible quality loss |
| `Q8_1` | 8 | 32 | + sum, used for activations |

### K-quants

Improved block layout with per-super-block (256 weights) F16 scales, and per-sub-block 6-bit scales. Better quality at the same bit width.

| Type | Effective bits |
|---|---|
| `Q2_K` | ~2.6 |
| `Q3_K_S/M/L` | ~3.4 / 3.4 / 3.7 |
| `Q4_K_S/M` | ~4.3 / 4.6 |
| `Q5_K_S/M` | ~5.2 / 5.5 |
| `Q6_K` | ~6.6 |
| `Q8_K` | ~8.5 (intermediate, used for compute) |

The `_M` variants use a mixed scheme: some layers / tensors are kept at higher precision while others use the lower form.

### IQ-quants (importance-matrix aware)

Designed by Ikawrakow. Use a non-uniform code-book and rely on an importance matrix (a per-tensor activation statistic produced by `llama-imatrix`) for best results.

| Type | Effective bits |
|---|---|
| `IQ1_S`, `IQ1_M` | ~1.6, 1.8 |
| `IQ2_XXS`, `IQ2_XS`, `IQ2_S` | ~2.1, 2.3, 2.5 |
| `IQ3_XXS`, `IQ3_S` | ~3.1, 3.5 |
| `IQ4_NL`, `IQ4_XS` | ~4.5, 4.3 |

IQ quants are smaller and frequently higher-quality than the equivalent-bit `Q*_K` quants — *if* you supply an imatrix. Without one, fall back to the `Q*_K` family.

### MXFP4

`GGML_TYPE_MXFP4` — Microscaling FP4 with shared E8M0 (1-byte) scales per 32-element block. Used natively by OpenAI's gpt-oss release. Maintains a sign bit and 3 bits of mantissa per weight.

### Float types kept at full precision

`F32`, `F16`, `BF16` are the unquantized choices. Conversion from HF produces F16 or BF16 by default depending on the source.

## How llama-quantize works

```bash
llama-quantize input-f16.gguf output-q4km.gguf Q4_K_M
```

Per tensor:

1. Read the tensor info from the input GGUF.
2. Apply per-tensor type overrides (e.g. keep `output.weight` at Q6_K even when targeting Q4_K).
3. If `--imatrix` was supplied and the type is an IQ quant, look up the corresponding importance vector by tensor name.
4. Convert (CPU-side) into the target format.
5. Write the new tensor and metadata to the output GGUF.

The "mixed" presets (`Q4_K_M`, `Q5_K_M`, etc.) carry an internal recipe that promotes a few important tensors (e.g. `attn.k_proj`, output projections, `attn_norm`) to higher precision. The `--pure` flag disables this and quantizes every weight tensor to the requested type uniformly.

### Useful flags

| Flag | Effect |
|---|---|
| `--imatrix FILE` | Importance matrix for IQ / advanced quants. |
| `--include-weights NAME` / `--exclude-weights NAME` | Per-tensor inclusion (repeatable). |
| `--leave-output-tensor` | Keep `output.weight` at the original precision. |
| `--output-tensor-type T` | Force `output.weight` to type T. |
| `--token-embedding-type T` | Force `token_embd.weight` to type T. |
| `--pure` | No per-tensor promotion; quantize everything to one type. |
| `--keep-split` | Output a sharded GGUF if input was sharded. |
| `--no-imatrix-rules` | Skip the curated tensor → quant-type table used by mixed recipes. |
| `--allow-requantize` | Re-quantize an already-quantized model. Lossy; usually avoid. |
| `--check-tensors` | Validate tensor data sanity after load. |

## Importance matrices

```bash
llama-imatrix \
  -m model-f16.gguf \
  -f wiki.train.raw \
  -o imatrix.gguf \
  --chunk 512 \
  --process-output
```

`llama-imatrix` runs the model on a calibration corpus and accumulates, per tensor, the average of `x^2` over input activations. The result is saved as a GGUF (or `.dat` with `--output-format dat`) that records one vector per quantized tensor.

Recommended calibration data: 4–10 MB of in-distribution text. WikiText, Pile, or model-domain-specific text all work; the official choice is `wiki.train.raw`. More data marginally helps; >50 MB rarely moves the needle.

Statistics: pass `--show-statistics` to print per-tensor min/max/mean/std of the importance vector — useful for spotting tensors that received insufficient activation coverage.

## Picking a quant

Rough rules of thumb on consumer hardware:

- **Q8_0** — practically lossless, ~50 % of F16 size. Use when you have the VRAM.
- **Q6_K** — very close to Q8_0 in quality, smaller.
- **Q5_K_M** — strong default if you can afford ~5 bits per weight.
- **Q4_K_M** — the most common "fits-in-VRAM" target. Quality drop is small.
- **Q3_K_M / Q3_K_S** — noticeable degradation but usable for many models. Below 4 bits, larger base models hold up better than smaller ones (a 70 B at Q3 often beats a 7 B at Q5 in quality).
- **IQ3_S / IQ3_XXS / IQ2_*** — only with an imatrix. Designed for tight memory budgets; quality varies a lot per tensor's sensitivity.
- **IQ1_S / IQ1_M** — extreme compression; usable mostly for very large MoE models where the per-expert tensors are weak signal.

Measure with `llama-perplexity --kl-divergence` against the F16 baseline to compare two quants quantitatively.

## Quant of K and V cache

Independent from weight quantization, the KV cache itself can be stored at lower precision:

```bash
llama-cli -m model.gguf -ctk q8_0 -ctv q8_0
```

Allowed values: `f32`, `f16`, `bf16`, `q8_0`, `q4_0`, `q4_1`, `iq4_nl`, `q5_0`, `q5_1`. F16 is the default. `q8_0` saves ~50 % cache memory with imperceptible quality loss; lower types start to noticeably hurt long-context retrieval.

For draft models in speculative decoding, the same flags exist as `-ctkd` / `-ctvd`.

## Quantization in code

Programmatic quantize via libllama:

```c
struct llama_model_quantize_params p = llama_model_quantize_default_params();
p.ftype          = LLAMA_FTYPE_MOSTLY_Q4_K_M;
p.imatrix        = imatrix_data;
p.kv_overrides   = overrides;

int rc = llama_model_quantize("in.gguf", "out.gguf", &p);
```

See `src/llama-quant.cpp` for the full machinery.
