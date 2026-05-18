# Package layout

Top-level directories of the `llama.cpp` repository.

```
llama.cpp/
├── README.md / CONTRIBUTING.md / AGENTS.md / CLAUDE.md / SECURITY.md
├── LICENSE / AUTHORS / CODEOWNERS
├── CMakeLists.txt / CMakePresets.json / Makefile
├── pyproject.toml / mypy.ini / pyrightconfig.json
├── requirements.txt
├── flake.nix
├── build-xcframework.sh
│
├── include/                 Public C headers for libllama
│   ├── llama.h
│   └── llama-cpp.h          Tiny C++ helpers (unique_ptr wrappers)
│
├── src/                     libllama implementation
│   ├── llama.cpp            Top-level entry points + chat-template glue
│   ├── llama-model.{h,cpp}  Model representation, devices, tensor placement
│   ├── llama-model-loader.* GGUF loader (mmap, splits, kv overrides)
│   ├── llama-model-saver.*  GGUF writer
│   ├── llama-arch.{h,cpp}   Architecture enum + per-arch tensor names
│   ├── llama-context.*      Per-session compute state (batches, KV)
│   ├── llama-batch.*        Batch building and scheduling
│   ├── llama-graph.*        Compute graph construction
│   ├── llama-hparams.*      Hyperparameter storage
│   ├── llama-cparams.*      Context parameter struct
│   ├── llama-memory*.{h,cpp} KV-cache variants: standard, SWA, hybrid, recurrent
│   ├── llama-kv-cache*.*    Standard KV cache + interleaved-SWA cache
│   ├── llama-kv-cells.h     Cell tracking
│   ├── llama-vocab.*        Tokenizer (BPE, SPM, WPM, Unigram, RWKV, PLaMo2)
│   ├── llama-grammar.*      GBNF grammar runtime
│   ├── llama-sampler.*      Sampler chain (top-k, top-p, temp, mirostat, …)
│   ├── llama-chat.*         Built-in chat templates
│   ├── llama-adapter.*      LoRA / control-vector loading
│   ├── llama-quant.*        Quantize entry points
│   ├── llama-mmap.*         OS-specific mmap / mlock / direct I/O
│   ├── llama-io.*           Generic I/O abstraction
│   ├── llama-impl.*         Logging, asserts
│   ├── unicode*.{h,cpp,data} Unicode normalization tables
│   └── models/              One file per architecture (llama, qwen, gemma, …)
│
├── ggml/                    The tensor library (own subproject)
│   ├── include/             Public ggml headers
│   ├── src/
│   │   ├── ggml.c / ggml.cpp / ggml-impl.h
│   │   ├── ggml-alloc.c     Memory allocator
│   │   ├── ggml-backend*    Backend registry + meta-device + dynamic loading
│   │   ├── ggml-quants.{c,h} Generic quant code
│   │   ├── ggml-opt.cpp     Training optimizer
│   │   ├── gguf.cpp         GGUF reader/writer (C++)
│   │   ├── ggml-cpu/        CPU kernels (SIMD per ISA)
│   │   ├── ggml-cuda/       CUDA kernels (.cu / .cuh per op)
│   │   ├── ggml-metal/      Metal kernels (.metal shaders)
│   │   ├── ggml-vulkan/     Vulkan kernels (GLSL/SPIRV)
│   │   ├── ggml-sycl/       Intel SYCL (oneAPI)
│   │   ├── ggml-hip/        AMD HIP (CUDA-like)
│   │   ├── ggml-musa/       Moore Threads MUSA
│   │   ├── ggml-cann/       Huawei Ascend NPU
│   │   ├── ggml-opencl/     OpenCL (Adreno GPUs in particular)
│   │   ├── ggml-openvino/   Intel OpenVINO
│   │   ├── ggml-rpc/        RPC client/server
│   │   ├── ggml-webgpu/     WebGPU
│   │   ├── ggml-blas/       BLAS dispatch
│   │   ├── ggml-hexagon/    Qualcomm Hexagon DSP
│   │   ├── ggml-zdnn/       IBM Z deep-neural NPU
│   │   ├── ggml-zendnn/     AMD ZenDNN
│   │   ├── ggml-virtgpu/    VirtGPU passthrough
│   │   ├── ggml-threading.* Worker pool
│   │   └── ggml-common.h    Shared types
│   └── cmake/               Build helpers per backend
│
├── common/                  Helpers shared by every tool (NOT public API)
│   ├── arg.* / common.*     The CLI argument parser used everywhere
│   ├── chat.* / chat-peg-parser.* / chat-auto-parser*.*
│   ├── sampling.*           Sampler chain construction from args
│   ├── jinja/               Jinja2-style chat-template engine in C++
│   ├── peg-parser.* / regex-partial.* / json-partial.*
│   ├── json-schema-to-grammar.* / llguidance.*  Grammar generators
│   ├── speculative.* / ngram-cache.* / ngram-map.* / ngram-mod.*
│   ├── http.h / hf-cache.* / download.*  HTTP client + HF cache
│   ├── fit.* / preset.*     Auto-fit-in-VRAM and named presets
│   ├── reasoning-budget.*   Budget tracking for reasoning models
│   ├── log.* / debug.* / console.* / unicode.* / base64.hpp
│   └── build-info.{h,cpp.in}
│
├── tools/                   Console tools (most ship as `llama-*` binaries)
│   ├── cli/                 llama-cli  (interactive / batch text gen)
│   ├── server/              llama-server  (HTTP + WebUI)
│   ├── ui/                  llama-ui  (Svelte SPA, bundled into the server)
│   ├── llama-bench/         llama-bench  (perf measurements)
│   ├── quantize/            llama-quantize  (GGUF → quantized GGUF)
│   ├── imatrix/             llama-imatrix  (importance matrix for better quant)
│   ├── perplexity/          llama-perplexity  (ppl evaluation)
│   ├── mtmd/                libmtmd + mtmd-cli (multimodal: images, audio)
│   ├── tokenize/            llama-tokenize  (debug the tokenizer)
│   ├── tts/                 llama-tts  (text-to-speech via OuteTTS)
│   ├── parser/              llama-parser  (test chat template / auto-parser)
│   ├── rpc/                 rpc-server  (remote ggml backend)
│   ├── completion/          shell-completion generator
│   ├── batched-bench/       Synthetic batched inference bench
│   ├── cvector-generator/   Control-vector training
│   ├── export-lora/         Merge a LoRA into the base model
│   ├── fit-params/          Auto-fit param search
│   ├── gguf-split/          Split/merge GGUF files
│   └── results/             Stored bench results
│
├── examples/                Smaller demos and reference programs
│   ├── simple/ / simple-chat/         Minimal C++ inference
│   ├── batched/ / batched.swift       Batched decoding examples
│   ├── embedding/                     Embedding extraction
│   ├── parallel/                      Parallel prompts
│   ├── speculative/ / speculative-simple/  Speculative decoding
│   ├── retrieval/                     RAG-style retrieval demo
│   ├── lookup/ / lookahead/           Prompt-lookup decoding
│   ├── passkey/                       Long-context passkey eval
│   ├── eval-callback/                 Hook into the compute graph
│   ├── diffusion/                     Image diffusion via ggml
│   ├── training/                      ggml-opt fine-tuning
│   ├── llama-eval/                    Generic eval harness
│   ├── gguf/ / gguf-hash/             GGUF tooling examples
│   ├── llama.android/ / llama.swiftui/ / llama.vim/  Platform/editor demos
│   ├── model-conversion/              Convert helpers
│   ├── convert-llama2c-to-ggml/       Legacy llama2.c → GGML
│   ├── convert_legacy_llama.py        Old-pre-HF llama checkpoint converter
│   ├── json_schema_to_grammar.py / pydantic_models_to_grammar.py / regex_to_grammar.py
│   ├── sycl/                          SYCL-specific sample
│   ├── server-llama2-13B.sh / server_embd.py / chat.mjs / chat.sh
│   └── save-load-state/               Session save/load demo
│
├── gguf-py/                 Standalone Python package "gguf"
│   ├── pyproject.toml
│   ├── gguf/__init__.py / gguf.py / constants.py / lazy.py
│   ├── gguf/gguf_reader.py / gguf_writer.py / metadata.py / quants.py
│   ├── gguf/tensor_mapping.py / utility.py / vocab.py
│   ├── gguf/scripts/        gguf_dump / gguf_set_metadata / gguf_new_metadata / gguf_convert_endian / gguf_editor_gui
│   ├── examples/            reader.py, writer.py
│   └── tests/
│
├── convert_hf_to_gguf.py    Main HF-checkpoint → GGUF converter
├── convert_hf_to_gguf_update.py  Refresh the tokenizer-pre-hashes table
├── convert_llama_ggml_to_gguf.py Legacy GGML v1/v2 → GGUF
├── convert_lora_to_gguf.py  LoRA adapter → GGUF
├── conversion/              Per-model conversion helpers and scratch
│
├── grammars/                Sample GBNF grammars (json, list, arithmetic, c, …)
├── models/                  Test/vocab GGUF files used by the test suite
│   └── templates/           Reference chat templates
│
├── docs/                    User & developer documentation
│   ├── build.md / install.md / docker.md
│   ├── multimodal.md / multimodal/        Per-model multimodal guides
│   ├── backend/                            Per-backend setup notes
│   ├── ops.md / ops/                       ggml op coverage matrix
│   ├── function-calling.md / autoparser.md / llguidance.md
│   ├── multi-gpu.md / speculative.md / preset.md
│   ├── android.md / android/
│   └── development/                        HOWTO-add-model, parsing, perf tips
│
├── benches/                 Benchmark harnesses
├── tests/                   GoogleTest + Python tests
├── pocs/                    Proof-of-concept code (not for users)
├── scripts/                 Maintainer scripts (build-info, sync-ggml, server-bench, tool_bench, ...)
├── ci/                      CI helpers
├── cmake/                   Toolchain files + helper modules
├── requirements/            Per-task pinned Python requirements
├── licenses/                Third-party license texts
├── media/                   Logos and graphics
└── vendor/                  Vendored dependencies (httplib, nlohmann/json, etc.)
```

The big things to remember:

- The compile-time entry point is `CMakeLists.txt` at the root, which descends into `ggml/`, then `src/`, then `common/`, then optionally `tools/`, `examples/`, `tests/`.
- Public ABI = `include/llama.h` only. Everything in `src/` is internal and may change.
- ggml is treated as a sub-library; its `ggml/CMakeLists.txt` can also be consumed standalone.
- `common/` is **not** a public API. Tools statically link it but third-party consumers should depend only on `libllama`.
