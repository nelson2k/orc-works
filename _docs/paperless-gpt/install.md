# Install

paperless-gpt ships as a single Docker image. You **also need
paperless-ngx running** — paperless-gpt is useless on its own.

## Minimal docker-compose

```yaml
services:
  paperless-ngx:
    image: ghcr.io/paperless-ngx/paperless-ngx:latest
    # ... your existing paperless-ngx config

  paperless-gpt:
    image: icereed/paperless-gpt:latest
    # Or: ghcr.io/icereed/paperless-gpt:latest
    environment:
      PAPERLESS_BASE_URL: "http://paperless-ngx:8000"
      PAPERLESS_API_TOKEN: "your_paperless_api_token"

      LLM_PROVIDER: "openai"
      LLM_MODEL: "gpt-4o"
      OPENAI_API_KEY: "sk-..."

      OCR_PROVIDER: "llm"
      VISION_LLM_PROVIDER: "openai"
      VISION_LLM_MODEL: "gpt-4o"

      OCR_LIMIT_PAGES: "0"          # 0 = no page limit
      LOG_LEVEL: "info"
    volumes:
      - ./prompts:/app/prompts      # persist prompt edits
    ports:
      - "8080:8080"
    depends_on:
      - paperless-ngx
```

## With Ollama (fully local, free)

```yaml
paperless-gpt:
  image: icereed/paperless-gpt:latest
  environment:
    PAPERLESS_BASE_URL: "http://paperless-ngx:8000"
    PAPERLESS_API_TOKEN: "..."

    LLM_PROVIDER: "ollama"
    LLM_MODEL: "qwen3:8b"
    OLLAMA_HOST: "http://host.docker.internal:11434"
    TOKEN_LIMIT: "2000"

    OCR_PROVIDER: "llm"
    VISION_LLM_PROVIDER: "ollama"
    VISION_LLM_MODEL: "minicpm-v"
```

Needs Ollama running on the host with both models pulled
(`ollama pull qwen3:8b minicpm-v`).

## With Google Document AI + searchable PDFs

```yaml
paperless-gpt:
  image: icereed/paperless-gpt:latest
  environment:
    PAPERLESS_BASE_URL: "http://paperless-ngx:8000"
    PAPERLESS_API_TOKEN: "..."

    LLM_PROVIDER: "openai"
    LLM_MODEL: "gpt-4o"
    OPENAI_API_KEY: "sk-..."

    OCR_PROVIDER: "google_docai"
    GOOGLE_PROJECT_ID: "your-project"
    GOOGLE_LOCATION: "us"
    GOOGLE_PROCESSOR_ID: "abc123"
    GOOGLE_APPLICATION_CREDENTIALS: "/app/credentials.json"

    CREATE_LOCAL_PDF: "true"
    LOCAL_PDF_PATH: "/app/pdf"
    PDF_UPLOAD: "true"
    PDF_COPY_METADATA: "true"
  volumes:
    - ./prompts:/app/prompts
    - ./pdf_out:/app/pdf
    - ${HOME}/.config/gcloud/application_default_credentials.json:/app/credentials.json
```

## Manual / from source

paperless-gpt is a single Go binary + static React assets
(see [Dockerfile](../../repos-folder/paperless-gpt/Dockerfile) for the
3-stage build). For a local dev build:

```bash
git clone https://github.com/icereed/paperless-gpt.git
cd paperless-gpt
docker build -t paperless-gpt .
docker run -p 8080:8080 -e PAPERLESS_BASE_URL=... -e PAPERLESS_API_TOKEN=... paperless-gpt
```

## Get the API token

In paperless-ngx admin → your user → API auth token. Paste into
`PAPERLESS_API_TOKEN`.
