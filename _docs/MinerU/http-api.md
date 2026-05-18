# HTTP API

The `mineru-api` server in [mineru/cli/fast_api.py](../../repos-folder/MinerU/mineru/cli/fast_api.py) is what the `mineru` CLI talks to under the hood. You can also call it directly.

Default protocol version: declared in `cli/api_protocol.py`. Default concurrency: 3 (`MINERU_API_MAX_CONCURRENT_REQUESTS`). Default processing window: 64 pages (`MINERU_PROCESSING_WINDOW_SIZE`).

## Endpoints

| Method | Path | Purpose |
|---|---|---|
| `GET`  | `/health` | Liveness + server capabilities |
| `POST` | `/file_parse` | **Synchronous** parse; waits server-side for completion and returns the result |
| `POST` | `/tasks` | **Async** submit; returns task id + polling URLs |
| `GET`  | `/tasks/{task_id}` | Task status |
| `GET`  | `/tasks/{task_id}/result` | Task result zip (202 until ready) |

Custom headers reused across endpoints:

- `X-MinerU-Task-Id`
- `X-MinerU-Task-Status`
- `X-MinerU-Task-Status-Url`
- `X-MinerU-Task-Result-Url`

## `GET /health`

Returns JSON with:

- `protocol_version` — for client/server compatibility checks
- `max_concurrent_requests` — server-side capacity
- `processing_window_size` — pages bundled per pipeline task
- `base_url` — what the server believes is its public URL
- More fields validated by `cli/api_client.py:validate_server_health_payload`.

## `POST /file_parse` and `POST /tasks`

Both accept the same `multipart/form-data` payload, defined in `cli/api_protocol.py` and bound through `parse_request_form` in fast_api.py. Files are uploaded under the `files` field; metadata fields:

| Field | Type | Notes |
|---|---|---|
| `lang_list` | repeated string | One language per uploaded file (or single) |
| `backend` | string | Same values as the CLI `--backend` flag |
| `parse_method` | string | `auto` / `txt` / `ocr` |
| `formula_enable` | bool | |
| `table_enable` | bool | |
| `image_analysis` | bool | |
| `server_url` | string | Required for `*-http-client` backends |
| `start_page_id` | int | 0-indexed |
| `end_page_id` | int / null | inclusive |
| `return_md` | bool | Include `<stem>.md` in result |
| `return_middle_json` | bool | Include `<stem>_middle.json` |
| `return_model_output` | bool | Include `<stem>_model.json` |
| `return_content_list` | bool | Include `<stem>_content_list.json` |
| `return_images` | bool | Include extracted images |
| `response_format_zip` | bool | When true, results returned as zip; when false, as inline JSON |
| `return_original_file` | bool | Include the source document copy |

`POST /file_parse` blocks until the task terminalizes, then either returns the zip or returns a JSON failure body with status 409 if the task failed (or 503 if the manager became unavailable).

`POST /tasks` returns 202 with the task id and the relative URLs to the status/result endpoints.

## Task lifecycle

States: `pending` → `processing` → `completed` | `failed`.

In-memory task manager (`TaskManager`) keeps tasks for `DEFAULT_TASK_RETENTION_SECONDS` (24 h) before cleanup, sweeping at `DEFAULT_TASK_CLEANUP_INTERVAL_SECONDS` (5 min). Closing `mineru-api` stops in-flight tasks and signals `wait_for_terminal_state` callers via `TaskWaitAbortedError`.

## Public-bind safety

When the server binds to a non-loopback host *and* `MINERU_API_PUBLIC_BIND_EXPOSED` is not truthy, requests targeting `*-http-client` backends are rejected — the design assumes an internal-only VLM HTTP client unless explicitly opted in. `MINERU_API_ALLOW_PUBLIC_HTTP_CLIENT` provides the corresponding override. Implementation lives in `cli/public_http_client_policy.py`.

## Router protocol

[router.py](../../repos-folder/MinerU/mineru/cli/router.py) hosts the same `/tasks`, `/tasks/{id}`, `/tasks/{id}/result`, `/file_parse`, and `/health` paths. Submitted tasks are routed to one of the configured upstream `mineru-api` instances based on observed `max_concurrent_requests` headroom. The protocol is wire-compatible with the single-server API.

## OpenAPI docs

`MINERU_API_ENABLE_FASTAPI_DOCS=1` exposes `/docs` and `/openapi.json`. Off by default in production.
