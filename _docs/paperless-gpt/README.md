# paperless-gpt — notes

Source: `repos-folder/paperless-gpt` (icereed). Go module `paperless-gpt`, Go 1.24 with toolchain 1.25. Distributed as a Docker image (`icereed/paperless-gpt` on Docker Hub and `ghcr.io/icereed/paperless-gpt` on GitHub Container Registry).

What it is: a sidecar service for [paperless-ngx](https://github.com/paperless-ngx/paperless-ngx) that pairs over the paperless-ngx REST API and adds two LLM-powered workflows:

1. **AI-enhanced OCR** — replaces or augments paperless-ngx's built-in OCR with a vision LLM (OpenAI, Ollama, Mistral, Anthropic, Google Gemini) or a managed OCR service (Azure Document Intelligence, Google Document AI, Docling, Mistral OCR).
2. **AI-generated metadata** — uses an LLM to generate titles, tags, correspondents, document types, created dates, and custom field values from document content, then writes them back to paperless-ngx.

Plus searchable-PDF generation (with transparent text layers over the original image), an ad-hoc multi-document analysis tool, a unified web UI for manual review, and tag-driven automatic processing.

Shape:

- A Go HTTP server (Gin) on port 8080. Backend Go code in the repo root: `main.go`, `app_http_handlers.go`, `app_llm.go`, `background.go`, `jobs.go`, `ocr.go`, `paperless.go`, `local_db.go`, etc.
- An OCR subsystem in [ocr/](../../repos-folder/paperless-gpt/ocr/) with a `Provider` interface and one implementation per backend.
- A React + Vite + TypeScript single-page frontend in [web-app/](../../repos-folder/paperless-gpt/web-app/), embedded into the binary via `embedded_assets.go`.
- A SQLite database (GORM) for modification history and per-page OCR results.
- Prompt templates as files in `default_prompts/` (built-in) and `prompts/` (user-mutable, persisted to a volume) using Go's `text/template` plus sprig functions.
- A background goroutine polls paperless-ngx every 10s for documents tagged with the auto-tag or auto-OCR-tag and processes them.

Two top-level tags drive the flow:

- `MANUAL_TAG` (default `paperless-gpt`) — surface documents in the web UI for manual review.
- `AUTO_TAG` (default `paperless-gpt-auto`) — process automatically without UI interaction.
- `AUTO_OCR_TAG` (default `paperless-gpt-ocr-auto`) — same for the OCR pipeline.
- `PDF_OCR_COMPLETE_TAG` (default `paperless-gpt-ocr-complete`) — added to documents after successful enhanced-PDF upload, prevents re-processing.

License: GPL-3.0 (project), MIT for some embedded dependencies. Maintained by icereed; supported by sponsors.
