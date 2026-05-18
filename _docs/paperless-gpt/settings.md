# Configuration

paperless-gpt is configured via environment variables (every setting), plus a `config/settings.json` file for a few server-side toggles managed through the UI, plus the `prompts/` directory of templates.

## Required env vars

| Var | Purpose |
|---|---|
| `PAPERLESS_BASE_URL` | URL of the paperless-ngx instance (e.g. `http://paperless-ngx:8000`) |
| `PAPERLESS_API_TOKEN` | API token issued in paperless-ngx admin |
| `LLM_PROVIDER` | Main LLM backend: `openai`, `ollama`, `googleai`, `mistral`, or `anthropic` |
| `LLM_MODEL` | Model name for the main LLM (e.g. `gpt-4o`, `qwen3:8b`, `claude-sonnet-4-5`) |

## paperless-ngx wiring

| Var | Default | Purpose |
|---|---|---|
| `PAPERLESS_PUBLIC_URL` | falls back to `PAPERLESS_BASE_URL` | Used in UI links to documents |
| `PAPERLESS_INSECURE_SKIP_VERIFY` | `false` | Skip TLS cert verification on the paperless-ngx connection |
| `PAPERLESS_GPT_CACHE_DIR` | unset | If set, downloaded document files are cached here |
| `MANUAL_TAG` | `paperless-gpt` | Surface documents for UI review |
| `AUTO_TAG` | `paperless-gpt-auto` | Process automatically (metadata) |
| `AUTO_OCR_TAG` | `paperless-gpt-ocr-auto` | Process automatically (OCR) |
| `PDF_OCR_COMPLETE_TAG` | `paperless-gpt-ocr-complete` | Marks docs that have finished OCR upload |

## LLM (main, used for titles / tags / etc.)

| Var | Default | Purpose |
|---|---|---|
| `LLM_LANGUAGE` | `English` | Injected into prompts as `{{.Language}}` |
| `OPENAI_API_KEY` | — | Required for `openai` |
| `OPENAI_API_TYPE` | — | Set to `azure` for Azure OpenAI |
| `OPENAI_BASE_URL` | — | Azure deployment URL or self-hosted OpenAI-compatible endpoint |
| `MISTRAL_API_KEY` | — | Required for `mistral` |
| `ANTHROPIC_API_KEY` | — | Required for `anthropic` |
| `GOOGLEAI_API_KEY` | — | Required for `googleai` |
| `GOOGLEAI_THINKING_BUDGET` | model default | int32; `0` disables Gemini thinking |
| `OLLAMA_HOST` | `http://127.0.0.1:11434` | Ollama server URL |
| `OLLAMA_CONTEXT_LENGTH` | model default | NumCtx (context window) |
| `LLM_REQUESTS_PER_MINUTE` | `120` | Rate limit |
| `LLM_MAX_RETRIES` | `3` | Retry budget on transient failures |
| `LLM_BACKOFF_MAX_WAIT` | `30s` | Max single backoff (Go duration string) |
| `TOKEN_LIMIT` | unset (no limit) | Max content tokens per prompt — useful for small local models |

## OCR — provider selection and tuning

| Var | Default | Purpose |
|---|---|---|
| `OCR_PROVIDER` | `llm` | One of `llm`, `azure`, `google_docai`, `mistral_ocr`, `docling` |
| `OCR_PROCESS_MODE` | `image` | `image`, `pdf`, `whole_pdf` (compatibility validated at startup) |
| `OCR_LIMIT_PAGES` | `5` | Cap pages processed per doc; `0` disables |
| `AUTO_OCR_TAG` | `paperless-gpt-ocr-auto` | Tag that triggers auto-OCR |
| `PDF_SKIP_EXISTING_OCR` | `false` | Skip PDFs that already have a text layer (pdf/whole_pdf modes only) |

LLM-OCR (vision model):

| Var | Default | Purpose |
|---|---|---|
| `VISION_LLM_PROVIDER` | — | `openai`, `ollama`, `mistral`, `anthropic`, `googleai` |
| `VISION_LLM_MODEL` | — | e.g. `gpt-4o`, `minicpm-v`, `claude-sonnet-4-5` |
| `VISION_LLM_REQUESTS_PER_MINUTE` | `120` | Rate limit |
| `VISION_LLM_MAX_RETRIES` | `3` | Retry budget |
| `VISION_LLM_BACKOFF_MAX_WAIT` | `30s` | Max backoff |
| `VISION_LLM_MAX_TOKENS` | unset | Output cap; reflected in `OcrLimitHit` |
| `VISION_LLM_TEMPERATURE` | unset | Sampling temperature — must be `1.0` for OpenAI GPT-5 |
| `OLLAMA_OCR_TOP_K` | unset | Ollama-only top-k for OCR |
| `OLLAMA_CONTEXT_LENGTH` | unset | NumCtx for the vision LLM |

Azure Document Intelligence:

| Var | Default | Purpose |
|---|---|---|
| `AZURE_DOCAI_ENDPOINT` | required | Resource endpoint URL |
| `AZURE_DOCAI_KEY` | required | API key |
| `AZURE_DOCAI_MODEL_ID` | `prebuilt-read` | Model ID |
| `AZURE_DOCAI_TIMEOUT_SECONDS` | `120` | HTTP timeout |
| `AZURE_DOCAI_OUTPUT_CONTENT_FORMAT` | `text` | `text` or `markdown` (markdown needs `prebuilt-layout`) |

Google Document AI:

