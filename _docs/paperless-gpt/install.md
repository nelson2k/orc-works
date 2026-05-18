# Installation

There's no PyPI / Go-get install — paperless-gpt ships as a Docker image. Two registries serve the same image: `icereed/paperless-gpt:latest` (Docker Hub) and `ghcr.io/icereed/paperless-gpt:latest` (GHCR).

## Prerequisites

- Docker on the host.
- A running `paperless-ngx` instance with an API token (generate one in the paperless-ngx admin).
- Access to at least one LLM provider:
  - OpenAI (API key — supports `gpt-4o`, `gpt-3.5-turbo`, etc., plus Azure OpenAI deployments)
  - Ollama (local server running models like `qwen3:8b`, `minicpm-v`)
  - Mistral (API key)
  - Anthropic (API key for Claude)
  - Google Gemini (`googleai`, API key)

## Compose snippet

The README example wires paperless-gpt alongside an existing paperless-ngx service:

```yaml
services:
  paperless-gpt:
    image: icereed/paperless-gpt:latest
    environment:
      PAPERLESS_BASE_URL: "http://paperless-ngx:8000"
      PAPERLESS_API_TOKEN: "your_paperless_api_token"
      LLM_PROVIDER: "openai"
      LLM_MODEL: "gpt-4o"
      OPENAI_API_KEY: "your_openai_api_key"
      OCR_PROVIDER: "llm"
      VISION_LLM_PROVIDER: "ollama"
      VISION_LLM_MODEL: "minicpm-v"
      OLLAMA_HOST: "http://host.docker.internal:11434"
      OCR_PROCESS_MODE: "image"
      AUTO_OCR_TAG: "paperless-gpt-ocr-auto"
      OCR_LIMIT_PAGES: "5"
      LOG_LEVEL: "info"
    volumes:
      - ./prompts:/app/prompts     # required to persist custom prompts
      - ./hocr:/app/hocr           # optional, only with CREATE_LOCAL_HOCR=true
      - ./pdf:/app/pdf             # optional, only with CREATE_LOCAL_PDF=true
    ports:
      - "8080:8080"
    depends_on:
      - paperless-ngx
```

`PAPERLESS_BASE_URL` and `PAPERLESS_API_TOKEN` are the only strictly required vars; everything else has defaults or is conditional.

## Volumes

| Path inside container | Purpose | Required when |
|---|---|---|
| `/app/prompts` | User-edited prompt templates; copied from `default_prompts/` on first run | Always (otherwise edits are lost on restart) |
| `/app/config` | `settings.json` — UI-managed server-side settings | Always (created at startup if absent) |
| `/app/hocr` | hOCR HTML output | `CREATE_LOCAL_HOCR=true` |
| `/app/pdf` | Enhanced searchable PDFs | `CREATE_LOCAL_PDF=true` |
| `/app/credentials.json` | Google service-account key | OCR_PROVIDER=google_docai |

For Google Document AI, mount the gcloud credentials file:

```yaml
volumes:
  - ${HOME}/.config/gcloud/application_default_credentials.json:/app/credentials.json
environment:
  GOOGLE_APPLICATION_CREDENTIALS: "/app/credentials.json"
```

## Building from source

The project is a standard Go module. From `repos-folder/paperless-gpt/`:

```
docker build -t paperless-gpt .
```

The Dockerfile builds the React frontend first (`web-app/dist`), embeds it into the Go binary via `embedded_assets.go`, then produces a small final image. A convenience script `build-and-run.sh` is in the repo root; `docker-build-and-push.sh` does the multi-arch publish.

Native build outside Docker requires Go 1.24+, Node.js / npm for the frontend (`web-app/package.json`), and `cgo`-capable C toolchain (the binary links against the sqlite3 driver `github.com/mattn/go-sqlite3`).
