# Servers (`mineru-api`, `mineru-router`, `mineru-{vllm,lmdeploy,openai}-server`)

MinerU's CLI is just an orchestration client. The real work happens behind
HTTP — `mineru-api` is the parsing server, optionally fronted by
`mineru-router` for multi-GPU load balancing, optionally talking to a
separate VLM inference server.

## `mineru-api` — the parsing server

Defined in `mineru/cli/fast_api.py`. FastAPI app exposing:

| Endpoint           | Method | Purpose                                                      |
|--------------------|--------|--------------------------------------------------------------|
| `POST /tasks`      | async  | Submit a parse job; returns `task_id`. Poll for status/result.|
| `POST /file_parse` | sync   | Old-style: upload + parse + return in one request. For back-compat with old plugins. |
| `GET /tasks/{id}`  | —      | Status snapshot (pending / processing / finished / failed).  |
| `GET /tasks/{id}/result` | — | Download result ZIP (md + JSON + images + visualizations).  |
| `GET /health`      | —      | Health check + capacity metadata (max_concurrent_requests, processing_window_size). |

Reads concurrency from the env / config (`MINERU_MAX_CONCURRENT_REQUESTS`,
or `mineru.json`). The CLI uses `/health` at startup to negotiate the
effective parallelism.

```bash
mineru-api --host 0.0.0.0 --port 8000
```

## `mineru-router` — multi-service router

Defined in `mineru/cli/router.py`. Same API as `mineru-api` but load-balances
requests across multiple `mineru-api` instances (e.g. one per GPU).

Typical multi-GPU setup:

```bash
# GPU 0
CUDA_VISIBLE_DEVICES=0 mineru-api --port 8001 &
# GPU 1
CUDA_VISIBLE_DEVICES=1 mineru-api --port 8002 &
# Router in front
mineru-router --backends http://localhost:8001 http://localhost:8002 --port 8000

# Client (CLI or your app) talks to the router
mineru -p doc.pdf -o out --api-url http://localhost:8000
```

Round-robin by default; falls over to the next backend on failure.

## VLM inference servers

These wrap a VLM engine (vLLM / LMDeploy) with sensible MinerU defaults so
you don't write the launch flags yourself. They serve the `MinerU2.5-Pro`
model on an OpenAI-compatible chat endpoint (`POST /v1/chat/completions`).

```bash
mineru-vllm-server     # spawns docker / direct vLLM with right model + args
mineru-lmdeploy-server # LMDeploy equivalent (Windows-friendly)
mineru-openai-server   # proxy to a remote OpenAI-compatible host
```

`mineru-api` (or `hybrid-http-client` / `vlm-http-client` CLI) then connects
to whichever URL you publish.

## Local auto-start

If you run the CLI without `--api-url`, `cli/client.py:LocalAPIServer`
spawns a `mineru-api` subprocess on a free port and waits for `/health` to
respond. Same code path as remote — no special "embedded" mode. The
process is torn down when the CLI run ends.

## Typical deployments

| Scale                          | Setup                                                            |
|--------------------------------|------------------------------------------------------------------|
| One-off, laptop                | CLI alone (auto-starts local mineru-api).                        |
| Single-GPU server, many users  | Persistent `mineru-api` behind a reverse proxy.                  |
| Multi-GPU box                  | One `mineru-api` per GPU + `mineru-router` in front.             |
| Multi-machine cluster          | `mineru-router` with backends across machines.                   |
| Remote VLM, local OCR          | `hybrid-http-client` + `mineru-vllm-server` on a separate box.   |