| Var | Purpose |
|---|---|
| `GOOGLE_PROJECT_ID` | GCP project |
| `GOOGLE_LOCATION` | Region (`us`, `eu`) |
| `GOOGLE_PROCESSOR_ID` | Processor ID |
| `GOOGLE_APPLICATION_CREDENTIALS` | Path to mounted service-account JSON |

Docling:

| Var | Default | Purpose |
|---|---|---|
| `DOCLING_URL` | required | Docling server URL |
| `DOCLING_IMAGE_EXPORT_MODE` | `embedded` | `embedded` / `placeholder` |
| `DOCLING_OCR_PIPELINE` | `vlm` | `vlm` or `standard` |
| `DOCLING_OCR_ENGINE` | `easyocr` | Used only when pipeline is `standard` |

Mistral OCR:

| Var | Default | Purpose |
|---|---|---|
| `MISTRAL_API_KEY` | required | API key |
| `MISTRAL_MODEL` | `mistral-ocr-latest` | Model name |

## Enhanced OCR (hOCR + searchable PDF)

| Var | Default | Purpose |
|---|---|---|
| `CREATE_LOCAL_HOCR` | `false` | Write hOCR HTML to `LOCAL_HOCR_PATH` |
| `LOCAL_HOCR_PATH` | `/app/hocr` | Output dir for hOCR files |
| `CREATE_LOCAL_PDF` | `false` | Write searchable PDF to `LOCAL_PDF_PATH` |
| `LOCAL_PDF_PATH` | `/app/pdf` | Output dir for PDFs |
| `PDF_UPLOAD` | `false` | Upload the enhanced PDF back to paperless-ngx as a new document |
| `PDF_REPLACE` | `false` | **Destructive.** Delete the original after a successful upload |
| `PDF_COPY_METADATA` | `true` | Copy title / tags / correspondent / created date onto the upload |
| `PDF_OCR_TAGGING` | `true` | Add `PDF_OCR_COMPLETE_TAG` to the uploaded document |
| `PDF_OCR_COMPLETE_TAG` | `paperless-gpt-ocr-complete` | Tag used as the "OCR done" marker |

Note: hOCR and searchable-PDF generation only work when the OCR provider implements `HOCRCapable` — currently only Google Document AI.

## Auto-processing toggles

When a document is tagged with `AUTO_TAG`, the background poller runs the LLM and writes back metadata. These booleans gate which fields it generates:

| Var | Default |
|---|---|
| `AUTO_GENERATE_TITLE` | `true` |
| `AUTO_GENERATE_TAGS` | `true` |
| `AUTO_GENERATE_CORRESPONDENTS` | `true` |
| `AUTO_GENERATE_DOCUMENT_TYPE` | `true` (only assigns existing types) |
| `AUTO_GENERATE_CREATED_DATE` | `true` |
| `CREATE_NEW_TAGS` | `false` — let the LLM invent new tags |
| `CORRESPONDENT_BLACK_LIST` | empty | Comma-separated names to exclude from suggestions |

Custom-field generation is enabled separately via the UI (`config/settings.json`) and applies to the IDs the user selects.

## Image processing limits

Used when rendering pages to JPEGs in `image` mode. The renderer (via `gen2brain/go-fitz` + `disintegration/imaging`) progressively reduces DPI / size until the output fits.

| Var | Default | Purpose |
|---|---|---|
| `IMAGE_MAX_PIXEL_DIMENSION` | `10000` | Max pixels along either axis |
| `IMAGE_MAX_TOTAL_PIXELS` | `40000000` | Max width × height |
| `IMAGE_MAX_RENDER_DPI` | `600` | Initial render DPI |
| `IMAGE_MAX_FILE_BYTES` | `10485760` (10 MiB) | Max JPEG size; oversize gets recompressed/resized |

## Misc

| Var | Default | Purpose |
|---|---|---|
| `LOG_LEVEL` | `info` | `debug` / `info` / `warn` / `error` |
| `LISTEN_INTERFACE` | `:8080` | Address Gin binds to |

## `config/settings.json`

Persisted server-side settings managed through the UI's Settings page. Schema (from [types.go](../../repos-folder/paperless-gpt/types.go) and [settings.go](../../repos-folder/paperless-gpt/settings.go)):

```json
{
  "custom_fields_enable": false,
  "custom_fields_selected_ids": [],
  "custom_fields_write_mode": "append"
}
```

`custom_fields_write_mode` is `append` (default — only add fields that don't exist on the document), `update` (add and overwrite), or `replace` (delete all existing custom fields, replace with suggestions).

Mount `./config:/app/config` to persist across restarts.

## Prompt templates

Eight prompt files, all driven by Go `text/template` with sprig functions:

| File | Variables available |
|---|---|
| `title_prompt.tmpl` | `Language`, `Content`, `Title` |
| `tag_prompt.tmpl` | `Language`, `AvailableTags`, `OriginalTags`, `Title`, `Content` |
| `ocr_prompt.tmpl` | `Language` |
| `correspondent_prompt.tmpl` | `Language`, `AvailableCorrespondents`, `BlackList`, `Title`, `Content` |
| `created_date_prompt.tmpl` | `Language`, `Content` |
| `document_type_prompt.tmpl` | `Language`, `AvailableDocumentTypes`, `Title`, `Content` |
| `custom_field_prompt.tmpl` | `DocumentType`, `CustomFieldsXML`, `Title`, `CreatedDate`, `Content` |
| `adhoc-analysis_prompt.tmpl` | `Documents`, `Prompt` (whatever the UI provided) |

On startup, missing files in `prompts/` are seeded from `default_prompts/`. Edits made via the Settings page in the UI write back to `prompts/<name>.tmpl` and are reloaded immediately. Mount `./prompts:/app/prompts` to persist them.
