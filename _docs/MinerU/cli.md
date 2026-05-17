# CLI (`mineru`)

`mineru` is the **orchestration client**. Defined in
`mineru/cli/client.py`. If you don't pass `--api-url`, it boots a temporary
local `mineru-api` server in-process and talks to it — same code path as
remote, no shortcuts.

```bash
mineru -p <INPUT> -o <OUTPUT_DIR> [options]
```

`INPUT` can be a single file or a directory (every supported file in it).

## Core options

| Flag                       | Default               | Notes                                                              |
|----------------------------|-----------------------|--------------------------------------------------------------------|
| `-p / --path`              | required              | File or directory.                                                  |
| `-o / --output`            | required              | Output directory.                                                   |
| `--api-url URL`            | (auto-start local)    | Hit an existing `mineru-api` or `mineru-router`.                    |
| `-b / --backend`           | `hybrid-auto-engine`  | `pipeline` / `vlm-auto-engine` / `vlm-http-client` / `hybrid-auto-engine` / `hybrid-http-client`. See [backends.md](backends.md). |
| `-m / --method`            | `auto`                | `auto`/`txt`/`ocr` — picks digital vs OCR per page (pipeline/hybrid only). |
| `-l / --lang`              | `ch`                  | OCR language hint (see below).                                      |
| `-u / --url`               | none                  | VLM server URL when backend is `*-http-client`.                     |
| `-s / --start`             | `0`                   | Start page (inclusive, 0-based).                                    |
| `-e / --end`               | none                  | End page (inclusive, 0-based).                                      |
| `-f / --formula`           | `True`                | Enable math-formula parsing.                                        |
| `-t / --table`             | `True`                | Enable table parsing.                                               |
| `--image-analysis`         | `True`                | VLM/hybrid: enable image+chart analysis.                            |

## Languages

`ch ch_server ch_lite en korean japan chinese_cht ta te ka th el latin
arabic east_slavic cyrillic devanagari`

These are *PaddleOCR* language packs, used only by pipeline / hybrid (VLM
backends are multilingual via the VLM itself).

## Examples

```bash
# Default: hybrid backend, local auto-engine, all pages, all features
mineru -p doc.pdf -o ./out

# Pipeline backend, English OCR, page range 0-9
mineru -p doc.pdf -o ./out -b pipeline -l en -s 0 -e 9

# VLM via a remote vLLM server
mineru -p doc.pdf -o ./out \
  -b vlm-http-client \
  -u http://192.168.1.10:30000

# Whole folder, no formula/table
mineru -p ./docs -o ./out -f False -t False

# Connect to a long-running mineru-api instead of auto-starting one
mineru -p doc.pdf -o ./out --api-url http://localhost:8000
```

## Live status (TTY only)

When `--api-url` is set and stderr is a TTY, the CLI renders a rolling
status line per task (`[==>     ] status=processing | task_id=…`). Auto-mode
falls back to plain log lines.

## All other CLIs

| Command                  | Purpose                                                              |
|--------------------------|----------------------------------------------------------------------|
| `mineru-api`             | Start the FastAPI parsing server (`POST /tasks`, `POST /file_parse`).|
| `mineru-router`          | Multi-service router for multi-GPU deployments. Same API as mineru-api. |
| `mineru-gradio`          | Web UI.                                                              |
| `mineru-vllm-server`     | Spawn a vLLM server pre-configured for the MinerU VLM.               |
| `mineru-lmdeploy-server` | LMDeploy variant.                                                    |
| `mineru-openai-server`   | OpenAI-compatible bridge (proxy mode).                               |
| `mineru-models-download` | Pre-fetch pipeline + VLM model weights.                              |

See [server.md](server.md) for the server-side CLIs.
