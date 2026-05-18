# Package layout

Contents of `repos-folder/paperless-gpt/`:

```
.
├── main.go                     Server entry point: env validation, deps wiring, Gin router, worker pool
├── app_http_handlers.go        Most /api/* handlers (documents, suggestions, prompts, settings, modifications)
├── app_http_handlers_test.go
├── app_llm.go                  LLM prompt rendering + suggestion generation
├── app_llm_googleai.go         Custom Gemini wrapper implementing langchaingo's llms.Model
├── app_llm_test.go
├── background.go               Background poller — auto-tag + auto-OCR loops
├── background_test.go
├── ocr.go                      App.ProcessDocumentOCR — orchestrates pages, hOCR, PDF, upload
├── ocr_test.go
├── jobs.go                     JobStore, jobQueue, worker pool, cancellation
├── llm_client.go               NewRateLimitedLLM — rate limit + retry wrapper for langchaingo
├── llm_client_test.go
├── paperless.go                PaperlessClient — REST client for paperless-ngx (~1500 lines)
├── paperless_test.go
├── local_db.go                 GORM init + modifications + ocr_pages tables
├── settings.go                 Load/save config/settings.json
├── tokens.go                   Token counting (tiktoken-go) for TOKEN_LIMIT enforcement
├── tokens_test.go
├── types.go                    Shared structs: Document, Settings, OCROptions, OCRResult, Job, etc.
├── version.go                  Version/commit/buildDate ldflags
├── main_test.go
├── embedded_assets.go          //go:embed of web-app/dist; serveEmbeddedFile helper
│
├── ocr/                        OCR provider implementations
│   ├── provider.go             Provider interface, Config struct, NewProvider dispatcher
│   ├── llm_provider.go         Vision-LLM provider (OpenAI/Ollama/Mistral/Anthropic/GoogleAI)
│   ├── google_ai_client.go     Custom Gemini client used both as main LLM and as OCR provider
│   ├── google_docai_provider.go    Google Document AI (the only HOCRCapable provider)
│   ├── azure_provider.go       Azure Document Intelligence (REST)
│   ├── azure_types.go          Azure response types
│   ├── docling_provider.go     Self-hosted docling server client
│   ├── mistral_provider.go     Mistral OCR API (mistral-ocr-latest)
│   ├── anthropic_provider_test.go
│   ├── utils.go                Shared helpers: image decoding, mime sniffing
│   └── *_test.go
│
├── default_prompts/            Baseline prompt templates (copied to prompts/ on first run)
│   ├── adhoc-analysis_prompt.tmpl
│   ├── correspondent_prompt.tmpl
│   ├── created_date_prompt.tmpl
│   ├── custom_field_prompt.tmpl
│   ├── document_type_prompt.tmpl
│   ├── ocr_prompt.tmpl
│   ├── tag_prompt.tmpl
│   └── title_prompt.tmpl
│
├── web-app/                    React + Vite + TS frontend
│   ├── src/                    Components and pages
│   ├── e2e/                    Playwright specs
│   ├── public/                 Static assets
│   ├── package.json, vite.config.ts, tailwind.config.js, postcss.config.js,
│   ├── tsconfig*.json, eslint.config.js
│   └── docker-compose.test.yml
│
├── docs/
│   └── mistral_llm.md          Mistral-specific setup notes
│
├── cline_docs/                 Internal design / context notes (kept for the maintainer's AI assistant)
│   ├── activeContext.md
│   ├── productContext.md
│   ├── progress.md
│   ├── systemPatterns.md
│   └── techContext.md
│
├── tests/                      Cross-cutting Go test fixtures
├── demo/                       Demo media (screenshots, mp4)
├── go.mod, go.sum              Go module manifest
├── package-lock.json           Top-level npm lock (build orchestration)
├── Dockerfile                  Multi-stage: build web-app → build Go binary → minimal final
├── docker-compose.yml          Example compose with paperless-ngx
├── build-and-run.sh            Local dev helper
├── docker-build-and-push.sh    Multi-arch Docker publish
├── renovate.json               Renovate bot config
├── README.md, CODE_OF_CONDUCT.md, CONTRIBUTING.md, LICENSE
├── E2E_TEST_FIXES.md           Known E2E quirks
└── paperless-gpt-screenshot.png
```

## Module identity

`go.mod` line 1 is `module paperless-gpt`. Go 1.24.4 minimum, toolchain pinned to 1.25.5.

Key direct dependencies (from `go.mod`):

| Module | Purpose |
|---|---|
| `cloud.google.com/go/documentai` | Google Document AI client |
| `github.com/Masterminds/sprig/v3` | Template helpers |
| `github.com/disintegration/imaging` | Image resize / encode |
| `github.com/gabriel-vasile/mimetype` | MIME sniffing |
| `github.com/gardar/ocrchestra` | hOCR struct + searchable-PDF generation (the `hocr` and `pdfocr` packages) |
| `github.com/gen2brain/go-fitz` | libmupdf bindings for rendering PDF pages to images |
| `github.com/gin-gonic/gin` | HTTP framework |
| `github.com/google/uuid` | Job ID generation |
| `github.com/hashicorp/go-retryablehttp` | HTTP retry semantics |
| `github.com/pdfcpu/pdfcpu` | PDF splitting / merging |
| `github.com/sirupsen/logrus` | Logging |
| `github.com/tmc/langchaingo` | LLM abstraction (OpenAI / Ollama / Mistral / Anthropic) |
| `golang.org/x/sync` | errgroup |
| `golang.org/x/time` | Rate limiter |
| `google.golang.org/api` | GCP transport |
| `google.golang.org/genai` | Gemini SDK |
| `gorm.io/gorm` + `gorm.io/driver/sqlite` | Local database |

Indirect dependencies include a sqlite3 cgo driver (`github.com/mattn/go-sqlite3`), tiktoken-go for token counting, and several QUIC / OpenTelemetry transitive deps.

## Entry points

Single binary, single subcommand: it just runs. The binary built from `main.go` reads env, starts HTTP, blocks. There are no CLI flags.
