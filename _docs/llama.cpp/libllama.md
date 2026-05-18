# libllama — the C API

`libllama` is the LLM runtime. The whole public surface lives in `include/llama.h` (~1600 lines, C-only). Every CLI tool and every third-party binding talks to this header.

## Opaque types

```c
struct llama_vocab;
struct llama_model;
struct llama_context;
struct llama_sampler;

typedef int32_t llama_pos;
typedef int32_t llama_token;
typedef int32_t llama_seq_id;
typedef struct llama_memory_i * llama_memory_t;
```

You never allocate these directly — every one is produced by a `llama_*_init_*` factory and freed by a matching `llama_*_free`.

## Lifecycle

```c
llama_backend_init();   // load all available ggml backends (CPU, CUDA, Metal, ...)

llama_model_params  mp = llama_model_default_params();
mp.n_gpu_layers        = 99;
struct llama_model * model = llama_model_load_from_file("model.gguf", mp);

llama_context_params cp = llama_context_default_params();
cp.n_ctx               = 4096;
cp.n_batch             = 2048;
struct llama_context * ctx = llama_init_from_model(model, cp);

// ... use ctx ...

llama_free(ctx);
llama_model_free(model);
llama_backend_free();
```

`llama_backend_init` does ggml time-init, builds the f16 lookup table, and calls `ggml_backend_load_all` so every shared-lib backend is registered. The numa policy is set separately by `llama_numa_init(GGML_NUMA_STRATEGY_*)`.

## Loading models

Three load entry points, all in `llama.h`:

- `llama_model_load_from_file(path, params)` — single-file GGUF on disk.
- `llama_model_load_from_splits(paths, n_paths, params)` — multi-file split GGUF (`-00001-of-00005.gguf` …).
- `llama_model_load_from_file_ptr(FILE *, params)` — already-open file handle.

Plus `llama_model_init_from_user(metadata, set_tensor_fn, ud, params)` for fully-custom load flows (used by training and quantization).

`llama_model_params` covers: device list, split mode (`NONE` / `LAYER` / `ROW` / `TENSOR`), main GPU, tensor split ratios, KV overrides, tensor buffer-type overrides, mmap, mlock, direct I/O, vocab-only, check-tensors, no-alloc, progress callback.

## Contexts and parameters

A `llama_context` owns the per-session state: KV cache, allocator, batch storage, perf counters. `llama_context_params` covers:

- `n_ctx`, `n_batch` (logical), `n_ubatch` (physical), `n_seq_max`.
- `type_k`, `type_v` (KV cache element types — F16 by default).
- `flash_attn_type` (auto / on / off).
- `rope_scaling_type`, `rope_freq_base`, `rope_freq_scale`, `yarn_*`.
- Pooling type for embeddings, attention type, causal-attn flag.
- `n_threads`, `n_threads_batch`.
- `op_offload`, `swa_full`, `kv_unified`, `embeddings`, `kv_offload`.

## The vocab

`llama_model_get_vocab(model)` returns the `llama_vocab`. From there:

- `llama_vocab_type(vocab)` — `SPM` / `BPE` / `WPM` / `UGM` / `RWKV` / `PLAMO2` / `NONE`.
- `llama_vocab_n_tokens(vocab)`.
- `llama_vocab_get_text/score/attr(vocab, token)`.
- Token-type helpers: `is_eog`, `is_control`.
- Special tokens: `bos`, `eos`, `eot`, `sep`, `nl`, `pad`, `mask`, FIM ids (prefix/suffix/middle/repo/sep/pad).
- `llama_tokenize(vocab, text, len, tokens, n_tokens_max, add_special, parse_special)` — text → ids.
- `llama_detokenize(...)` — ids → text.
- `llama_token_to_piece(vocab, token, buf, ...)` — single token to bytes.

## Batches

```c
llama_batch batch = llama_batch_init(n_tokens, /*embd*/ 0, /*n_seq_max*/ 1);
// populate batch.token / pos / seq_id / logits ...
llama_decode(ctx, batch);  // run forward pass
llama_batch_free(batch);
```

There's also `llama_batch_get_one(tokens, n_tokens)` for the simple "one prompt, one sequence" case. The struct fields:

- `token[i]` — input token, or `LLAMA_TOKEN_NULL` if using `embd`.
- `embd[i*n_embd .. ]` — direct embeddings instead of tokens.
- `pos[i]` — absolute position.
- `n_seq_id[i]`, `seq_id[i][...]` — which sequence(s) this position belongs to (for sharing KV across sequences).
- `logits[i]` — set non-zero to ask the runtime to produce logits at position `i`.

## Decoding

