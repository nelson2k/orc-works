# Examples

`examples/` collects small, focused programs that demonstrate libllama features. They are not part of the install (`-DLLAMA_BUILD_EXAMPLES=OFF` skips them). Most build as `llama-<example-name>` binaries; a few are scripts.

These are the things to read when you want to learn the API by example.

## Reference examples (C++)

### `simple/`

The minimum working program: load a model, decode a prompt, sample tokens to stdout. ~70 lines. Read this first.

### `simple-chat/`

Chat variant of `simple/`. Multi-turn loop using the model's stored chat template, KV cache reused across turns.

### `embedding/`

Run the model in embedding mode and print the sentence vector. Demonstrates `llama_pooling_type` and `llama_get_embeddings_seq`.

### `batched/`

Build a single `llama_batch` that carries multiple sequences and decode them all in one forward pass. Shows the `seq_id` field of the batch.

### `batched.swift/`

Swift port of `batched/`. Demonstrates calling the C API from Swift.

### `parallel/`

Higher-level parallel decoding example: spin up N independent generation tasks against one model + one context, using slot-style scheduling.

### `speculative/` and `speculative-simple/`

Speculative decoding with a draft model and a main model. `speculative/` is the full-featured version with adjustable draft tree depth; `speculative-simple/` is the minimum demonstration.

### `lookup/`

Prompt-lookup decoding: a static draft strategy that matches an n-gram from your prompt to predict the next tokens. No draft model needed.

### `lookahead/`

Jacobi-style lookahead decoding (multiple draft positions in parallel, accept the verified prefix).

### `retrieval/`

Minimal RAG demo: embed a corpus, embed a query, top-k cosine search, then run the retrieved context through the LLM.

### `passkey/`

Reproduces the standard long-context passkey-recall benchmark. Plant a short passkey inside a long random prefix and check that the model can recall it.

### `eval-callback/`

Demonstrates the per-op evaluation callback: hook into the compute graph and inspect tensor values as decoding proceeds. Useful for debugging numerical issues.

### `save-load-state/`

Save the full context state to disk after some decoding, exit, restart, reload, continue from exactly where you left off.

### `diffusion/`

Image diffusion pipeline running on top of ggml (separate from the LLM compute graph but using the same backend infrastructure). Demonstrates ggml's flexibility beyond transformer decode.

### `training/`

Fine-tune a small model end-to-end using `ggml-opt`. Loads a dataset, runs forward + backward + AdamW updates. Not a production training stack — a demonstration of what `ggml-opt` makes possible.

### `llama-eval/`

A general-purpose eval harness. Loads a JSONL of prompts + reference completions and computes accuracy / perplexity metrics across many configurations.

### `gguf/`

Read a GGUF, dump its metadata and tensor table, write a synthetic GGUF from scratch. Reference for using `gguf_context` directly.

### `gguf-hash/`

Hash every tensor in a GGUF (sha256 / xxhash). Useful for detecting tensor corruption or verifying download integrity.

## Platform demos

### `llama.android/`

Android Studio project demonstrating libllama integration on Android (JNI bridge, single-binary deployment). Built from the same CMake graph with the NDK toolchain.

### `llama.swiftui/`

SwiftUI app for iOS / macOS that wraps libllama. Demonstrates the Swift Package layout and Metal-friendly defaults.

### `llama.vim/`

Vim plugin for in-editor inline completion. Talks to a `llama-server` running locally.

## Conversion / interop helpers

### `convert_legacy_llama.py`

Pre-HF LLaMA-1 / LLaMA-2 (consolidated.NN.pth + tokenizer.model) → GGUF.

### `convert-llama2c-to-ggml/`

Karpathy's llama2.c `model.bin` → GGML / GGUF. A tiny C program plus a Make target.

### `json_schema_to_grammar.py`

Pure-Python JSON-schema → GBNF. Mirrors the C++ implementation in `common/`; useful when you want to inspect or post-process the grammar.

### `pydantic_models_to_grammar.py` and `pydantic_models_to_grammar_examples.py`

Generate GBNF from Pydantic model definitions. Convenient if your schema lives in Python types.

### `regex_to_grammar.py`

Compile a Python regex into GBNF.

### `ts-type-to-grammar.sh`

Wraps a small TypeScript program that converts TS types → GBNF.

### `model-conversion/`

Helper scripts for the maintainers (chat-template extractors, sanity probes).

## Scripts and one-offs

### `chat.mjs` and `chat.sh`

Tiny clients for `llama-server`. `chat.mjs` is a Node.js single-file SSE client.

### `server-llama2-13B.sh`

Reference shell script for launching `llama-server` with a known good Llama-2-13B configuration. Historical, but illustrates flag-tuning.

### `server_embd.py`

Python example of using `llama-server` as an embedding service.

### `reason-act.sh`

ReAct-style agent loop using `llama-cli` + shell tools. Demonstrates tool-use patterns from a shell environment.

## Other

### `deprecation-warning/`

Tiny stub binaries that print a deprecation warning when invoked with the legacy name (e.g. `main`, `quantize`) that has since been renamed to `llama-cli`, `llama-quantize`, etc.

### `gen-docs/`

The `llama-gen-docs` tool. Walks the argument parser and produces the auto-generated argument tables in `tools/server/README.md` and `tools/cli/README.md`.

### `sycl/`

Tiny SYCL kernel demonstration, mostly for testing the SYCL backend in isolation.

### `idle/`

Idle-power benchmark — measure GPU idle draw between requests.

## Pattern across examples

Almost every C++ example follows the same shape:

```cpp
#include "common.h"
#include "llama.h"

int main(int argc, char ** argv) {
    common_params params;
    if (!common_params_parse(argc, argv, params, LLAMA_EXAMPLE_<NAME>)) return 1;

    common_init_from_params(params);
    llama_backend_init();

    auto llama_init = common_init_from_params(params);
    llama_model   * model = llama_init.model.get();
    llama_context * ctx   = llama_init.context.get();

    // ... build a batch, decode, sample ...

    llama_backend_free();
    return 0;
}
```

That is: parse args with the shared parser, build model + context, do the example-specific work, free. Reading any two examples side-by-side gives you the full API surface.
