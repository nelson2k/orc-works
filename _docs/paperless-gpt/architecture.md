# Architecture

paperless-gpt is a single Go binary that exposes an HTTP server, a background poller, and an embedded React UI. It talks to paperless-ngx over HTTP and to one or more LLM/OCR backends over HTTP.

## High-level shape

```
+--------------------+        +-------------------------+        +----------------+
|   paperless-ngx    | <----> |     paperless-gpt       | <----> |  LLM / OCR     |
|  (REST API, port   |  HTTP  |  - Gin HTTP server      |  HTTP  |  providers     |
|   8000)            |        |  - Background poller    |        |  (OpenAI,      |
|                    |        |  - SQLite (modifications|        |   Ollama,      |
+--------------------+        |    + per-page OCR)      |        |   Mistral,     |
                              |  - React UI (embedded)  |        |   Anthropic,   |
                              +-------------------------+        |   GoogleAI,    |
                                                                 |   Azure DocAI, |
                                                                 |   Google DocAI,|
                                                                 |   Docling,     |
                                                                 |   Mistral OCR) |
                                                                 +----------------+
```

## Process startup ([main.go](../../repos-folder/paperless-gpt/main.go))

1. `validateOrDefaultEnvVars()` — checks required env vars, fails fast on missing `PAPERLESS_BASE_URL` / `PAPERLESS_API_TOKEN` / `LLM_PROVIDER` / `LLM_MODEL`, and applies tag defaults.
2. `initLogger()` — logrus, level from `LOG_LEVEL`.
3. `loadSettings()` — reads `config/settings.json` (custom-field flags), or writes defaults if missing.
4. `NewPaperlessClient(...)` — bare `http.Client` with optional `InsecureSkipVerify`. Caches downloads under `PAPERLESS_GPT_CACHE_DIR` if set.
5. `refreshCustomFieldsCache()` — pulls the custom-fields list from paperless-ngx, then schedules an hourly refresh in a goroutine.
6. `InitializeDB()` — opens / migrates the SQLite DB (modifications + OCR pages).
7. `loadTemplates()` — for each of eight prompts (`title`, `tag`, `correspondent`, `document_type`, `created_date`, `custom_field`, `ocr`, `adhoc-analysis`): copy from `default_prompts/` to `prompts/` if missing, then parse with sprig funcs.
8. `createLLM()` / `createVisionLLM()` — construct the main and vision LLM clients via [langchaingo](https://github.com/tmc/langchaingo). Both get wrapped in `NewRateLimitedLLM(...)` from [llm_client.go](../../repos-folder/paperless-gpt/llm_client.go) honoring `*_REQUESTS_PER_MINUTE`, `*_MAX_RETRIES`, `*_BACKOFF_MAX_WAIT`.
9. `ocr.NewProvider(config)` — builds an `ocr.Provider` for the chosen backend. Validates that the configured `OCR_PROCESS_MODE` is supported by that provider (`validateOCRProviderModeCompatibility`).
10. `App{...}` is assembled and `StartBackgroundTasks(ctx, app)` spawns the polling goroutine.
11. `startWorkerPool(app, 1)` starts a single OCR worker goroutine that consumes from `jobQueue`.
12. The Gin router is mounted (static frontend + API routes) and `router.Run(LISTEN_INTERFACE)` blocks. Default listen interface is `:8080`.

## Components

### `App` struct ([main.go](../../repos-folder/paperless-gpt/main.go))

Holds runtime dependencies: `Client` (paperless-ngx), `Database` (GORM), `LLM`, `VisionLLM`, `ocrProvider`, `ocrProcessMode`, `docProcessor` (for tests), plus all the PDF / hOCR / metadata flags. Most HTTP handlers are methods on `*App`.

### Background poller ([background.go](../../repos-folder/paperless-gpt/background.go))

`StartBackgroundTasks` runs an infinite loop with:

- 10s polling interval.
- Exponential backoff (10s → 1h max) on error.
- Each tick: if OCR enabled, `processAutoOcrTagDocuments`; then `processAutoTagDocuments`. Both query paperless-ngx for documents tagged with their respective tags (default page size 25) and process them.

### Job worker pool ([jobs.go](../../repos-folder/paperless-gpt/jobs.go))

A buffered `chan *Job` of capacity 100 plus a single worker goroutine. Manual OCR submissions from the UI go through `POST /api/documents/:id/ocr` which creates a `Job` (UUID, status, pages-done counter, options) and pushes it onto the queue. The worker drains the queue and runs `App.ProcessDocumentOCR`.

Concurrent control:

- `jobCancellersMu` + `jobCancellers` — map of jobID → `context.CancelFunc` so `POST /api/ocr/jobs/:job_id/stop` can cancel an in-flight job.
- `reOcrCancellers` — same idea for the per-page re-OCR endpoint.

### OCR provider interface ([ocr/provider.go](../../repos-folder/paperless-gpt/ocr/provider.go))

```go
type Provider interface {
    ProcessImage(ctx context.Context, imageContent []byte, pageNumber int) (*OCRResult, error)
}

type OCRResult struct {
    Text           string
    HOCRPage       *hocr.Page                // optional, provider-dependent
    Metadata       map[string]string
    OcrLimitHit    bool
    GenerationInfo map[string]interface{}    // LLM-specific (token counts, etc.)
}
```

`ocr.NewProvider(config)` dispatches on `Provider` (`llm`, `google_docai`, `azure`, `docling`, `mistral_ocr`) and returns the matching implementation. Each provider lives in its own file under `ocr/`.

An additional interface `HOCRCapable` (in [ocr.go](../../repos-folder/paperless-gpt/ocr.go)) is implemented by providers that can return precise word-level positions for searchable-PDF generation. As of the current code, only Google Document AI implements it.

### Rate-limited LLM ([llm_client.go](../../repos-folder/paperless-gpt/llm_client.go))

`NewRateLimitedLLM(llm, RateLimitConfig{RequestsPerMinute, MaxRetries, BackoffMaxWait})` wraps a `langchaingo` `llms.Model`. Uses `github.com/hashicorp/go-retryablehttp` semantics: retries on transient errors with exponential backoff up to `BackoffMaxWait`, while enforcing a token-bucket rate limit via `golang.org/x/time/rate`.

### SQLite DB ([local_db.go](../../repos-folder/paperless-gpt/local_db.go))

Two tables (GORM-managed):

- `modifications` — every document update made via paperless-gpt, used for the History view and the undo endpoint.
- `ocr_pages` — per-page OCR results: `document_id`, `page_index`, `text`, `ocr_limit_hit`, `generation_info_json`, timestamps. Surfaced via `GET /api/documents/:id/ocr_pages` and re-edited via `POST /api/documents/:id/ocr_pages/:pageIndex/reocr`.

DB file path defaults next to the binary; configurable via env (`PAPERLESS_GPT_DB_PATH`).

### Frontend ([web-app/](../../repos-folder/paperless-gpt/web-app/))

React 18 + Vite + TypeScript + Tailwind, served either from `web-app/dist/` on disk (if present, useful for local dev) or from embedded assets (`embedded_assets.go`). Routes: `/`, `/history`, `/experimental-ocr`, `/settings`, `/adhoc-analysis` — all 200-served the same `index.html` so the client-side router takes over.

Component layout in [web-app/src/](../../repos-folder/paperless-gpt/web-app/src/): `App.tsx` (router shell), `DocumentProcessor.tsx` (review queue), `ExperimentalOCR.tsx` (OCR review with per-page re-OCR), `AdhocAnalysis.tsx`, `History.tsx`, plus `components/` for shared widgets and `ocrStatus.ts` for the polling utilities.

## OCR pipeline ([ocr.go](../../repos-folder/paperless-gpt/ocr.go))

`App.ProcessDocumentOCR(ctx, documentID, options, jobID) -> *ProcessedDocument`. Steps:

1. Optional skip if `PDF_SKIP_EXISTING_OCR` is set and the source PDF already has a text layer (`pdfocr.DetectOCR` from `github.com/gardar/ocrchestra/pkg/pdfocr`), or if the OCR-complete tag is already present.
2. Switch on `OCR_PROCESS_MODE`:
   - `whole_pdf` — download the whole PDF and pass it to `provider.ProcessImage(_, _, 0)` once.
   - `pdf` — download per-page PDFs and run `ProcessImage` per page.
   - `image` (default) — render each page to a JPEG (subject to the `IMAGE_MAX_*` env limits) and run `ProcessImage` per page.
3. Persist each page's result to `ocr_pages` so the UI can show partial progress and per-page reruns.
4. If the provider implements `HOCRCapable`, assemble a complete `*hocr.HOCR` document via `GetHOCRDocument()`, then generate the HTML.
5. Optional file outputs (`CREATE_LOCAL_HOCR`, `CREATE_LOCAL_PDF`):
   - For `image` mode: use `pdfocr.AssembleWithOCR(hocrDoc, imageData, config)` to build a searchable PDF from scratch.
   - For `pdf`/`whole_pdf` modes: use `pdfocr.ApplyOCR(originalPDFBytes, hocrDoc, config)` to overlay the OCR text layer on the original PDF (preserving the image content).
6. Optional upload back to paperless-ngx (`PDF_UPLOAD`): create a new document with the searchable PDF; copy metadata if `PDF_COPY_METADATA`; delete the original if `PDF_REPLACE`. A safety check refuses to upload when fewer pages were processed than the original document has (e.g. `OCR_LIMIT_PAGES` cut short).

## Suggestion pipeline ([app_llm.go](../../repos-folder/paperless-gpt/app_llm.go))

`generateSuggestionsHandler` / `processAutoTagDocuments` build a `GenerateSuggestionsRequest` and run one or more LLM calls per document. The set of fields generated is controlled by:

- `GenerateTitles`, `GenerateTags`, `GenerateCorrespondents`, `GenerateCreatedDate`, `GenerateCustomFields`, `GenerateDocumentTypes` — booleans on the request struct.
- Server-side flags `AUTO_GENERATE_*` toggle the auto path; the UI lets the user pick per-document for manual review.
- `CREATE_NEW_TAGS` decides whether the LLM is allowed to invent tags that don't already exist in paperless-ngx (and they get created when applied).

Each generation step runs its prompt template through `text/template` + sprig, then through the rate-limited LLM, then validates and writes back via `PATCH /api/update-documents`.