- `llama_decode(ctx, batch)` — single-direction decode (causal / non-causal depending on model).
- `llama_encode(ctx, batch)` — encoder pass for encoder-decoder models.
- `llama_get_logits_ith(ctx, i)` / `llama_get_logits(ctx)` — pull logits.
- `llama_get_embeddings(ctx)` / `llama_get_embeddings_seq(ctx, seq_id)` — for embedding models.
- `llama_synchronize(ctx)` — block until any async device ops finish.

## KV cache & memory

The opaque `llama_memory_t` (a `llama_memory_i *`) is the abstract KV memory. Several implementations exist behind it:

- standard self-attention KV cache,
- sliding-window-attention (SWA) cache,
- interleaved SWA cache (iSWA — Gemma 3 etc.),
- hybrid (some layers recurrent, others attention — Jamba, Granite-Hybrid),
- recurrent (RWKV, Mamba).

Common operations: `llama_memory_clear`, `llama_memory_seq_rm`, `llama_memory_seq_cp`, `llama_memory_seq_keep`, `llama_memory_seq_add`, `llama_memory_seq_div`, `llama_memory_seq_pos_min/max`. Plus state save/load functions to dump and restore the cache.

## Samplers

A `llama_sampler` is a chain of token-level transforms applied to the logits/distribution. Built-ins (each via `llama_sampler_init_*`):

- `greedy`, `dist` (multinomial), `top_k`, `top_p`, `min_p`, `typical`, `temp`, `temp_ext`, `xtc`, `top_n_sigma`.
- `mirostat_v1`, `mirostat_v2`.
- `penalties` (repeat / freq / presence), `dry`.
- `grammar_lazy`, `grammar` — grammar-constrained sampling using `llama_grammar`.
- `logit_bias`, `infill`, `softmax`.
- `chain` — composes a sequence of samplers.

Usage:

```c
struct llama_sampler * smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
llama_sampler_chain_add(smpl, llama_sampler_init_top_k(40));
llama_sampler_chain_add(smpl, llama_sampler_init_top_p(0.95f, 1));
llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.8f));
llama_sampler_chain_add(smpl, llama_sampler_init_dist(42));

llama_token next = llama_sampler_sample(smpl, ctx, /*idx*/ -1);
llama_sampler_accept(smpl, next);
```

## LoRA, control vectors, adapters

- `llama_adapter_lora_init(model, path)` returns a `llama_adapter_lora *`.
- `llama_set_adapter_lora(ctx, adapter, scale)` activates it for the context.
- `llama_rm_adapter_lora(ctx, adapter)` removes one; `llama_clear_adapter_lora(ctx)` removes all.
- `llama_apply_adapter_cvec(ctx, data, len, n_embd, l_start, l_end)` applies a control vector to a layer range.

## Chat templates

`llama_chat_apply_template(tmpl, messages, n_msg, add_assistant, buf, buf_len)` formats a Jinja-style template. If `tmpl` is `NULL` the model's stored template is used. The internal template detector (`llm_chat_detect_template`) recognizes ~30 named templates (chatml, llama2, llama3, mistral, phi3, qwen, deepseek, command-r, granite, gemma, …).

## State save / load

`llama_state_get_size`, `llama_state_get_data`, `llama_state_set_data` — full context state. The seq variants (`llama_state_seq_*`) handle a single sequence within a context. There are also file-based helpers (`llama_state_save_file`, `llama_state_load_file`, plus seq variants).

## Helpers

- `llama_supports_mmap()`, `llama_supports_mlock()`, `llama_supports_gpu_offload()`, `llama_supports_rpc()`.
- `llama_max_devices()` (16), `llama_max_tensor_buft_overrides()` (4096).
- `llama_split_path(out, max, prefix, n, count)` — produce `prefix-00001-of-00005.gguf` paths; inverse: `llama_split_prefix`.
- `llama_print_system_info()` — list compiled-in backend features.
- `llama_time_us()`, `llama_perf_context_*`, `llama_perf_sampler_*` — perf counters.

## C++ helpers

`include/llama-cpp.h` adds a few RAII wrappers (`unique_ptr` aliases for `llama_model`, `llama_context`, `llama_sampler`, `llama_batch`). These are header-only and not part of the C ABI.

## Versioning

Source-tree version is whatever the latest tag is on `master`. Compile-time constants:

- `LLAMA_DEFAULT_SEED = 0xFFFFFFFF`
- `LLAMA_TOKEN_NULL = -1`
- `LLAMA_SESSION_VERSION = 9`
- `LLAMA_STATE_SEQ_VERSION = 2`

`LLAMA_FILE_MAGIC_GGLA / GGSN / GGSQ` — magic numbers used in old session blobs (kept for compatibility).

ABI is C-only and shared-lib-friendly: build with `-DBUILD_SHARED_LIBS=ON` for `libllama.so` / `llama.dll`.
