# ggml — the tensor library

`ggml` is the tensor and compute-graph library that powers llama.cpp. It lives under `ggml/` and can be consumed either as a sub-library of llama.cpp or standalone. Public headers are under `ggml/include/`.

It is intentionally small: no PyTorch, no autograd at runtime, no dynamic shapes. You build a graph, allocate tensors, run a compute, read back results.

## Core abstractions

- `ggml_context` — an arena for tensors and graph nodes. You create one with `ggml_init({ .mem_size, .mem_buffer, .no_alloc })` and tear it down with `ggml_free`. Tensor memory comes from this arena.
- `ggml_tensor` — n-dimensional tensor (up to 4 dims by default). Carries shape `ne[4]`, strides `nb[4]`, element type, op tag, source operands (`src[GGML_MAX_SRC]`), per-op params, and a pointer to the data buffer.
- `ggml_cgraph` — directed graph of tensor operations. Built by repeatedly calling op constructors (`ggml_mul_mat`, `ggml_add`, `ggml_silu`, …) that wire their result tensor's `src[]` to the input tensors, then expanding with `ggml_build_forward_expand(graph, output)`.

## Element types

Mixed precision, integer quantization, and packed formats — declared in `ggml.h`:

```
GGML_TYPE_F32, F16, BF16,
GGML_TYPE_Q4_0, Q4_1, Q5_0, Q5_1, Q8_0, Q8_1,
GGML_TYPE_Q2_K, Q3_K, Q4_K, Q5_K, Q6_K, Q8_K,
GGML_TYPE_IQ2_XXS, IQ2_XS, IQ2_S, IQ3_XXS, IQ3_S, IQ4_NL, IQ4_XS, IQ1_S, IQ1_M,
GGML_TYPE_MXFP4,
GGML_TYPE_I8, I16, I32, I64,
GGML_TYPE_F64,
GGML_TYPE_COUNT
```

The K-quants (Q2_K – Q8_K) pack super-block-quantized weights with per-block scales; the IQ-quants are import-matrix-aware (best with a `imatrix` calibration); MXFP4 is the new OpenAI / NVIDIA 4-bit FP format used by `gpt-oss`.

## Backends

A backend is a device + the code that runs ops on it. The registry is in `ggml-backend.h`:

```c
size_t                  ggml_backend_reg_count(void);
ggml_backend_reg_t      ggml_backend_reg_get(size_t i);
ggml_backend_dev_t      ggml_backend_dev_get(size_t i);
ggml_backend_dev_type   ggml_backend_dev_type(ggml_backend_dev_t);  // CPU / GPU / IGPU / ACCEL / META
ggml_backend_t          ggml_backend_dev_init(ggml_backend_dev_t dev, const char * params);
```

Backends ship as shared libraries (CUDA, Metal, Vulkan, SYCL, HIP, …) and are loaded by `ggml_backend_load_all()` from the install location. Each backend exposes:

- Devices (one or more).
- Buffer types (where tensors live).
- A scheduler that runs ops on its devices.

The CPU backend is always linked in. Others are auto-detected at runtime — if you ship a binary built with `GGML_CUDA=ON` it gracefully falls back to CPU on a system without an NVIDIA driver.

Per-backend public headers:

| Header | Backend |
|---|---|
| `ggml-cpu.h` | CPU (always present) |
| `ggml-cuda.h` | NVIDIA CUDA |
| `ggml-metal.h` | Apple Metal |
| `ggml-vulkan.h` | Vulkan (cross-vendor GPU) |
| `ggml-sycl.h` | Intel SYCL / oneAPI |
| `ggml-blas.h` | BLAS dispatch (OpenBLAS, MKL, BLIS, Accelerate) |
| `ggml-cann.h` | Huawei Ascend |
| `ggml-opencl.h` | OpenCL (Adreno) |
| `ggml-openvino.h` | Intel OpenVINO |
| `ggml-hexagon.h` | Qualcomm Hexagon DSP |
| `ggml-zdnn.h` | IBM Z |
| `ggml-zendnn.h` | AMD ZenDNN |
| `ggml-webgpu.h` | WebGPU (in progress) |
| `ggml-virtgpu.h` | VirtGPU |
| `ggml-rpc.h` | Remote ggml backend over TCP |
| `ggml-cpp.h` | C++ RAII helpers |
| `ggml-opt.h` | Training-time optimizer |

