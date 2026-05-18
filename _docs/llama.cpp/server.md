# llama-server

`tools/server/` is a pure C/C++ HTTP server that exposes libllama as a REST API plus a bundled web UI. Built on `cpp-httplib` + `nlohmann::json`.

## Binary

```
build/bin/llama-server
```

Built by default unless `-DLLAMA_BUILD_SERVER=OFF` is passed to CMake.

## Quick start

```bash
# Load a local model and listen on http://localhost:8080
llama-server -m model.gguf -ngl 99

# Or pull from Hugging Face
llama-server -hf ggml-org/gemma-3-1b-it-GGUF

# Or from Docker Hub
llama-server -dr gemma3:latest

# Pick a port and host
llama-server -m model.gguf --host 0.0.0.0 --port 8000

# Multiple parallel sequences for many concurrent users
llama-server -m model.gguf -np 8 -c 32768

# Multimodal
llama-server -m text.gguf --mmproj projector.gguf
```

## REST endpoints

### OpenAI-compatible

| Method + path | Compatible with | Notes |
|---|---|---|
| `POST /v1/chat/completions` | OpenAI chat | Streaming via SSE if `"stream": true`. Tool calling and JSON schema both supported. |
| `POST /v1/completions` | OpenAI completions | Legacy text-only completion. |
| `POST /v1/embeddings` | OpenAI embeddings | Requires an embedding model. |
| `POST /v1/responses` | OpenAI responses | Including assistant-prefill (you can begin the assistant message and the model continues). |
| `POST /v1/rerank` | (custom) | Reranker for retrieval; needs a reranker model. |
| `GET /v1/models` | OpenAI | Lists the currently loaded model. |

### Anthropic-compatible

| Method + path | Notes |
|---|---|
| `POST /v1/messages` | Same shape as Anthropic's Messages API. |

### Native endpoints

| Method + path | Purpose |
|---|---|
| `GET /` | The bundled WebUI (Svelte SPA). |
| `GET /health` | Liveness — `{"status":"ok"}` once a model is loaded. |
| `GET /metrics` | Prometheus metrics (rate, latency, tok/s, slot utilization). |
| `GET /props` | Live model / context / sampler properties. |
| `POST /props` | Mutate sampler defaults at runtime. |
| `GET /slots` | Per-slot state (id, prompt, tokens, idle). |
| `POST /tokenize` | Text → token ids. |
| `POST /detokenize` | Token ids → text. |
| `POST /apply-template` | Apply the model's chat template to a list of messages. |
| `POST /infill` | FIM / fill-in-middle. |
| `POST /completion` | The native completion route (richer than `/v1/completions`). |
| `POST /lora-adapters` / `GET /lora-adapters` | Hot-swap LoRA scales. |

## Continuous batching ("slots")

`-np N` (or `--parallel N`) creates **N slots**. Every incoming request is dispatched to a free slot. While slots are decoding, libllama merges them into a single batch each forward pass — this is "continuous batching". One model in memory, many users in parallel, near-linear throughput scaling until you saturate the GPU.

The total context (`-c`) is divided across slots, so `-c 32768 -np 8` means each slot gets 4 k tokens.

`GET /slots` shows live slot state. `POST /slots/{id}?action=restore|save|erase` manages slot KV state for stateful prompts.

## Speculative decoding

Run a small draft model alongside the main model:

```bash
llama-server \
  -m big.gguf \
  -md small-draft.gguf \
  --draft-max 5 --draft-min 0
```

Speedup is most noticeable on tasks where the draft model "guesses" the main model's output well.

## Schema-constrained JSON

Pass `"response_format": {"type": "json_schema", "json_schema": {...}}` in a request, or the legacy `"json_schema": {...}` field, and the server compiles it to GBNF and enforces it during sampling. The full converter is `common/json-schema-to-grammar.cpp`. There's also `llguidance` mode (`-DLLAMA_LLGUIDANCE=ON`) for more sophisticated guided decoding.

## Function calling / tool use

`tools/server/` has dedicated parsers for tool-call output. The server understands many models' native tool-call formats (Hermes 2 Pro, Functionary, Llama 3.x, Mistral, Qwen, DeepSeek, Granite, etc.) and presents them through OpenAI-shaped `"tool_calls"` in the response. Auto-detection is best-effort and configurable via `--chat-template-format` and `--reasoning-format`.

## Multimodal

`--mmproj path/to/projector.gguf` (or `--mmproj-url` / `--hf-repo` with `--no-mmproj` to disable auto-fetch). Once loaded, images can be embedded inline in OpenAI-style messages:

```json
{
  "role": "user",
  "content": [
    {"type": "text", "text": "What's in this image?"},
    {"type": "image_url", "image_url": {"url": "data:image/png;base64,..."}}
  ]
}
```

Audio inputs work the same way via `audio_url`.

## WebUI

`tools/ui/` is a Svelte+Vite single-page app. The build pipeline produces a single static bundle that is embedded into the server binary (via `embedded_assets.cpp` generated at build time). On startup `llama-server` serves `/` from the embedded bundle. There is no need to run a separate frontend dev server in production.

## Auth / CORS / TLS

- API key: `--api-key KEY` (or `--api-key-file`). Clients must send `Authorization: Bearer KEY`.
- CORS: `--cors-allow-origin` (repeatable) and `--cors-allow-headers`.
- TLS: `--ssl-cert-file` + `--ssl-key-file`. Requires building with OpenSSL.

## Operational flags

| Flag | Purpose |
|---|---|
| `--port`, `--host` | Bind address. |
| `-t`, `--threads` | Worker threads. |
| `-np`, `--parallel` | Number of concurrent decoding slots. |
| `-c`, `--ctx-size` | Total context; divided across slots. |
| `--cont-batching` | Already on by default — continuous batching. |
| `--chat-template` | Override the chat template (use a known name or a Jinja string). |
| `--slot-save-path` | Where to persist slot KV state on `POST /slots/N?action=save`. |
| `--metrics` | Enable `/metrics` Prometheus endpoint. |
| `--no-webui` | Don't serve `/`. |
| `--no-warmup` | Skip the warm-up decode at startup. |
| `--props` | Allow runtime mutation of sampler defaults. |
| `-md` | Draft model for speculative decoding. |
| `--mmproj`, `--mmproj-url` | Multimodal projector. |
| `--lora-init-without-apply` | Load LoRA adapters but don't enable until requested. |
| `--cache-reuse N` | Try to reuse KV cache across requests when prefixes match (≥ N tokens). |

## Tests and benches

- `tools/server/tests/` — behave + pytest integration tests.
- `tools/server/bench/` — Python load-testing script (uses k6).
- `scripts/server-bench.py`, `scripts/server-test-*.py` — function-calling, parallel-tool-call, structured-output, model-correctness probes.
- `scripts/tool_bench.py` — function-calling accuracy benchmark across many models.
