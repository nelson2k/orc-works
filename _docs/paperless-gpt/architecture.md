# Architecture

Single Docker image, three layers:

```
+--------------------------+
|  React/Vite frontend     |  (web-app/, served on :8080)
|  - Document review UI    |
|  - Settings UI           |
|  - Prompt editor         |
+-------------+------------+
              |
              v
+--------------------------+
|  Go HTTP server (Gin)    |  (main.go, app_http_handlers.go)
|  - REST endpoints        |
|  - Job queue (jobs.go)   |
|  - Background poller     |
+-------+----------+-------+
        |          |
        v          v
+---------------+ +---------------------+
| LLM provider  | | OCR provider        |
| (langchaingo) | | (ocr/ package)      |
|               | |                     |
| openai /      | | llm /               |
| ollama /      | | google_docai /      |
| mistral /     | | azure /             |
| anthropic /   | | mistral_ocr /       |
| googleai      | | docling             |
+---------------+ +---------------------+
        |
        v
+--------------------------+
|  paperless-ngx REST API  |
|  (paperless.go)          |
+--------------------------+
```

## Key Go source files

| File | Role |
|------|------|
| [main.go](../../repos-folder/paperless-gpt/main.go) | Entry point, env-var parsing, App struct |
| [app_http_handlers.go](../../repos-folder/paperless-gpt/app_http_handlers.go) | REST endpoints (`/api/...`) |
| [app_llm.go](../../repos-folder/paperless-gpt/app_llm.go) | Title/tag/correspondent generation via LLM |
| [background.go](../../repos-folder/paperless-gpt/background.go) | Polls paperless-ngx for tagged docs |
| [jobs.go](../../repos-folder/paperless-gpt/jobs.go) | In-memory job store for tracking progress |
| [paperless.go](../../repos-folder/paperless-gpt/paperless.go) | paperless-ngx REST client |
| [ocr.go](../../repos-folder/paperless-gpt/ocr.go) | OCR pipeline (page splitting, hOCR assembly, PDF upload) |
| [ocr/provider.go](../../repos-folder/paperless-gpt/ocr/provider.go) | `Provider` interface + factory |
| [local_db.go](../../repos-folder/paperless-gpt/local_db.go) | SQLite for history / undo |

## Data flow for a single document

1. User adds `paperless-gpt` tag to a doc in paperless-ngx.
2. `background.go` polls `/api/documents/?tags__id__in=...` every N
   seconds, gets the doc ID.
3. `ocr.go::ProcessDocumentOCR` downloads the document (as images or
   PDF depending on `OCR_PROCESS_MODE`), pushes pages through the OCR
   provider, gets text (+ optional hOCR pages).
4. `app_llm.go` runs the title/tag/correspondent prompts against the
   OCR text.
5. Result lands in the web UI for review, or auto-applies via the
   paperless-ngx API depending on which trigger tag was used.
6. If hOCR is available *and* `CREATE_LOCAL_PDF` / `PDF_UPLOAD` is on,
   `pdfocr.ApplyOCR` (from `ocrchestra`) assembles a searchable PDF and
   either writes it locally or uploads it back to paperless-ngx.

## Dependencies worth knowing

- **langchaingo** — unified LLM client across OpenAI/Ollama/Anthropic/Mistral/GoogleAI.
- **go-fitz** (MuPDF) — PDF rasterisation to images.
- **pdfcpu** — PDF manipulation.
- **ocrchestra** (`gardar/ocrchestra`) — hOCR → searchable PDF assembly.
- **gorm + sqlite** — local DB for history and OCR-page cache.