## Backend scheduler

`ggml-backend-meta.cpp` builds a meta-device for multi-GPU tensor parallelism. The libllama loader picks devices and split mode at model-load time; the meta scheduler partitions a graph across multiple backend devices automatically.

Split modes (controlled from libllama):

- `NONE` — single device.
- `LAYER` — layer-pipeline split: each layer runs on one device; the KV cache for that layer lives there.
- `ROW` — row-tensor split: tensor parallelism across devices.
- `TENSOR` — full tensor parallelism across all weights and the KV cache.

## Memory allocator

`ggml-alloc.{c,h}` provides the graph allocator. Tensors are scheduled into per-buffer arenas with lifetime analysis so a fixed budget can host a much larger logical graph (op outputs that go dead get reused).

## Compute

```c
struct ggml_cgraph * gf = ggml_new_graph(ctx);
ggml_build_forward_expand(gf, output);
ggml_graph_compute_with_ctx(ctx, gf, /*n_threads*/ 8);
```

Or, for backend execution:

```c
ggml_backend_t backend = ggml_backend_init_by_type(GGML_BACKEND_DEVICE_TYPE_GPU, NULL);
ggml_backend_sched_t sched = ggml_backend_sched_new(&backend, NULL, 1, /*max_nodes*/ 8192, false);
ggml_backend_sched_graph_compute(sched, gf);
```

The CPU backend uses a worker pool (`ggml-threading.cpp`) with optional OpenMP. AVX/AVX2/AVX512/AMX/NEON kernels live under `ggml/src/ggml-cpu/`.

## Operations

The op set is in `ggml.h` (~2800 lines) and covers what's needed for LLM inference:

- Elementwise: add, sub, mul, div, sqr, sqrt, log, sin, cos, tanh, gelu, silu, relu, swiglu, gated_linear_attn, …
- Reductions: sum, mean, max, argmax, std, var, l2_norm, rms_norm, group_norm, layer_norm.
- Linear: `mul_mat`, `mul_mat_id` (MoE), `out_prod`, `set_rows`, `get_rows`.
- Attention building blocks: `rope`, `rope_back`, `flash_attn_ext`, `softmax`, `softmax_back`, `attn_bias`.
- Conv / pool: `conv_1d`, `conv_2d`, `conv_transpose_2d`, `pool_1d`, `pool_2d`.
- Shape / data movement: `reshape`, `view`, `permute`, `transpose`, `cont`, `cpy`, `dup`, `concat`, `repeat`, `pad`, `unpad`, `tile`, `acc`, `scale`, `clamp`.
- RNN: `ssm_conv`, `ssm_scan` (Mamba); `time_mix` / `channel_mix` (RWKV).
- Quantization: `quantize_q*_*` per format.

A complete op coverage matrix per backend lives in `docs/ops.md`.

## gguf

`gguf.h` defines the file format used by llama.cpp and other ggml-based projects (whisper.cpp, stable-diffusion.cpp, …). Two layers:

- `gguf_context` — in-memory representation of a GGUF file: a header, a flat key/value metadata map, and a list of tensor infos pointing into a tensor data section. Created via `gguf_init_empty()` or `gguf_init_from_file(path, params)`.
- Key/value getters/setters: `gguf_get_kv_type`, `gguf_get_val_u32`, `gguf_get_arr_data`, `gguf_set_val_str`, `gguf_set_arr`, `gguf_set_kv` (copy from another context), `gguf_remove_key`.
- Tensor info: `gguf_get_n_tensors`, `gguf_find_tensor(ctx, name)`, `gguf_get_tensor_offset`, `gguf_add_tensor(ctx, t)`.
- Persistence: `gguf_write_to_file(ctx, path, only_meta)`, plus appended-tensor variants for streaming writes.

The same format is implemented in Python under `gguf-py/` for offline conversion.

## ggml-opt

`ggml-opt.cpp` adds a small training/optimization loop on top of the compute graph: gradient ops, AdamW, simple datasets. Used by the `examples/training/` demo and the LoRA training paths.

## Op-documentation

`docs/ops.md` is auto-generated by `scripts/create_ops_docs.py` and lists every op × every backend, marking which combinations are implemented. Useful when porting a new model architecture or contributing a new backend.
