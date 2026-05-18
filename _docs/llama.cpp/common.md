# common/ — shared helper library

The `common/` directory is a **non-public** helper library statically linked into every tool and example. It is not part of the libllama ABI. Third-party consumers should depend only on `libllama` (i.e. on `include/llama.h`); they should not link against `common/`.

That said, `common/` is where most of the "useful glue" lives — argument parsing, sampler chain construction, chat templating, JSON-schema-to-grammar, HF download, logging — and the source is a useful reference for "how to drive libllama correctly".

## Major pieces

### Argument parser — `arg.{h,cpp}`, `common.{h,cpp}`

A single shared CLI definition that knows about hundreds of flags (`-m`, `-c`, `-ngl`, `--temp`, `--grammar`, …). All tools call:

```cpp
common_params params;
if (!common_params_parse(argc, argv, params, LLAMA_EXAMPLE_COMMON)) return 1;
common_init_from_params(params);  // build llama_model_params + llama_context_params
```

Most flags also have an `LLAMA_ARG_*` env-var equivalent so the same configuration can be set from a service manager.

`common_params_parse` does a lot:

- Decoding `-hf user/repo[:quant]` / `-hff` / `-mu URL` / `-dr docker_repo` into download decisions.
- Auto-fitting tensors to device free memory (`-fit`).
- Parsing chat-template names and Jinja strings.
- Composing the sampler chain (top-k → top-p → temp → dist → grammar) from each `--<sampler>` flag.
- Loading grammar files (`.gbnf`) or compiling JSON schemas to GBNF on the fly.

### Sampling — `sampling.{h,cpp}`

Bridges `common_params.sparams` to a `llama_sampler` chain. Knows about every `LLAMA_SAMPLER_TYPE_*` and assembles them in the requested order (`--samplers`).

### Chat templates — `chat.{h,cpp}`, `chat-peg-parser.*`, `chat-auto-parser*.*`

Two angles:

- **Apply** — turn a `[{role, content}, ...]` message list into a single prompt string using the model's chat template (or a named one). Wraps both `llama_chat_apply_template` and a Jinja engine for templates that need it.
- **Parse** — when a model's output contains structured fields (tool calls, reasoning blocks), extract them. Two parsers:
  - **PEG parser** (`peg-parser.*`, `chat-peg-parser.*`) — a from-scratch parsing expression grammar engine. Custom grammars per model express how to find `<tool_call>`, `<reasoning>`, JSON-blob, etc. in the streaming output.
  - **Auto-parser** (`chat-auto-parser*.*`) — sits on top of the PEG parser and auto-detects the model family from chat template and props, then picks the right grammar.

The auto-parser is what makes `llama-server` deliver OpenAI-shaped `"tool_calls"` for many different model output formats without per-model config.

### Jinja engine — `jinja/`

A C++ Jinja-template implementation tailored to chat-template needs (most HF tokenizer.json files ship a Jinja chat template under `chat_template`). Supports `if`/`for`/`set`/`filter`, plus the subset of filters Jinja chat templates use (`tojson`, `trim`, `strftime_now`, custom `raise_exception`, etc.).

### JSON schema → GBNF — `json-schema-to-grammar.{h,cpp}`

Compiles a JSON schema into a GBNF grammar. The result is fed to libllama as a grammar sampler so generation is *guaranteed* to produce schema-valid JSON. Supported subset:

- Primitives: `string`, `number`, `integer`, `boolean`, `null`, `array`, `object`.
- Composition: `oneOf`, `anyOf`, `allOf`.
- Constraints: `enum`, `const`, `minLength`, `maxLength`, `pattern`, `minItems`, `maxItems`, `minimum`, `maximum`, `multipleOf`.
- Refs: `$ref`, `definitions` / `$defs`.

Plus partial-input variants for streaming (`json-partial.{h,cpp}`).

### llguidance — `llguidance.cpp`

Optional integration with the [LLGuidance](https://github.com/microsoft/llguidance) grammar engine. Build with `-DLLAMA_LLGUIDANCE=ON`. More expressive than GBNF (full context-free grammars, Earley-style parsing) at the cost of an extra dependency.

### Speculative decoding — `speculative.{h,cpp}`

State and orchestration for running a draft model alongside the main model. Tools that support speculative decoding (`llama-cli`, `llama-server`) drive this. Algorithm: draft N tokens with the small model, then verify with one main-model decode that accepts the longest matching prefix.

### N-gram tables — `ngram-cache.{h,cpp}`, `ngram-map.{h,cpp}`, `ngram-mod.{h,cpp}`

Static draft strategies (lookup-style speculative decoding): pre-build an n-gram table over your corpus and use it to draft continuations. Used by `examples/lookup/` and `examples/lookahead/`.

### HTTP and downloads — `http.h`, `hf-cache.{h,cpp}`, `download.{h,cpp}`

A small HTTP client wrapping libcurl (when built with `-DLLAMA_CURL=ON`). Provides:

- `-mu URL` model URL fetch.
- `-hf user/repo[:quant]` Hugging Face resolution (model + tokenizer + mmproj), with progress and resume.
- HF cache layout compatible with `~/.cache/huggingface/hub/`.
- `-dr` Docker Hub model registry support.
- `--hf-token` auth (env `HF_TOKEN`).

### Fit + presets — `fit.{h,cpp}`, `preset.{h,cpp}`

- `fit` implements `--fit on` — at load time, query each device's free memory and choose the largest context size / batch / KV-type combination that fits within `--fit-target` MiB margin.
- `preset` defines named recipes (e.g. `--preset gpu` / `--preset cpu` / `--preset balanced`) that pre-set a coherent group of flags.

### Reasoning-budget — `reasoning-budget.{h,cpp}`

Tracking machinery for the "reasoning" output channel of thinking-style models (DeepSeek-R1, Qwen3-Thinking, ...). Tracks how many tokens were emitted inside `<think>...</think>` blocks and can enforce a max budget by suppressing further reasoning output.

### Logging + console — `log.{h,cpp}`, `debug.{h,cpp}`, `console.{h,cpp}`

- `log` is the structured logger used by every tool (with verbosity levels, colors, file output, timestamps).
- `console` is a thin TTY abstraction (resize, raw mode, color reset on exit) for the REPL in `llama-cli -i`.

### Build metadata — `build-info.{h,cpp.in}`, `build-info.cpp`

Generated at build time from git: build number, commit hash, compiler. Exposed via `--version` and embedded in the server's `/props` endpoint.

### Misc

- `regex-partial.{h,cpp}` — streaming regex matcher (used by chat parsers).
- `unicode.{h,cpp}` — Unicode helpers (case fold, normalization, byte/codepoint round-tripping). Tokenizers also pull from `src/unicode-data.cpp`.
- `base64.hpp` — base64 encode/decode for inline images in chat messages.

## Why a separate helper library?

The split exists because:

- libllama wants to stay narrow and ABI-stable.
- Many tools share the same prompt-loop / sampler-chain code; not duplicating it is sane.
- Some functionality (JSON-schema, Jinja, HTTP) pulls in dependencies that not every consumer wants forced on them.

Result: `libllama` stays small and pure; `libcommon` is the convenient batteries-included companion.
