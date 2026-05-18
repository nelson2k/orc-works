# llama.cpp

Source: `repos-folder/llama.cpp` (ggml-org / GitHub user ggerganov). License: MIT (code), per-model licenses for weights. Build: C/C++ with a small Python toolchain alongside for model conversion. Minimum: C++17, CMake 3.14, Python 3.10+ for the converter scripts.

## What it is

A plain C/C++ runtime for transformer inference. The library has no required runtime dependencies — it can be built standalone with just a C++ compiler and CMake — and is designed to run LLMs on essentially any device, from a phone to a multi-GPU server.

The project owns three related artifacts:

- **ggml** — a small tensor library with its own graph, allocator, backend abstraction, and per-architecture kernels. Maintained in this repo (originally split out at `ggml-org/ggml` but mirrored here as the canonical copy).
- **libllama** — the LLM runtime built on top of ggml. Loads GGUF model files, builds the per-architecture compute graph, runs decode/encode with batching, manages a KV cache, exposes sampling and grammar.
- **tools/** and **examples/** — many CLIs and small programs that use libllama: `llama-cli`, `llama-server` (OpenAI-compatible HTTP server), `llama-bench`, `llama-quantize`, `llama-imatrix`, `mtmd-cli` (multimodal), and more.

Plus a separate Python package `gguf-py/gguf` that reads/writes the GGUF model file format, and a set of `convert_*.py` scripts that turn HuggingFace / safetensors / legacy-llama checkpoints into GGUF.

## The three-layer architecture

```
+--------------------------------------------------------------+
|  CLIs / examples / bindings (tools/, examples/)              |
|  llama-cli, llama-server, llama-bench, llama-quantize, ...   |
+--------------------------------------------------------------+
|  libllama  (src/, include/llama.h)                           |
|  model loader, arch implementations, KV cache, samplers,     |
|  grammars, chat templates, batching, speculative decoding    |
+--------------------------------------------------------------+
|  ggml  (ggml/)                                               |
|  tensors, compute graph, backend registry, per-device        |
|  kernels (CPU/CUDA/Metal/Vulkan/SYCL/HIP/CANN/OpenCL/...)    |
+--------------------------------------------------------------+
```

`libllama` is the public consumable. It speaks the GGUF format end-to-end, decides which ggml backend(s) to use, and drives generation. Everything in `tools/` and `examples/` is a thin user of the same C API.

## What it can do

- Text generation with hundreds of supported model architectures (LLaMA family, Mistral / Mixtral, Qwen, Phi, Gemma, GLM, DeepSeek, Falcon, MPT, BERT, RWKV, Mamba, Granite, Hunyuan, Bitnet, OLMo, …).
- Multiple quantization types from 1.5-bit up to 8-bit integer (Q2_K, Q3_K, Q4_K, Q5_K, Q6_K, Q8_0, IQ-series, MXFP4) plus F16, BF16, F32.
- KV cache with optional Flash Attention, full or sliding-window attention, quantized K and V caches.
- Parallel decoding of multiple sequences in one batch (continuous batching for the server).
- Speculative decoding with a draft model.
- Multimodal: vision and audio inputs through `libmtmd` (sister library in `tools/mtmd/`).
- Function calling / tool use, schema-constrained JSON output, GBNF grammars.
- Speech-to-text via OuteTTS, image generation pipelines (diffusion examples).
- Multi-GPU split (by layer / row / tensor) and CPU+GPU hybrid offload.
- RPC backend: run the model on one or more remote machines over TCP.

## Three ways to use it

1. **Command-line** — pick a tool in `tools/` (most usefully `llama-cli` and `llama-server`) and run it against a GGUF file. Supports auto-download from Hugging Face (`-hf user/repo[:quant]`).
2. **C API** — link against `libllama` and call functions from `include/llama.h`. Open the model, create a context, build batches, decode tokens, sample. Stable, C-only ABI.
3. **Bindings** — many third-party bindings exist (Python, Go, Node, Ruby, Rust, C#, Java, Swift, Zig, Dart, …). They wrap libllama from each language.

## Notable design properties

- **Single binary, optional shared lib.** Tools statically link by default; build with `-DBUILD_SHARED_LIBS=ON` to get `libllama.so` / `llama.dll`.
- **Backend autoload.** At init time libllama discovers all installed ggml backends (CPU is always present; CUDA, Metal, Vulkan, etc. are loaded as plugins or compiled in) and picks devices automatically.
- **GGUF is the only model format.** Everything else (HuggingFace, PyTorch checkpoints) has to be converted first using the Python scripts.
- **No PyTorch at runtime.** The Python is only for conversion / offline tooling, never for inference.
- **AVX/AVX2/AVX512/AMX on x86, NEON/Accelerate/Metal on Apple Silicon, RVV/ZVFH/ZFH on RISC-V** are all handled in hand-written ggml-cpu kernels.

## Hot spots in the tree

| Path | What lives here |
|---|---|
| `include/llama.h` | The C API (~1600 lines). |
| `src/` | libllama implementation, split across `llama-*.cpp`. |
| `src/models/` | One file per supported model architecture. |
| `ggml/include/` | ggml public headers (`ggml.h`, `ggml-backend.h`, one per backend). |
| `ggml/src/` | ggml core + one subfolder per backend (`ggml-cuda/`, `ggml-metal/`, `ggml-vulkan/`, `ggml-sycl/`, `ggml-cpu/`, …). |
| `common/` | A non-API helper library shared by every tool: arg parser, chat / sampling glue, JSON-schema → grammar, HF download, logging. |
| `tools/` | CLIs: `cli`, `server`, `bench`, `quantize`, `imatrix`, `mtmd`, `tts`, `perplexity`, `parser`, `rpc`, `ui`, … |
| `examples/` | Smaller demo programs (simple, simple-chat, embedding, parallel, speculative, training, gguf, batched, …). |
| `gguf-py/` | The standalone Python `gguf` package (reader/writer, scripts, GUI editor). |
| `convert_hf_to_gguf.py` | The main HF-to-GGUF converter at repo root. |
| `docs/` | Markdown documentation set (build, multimodal, backends, function calling, autoparser, …). |

## Entry points

| Surface | How to start |
|---|---|
| CLI inference | `llama-cli -m model.gguf -p "..."` |
| HTTP server | `llama-server -hf ggml-org/gemma-3-1b-it-GGUF` |
| Quantize a model | `llama-quantize in.gguf out.gguf Q4_K_M` |
| Benchmark | `llama-bench -m model.gguf` |
| Convert HF model | `python convert_hf_to_gguf.py /path/to/hf-checkpoint` |
| Embed in your app (C) | include `llama.h`, link `libllama`, call `llama_backend_init` → `llama_model_load_from_file` → `llama_init_from_model` → `llama_decode` |
