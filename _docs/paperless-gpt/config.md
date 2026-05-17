# Configuration (env vars)

Full table is in the upstream
[README.md](../../repos-folder/paperless-gpt/README.md). This page
groups the ones you actually need by scenario.

## Mandatory

| Var | Example |
|-----|---------|
| `PAPERLESS_BASE_URL` | `http://paperless-ngx:8000` |
| `PAPERLESS_API_TOKEN` | from paperless-ngx admin |
| `LLM_PROVIDER` | `openai` / `ollama` / `mistral` / `anthropic` / `googleai` |
| `LLM_MODEL` | `gpt-4o` / `qwen3:8b` / `claude-sonnet-4-5` / ... |
| (provider key) | `OPENAI_API_KEY` / `ANTHROPIC_API_KEY` / etc. |

See [llm_providers.md](llm_providers.md) for per-provider keys.

## Common optional

| Var | Default | Meaning |
|-----|---------|---------|
| `PAPERLESS_PUBLIC_URL` | — | External URL if different from internal |
| `MANUAL_TAG` | `paperless-gpt` | Trigger tag for manual review |
| `AUTO_TAG` | `paperless-gpt-auto` | Trigger tag for auto-apply |
| `AUTO_OCR_TAG` | `paperless-gpt-ocr-auto` | Trigger tag for auto-OCR |
| `LLM_LANGUAGE` | `English` | Hint passed to prompts |
| `CREATE_NEW_TAGS` | `false` | Allow LLM to invent tags |
| `LOG_LEVEL` | `info` | `debug` / `info` / `warn` / `error` |
| `LISTEN_INTERFACE` | `8080` | HTTP port |

## OCR

| Var | Default | Meaning |
|-----|---------|---------|
| `OCR_PROVIDER` | `llm` | `llm` / `azure` / `google_docai` / `mistral_ocr` / `docling` |
| `OCR_PROCESS_MODE` | `image` | `image` / `pdf` / `whole_pdf` |
| `OCR_LIMIT_PAGES` | `5` | `0` = no limit |
| `VISION_LLM_PROVIDER` | — | Required if `OCR_PROVIDER=llm` |
| `VISION_LLM_MODEL` | — | Required if `OCR_PROVIDER=llm` |
| `PDF_SKIP_EXISTING_OCR` | `false` | Skip docs that already have an OCR layer |

Provider-specific OCR vars (Azure, Google, Docling, Mistral) — see
[ocr_providers.md](ocr_providers.md).

## Image rendering caps (for `image` mode)

| Var | Default |
|-----|---------|
| `IMAGE_MAX_PIXEL_DIMENSION` | `10000` |
| `IMAGE_MAX_TOTAL_PIXELS` | `40000000` |
| `IMAGE_MAX_RENDER_DPI` | `600` |
| `IMAGE_MAX_FILE_BYTES` | `10485760` (10 MB) |

## Enhanced PDF (Google DocAI only)

See [enhanced_pdf.md](enhanced_pdf.md).

| Var | Default |
|-----|---------|
| `CREATE_LOCAL_HOCR` | `false` |
| `LOCAL_HOCR_PATH` | `/app/hocr` |
| `CREATE_LOCAL_PDF` | `false` |
| `LOCAL_PDF_PATH` | `/app/pdf` |
| `PDF_UPLOAD` | `false` |
| `PDF_COPY_METADATA` | `true` |
| `PDF_OCR_TAGGING` | `true` |
| `PDF_OCR_COMPLETE_TAG` | `paperless-gpt-ocr-complete` |
| `PDF_REPLACE` | `false` ⚠ destructive |

## Throttling

| Var | Default |
|-----|---------|
| `LLM_REQUESTS_PER_MINUTE` | `120` |
| `LLM_MAX_RETRIES` | `3` |
| `LLM_BACKOFF_MAX_WAIT` | `30s` |
| `VISION_LLM_REQUESTS_PER_MINUTE` | `120` |
| `VISION_LLM_MAX_RETRIES` | `3` |
| `VISION_LLM_BACKOFF_MAX_WAIT` | `30s` |
| `TOKEN_LIMIT` | — | `0` = unlimited |
| `OLLAMA_CONTEXT_LENGTH` | — | NumCtx for Ollama |
