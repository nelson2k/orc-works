# CLI entry points

All console scripts are declared under `[project.scripts]` in [pyproject.toml](../../repos-folder/MinerU/pyproject.toml). Their handlers live under [mineru/cli/](../../repos-folder/MinerU/mineru/cli/).

| Script | Module:func | Role |
|---|---|---|
| `mineru` | [client.py](../../repos-folder/MinerU/mineru/cli/client.py) `:main` | Primary CLI — orchestrates a parse against `mineru-api` |
| `mineru-api` | [fast_api.py](../../repos-folder/MinerU/mineru/cli/fast_api.py) `:main` | FastAPI server |
| `mineru-router` | [router.py](../../repos-folder/MinerU/mineru/cli/router.py) `:main` | Multi-server load balancer |
| `mineru-gradio` | [gradio_app.py](../../repos-folder/MinerU/mineru/cli/gradio_app.py) `:main` | Browser UI |
| `mineru-models-download` | [models_download.py](../../repos-folder/MinerU/mineru/cli/models_download.py) `:download_models` | Model fetcher |
| `mineru-vllm-server` | [vlm_server.py](../../repos-folder/MinerU/mineru/cli/vlm_server.py) `:vllm_server` | vLLM-backed VLM server |
| `mineru-lmdeploy-server` | `:lmdeploy_server` | LMDeploy-backed VLM server |
| `mineru-openai-server` | `:openai_server` | OpenAI-compatible adapter |

## `mineru` (the orchestrating client)

```text
mineru -p PATH -o OUTPUT [options]
```

Inputs may be a single file or a directory; supported suffixes: `pdf`, `png`, `jpeg`, `jp2`, `webp`, `gif`, `bmp`, `jpg`, `tiff`, `docx`, `pptx`, `xlsx`.

| Flag | Default | Notes |
|---|---|---|
| `-p / --path` | required | File or directory of inputs |
| `-o / --output` | required | Output directory |
| `--api-url` | none | If omitted, spawns a temporary local `mineru-api` |
| `-m / --method` | `auto` | `auto` \| `txt` \| `ocr` — pipeline/hybrid only |
| `-b / --backend` | `hybrid-auto-engine` | `pipeline` \| `vlm-auto-engine` \| `vlm-http-client` \| `hybrid-auto-engine` \| `hybrid-http-client` |
| `-l / --lang` | `ch` | One of: ch, ch_server, ch_lite, en, korean, japan, chinese_cht, ta, te, ka, th, el, latin, arabic, east_slavic, cyrillic, devanagari |
| `-u / --url` | none | Required for `*-http-client` backends — points at the VLM server |
| `-s / --start` | 0 | First PDF page (0-indexed) |
| `-e / --end` | none | Last PDF page (inclusive) |
| `-f / --formula` | True | Enable formula parsing |
| `-t / --table` | True | Enable table parsing |
| `--image-analysis` | True | Enable image/chart analysis (VLM/hybrid) |

What it actually does (`run_orchestrated_cli` in client.py:836):

1. Validates the backend and (for `hybrid-*`) ensures `torch` is importable.
2. Walks the input path, classifies each file by suffix, probes PDF page counts.
3. **Plans tasks** — for the pipeline backend, packs short documents into bins up to `MINERU_PROCESSING_WINDOW_SIZE` pages; for other backends, one document per task.
4. Starts a local `mineru-api` if no `--api-url` was given.
5. Submits each task asynchronously, waits for the server's `/tasks/{id}` to terminalize, downloads the result zip, extracts it.
6. Launches a one-worker `ProcessPoolExecutor` for visualization (`layout.pdf` / `span.pdf` overlays).
7. Reports per-task failures at the end.

The renderer displays a live status line with a moving runner bar via `LiveTaskStatusRenderer` when an external `--api-url` is in use and stderr is a TTY.

## `mineru-api` (the parse server)

`mineru-api` boots a FastAPI app and exposes:

- `POST /file_parse` — synchronous parse (waits server-side for completion)
- `POST /tasks` — submit an async task, returns task id and polling URLs
- `GET /tasks/{task_id}` — task status
- `GET /tasks/{task_id}/result` — fetch result zip when ready
- `GET /health` — liveness + capacity (`max_concurrent_requests`, `processing_window_size`, `protocol_version`)

CLI flags (parsed in `fast_api.py`): host, port, output root, max-concurrent-requests, processing-window-size, retention, preloads.

## `mineru-router`

Spreads tasks across multiple `mineru-api` instances. Same path schema as `mineru-api` (`/file_parse`, `/tasks`, `/tasks/{id}`, `/tasks/{id}/result`, `/health`). The router round-robins / load-balances using the per-server `max_concurrent_requests` reported by `/health`.

## VLM servers

`mineru-vllm-server` / `mineru-lmdeploy-server` wrap a chosen inference engine and expose an OpenAI-compatible chat completions endpoint that the VLM and hybrid backends use as their VLM source. `mineru-openai-server` is a thin pass-through to a remote OpenAI-compatible server.

When `--backend vlm-http-client` or `hybrid-http-client` is used, point `-u/--url` at one of these servers.

## `mineru-gradio`

Launches a Gradio app (with `gradio-pdf` viewer) that wraps the same parse pipeline for interactive use.

## `mineru-models-download`

Downloads model files for one or both backends from HuggingFace or ModelScope. Writes the chosen cache directory into the `models-dir` section of `~/mineru.json` so subsequent runs don't re-download.
