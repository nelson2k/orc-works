# HTTP API

All routes are mounted under `/api/`, defined in [main.go](../../repos-folder/paperless-gpt/main.go) with handlers in [app_http_handlers.go](../../repos-folder/paperless-gpt/app_http_handlers.go). Gin defaults apply (logger + recovery middleware). Listen address is `LISTEN_INTERFACE`, default `:8080`.

Authentication: paperless-gpt does **not** authenticate inbound HTTP. The intended deployment is behind another auth layer (a reverse proxy, Tailscale, a Docker network, etc.). The only credential the service holds is the paperless-ngx API token it uses outbound.

## Documents

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api/documents` | List documents tagged with `MANUAL_TAG`. Used by the review queue. |
| `GET` | `/api/documents/:id` | Fetch one document with full metadata, content, current tags / correspondent / created date / custom fields |
| `POST` | `/api/generate-suggestions` | Run the LLM to suggest titles, tags, correspondents, dates, document types, and custom fields for a list of documents. Body: `GenerateSuggestionsRequest`. |
| `PATCH` | `/api/update-documents` | Apply a list of `DocumentSuggestion` updates back to paperless-ngx. Body: `[]DocumentSuggestion`. |

`GenerateSuggestionsRequest` (from [types.go](../../repos-folder/paperless-gpt/types.go)) has booleans `GenerateTitles`, `GenerateTags`, `GenerateCorrespondents`, `GenerateCreatedDate`, `GenerateCustomFields`, `GenerateDocumentTypes` plus a `Documents []Document` list. Each generated field becomes a `Suggested*` field on the returned `DocumentSuggestion`.

`DocumentSuggestion` also carries `KeepOriginalTags`, `RemoveTags`, `AddTags`, `CustomFieldsEnable`, `CustomFieldsWriteMode` so the client can express tag-set arithmetic explicitly.

## Tags / fields / settings

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api/tags` | Map of `tag_name → tag_id` from paperless-ngx |
| `GET` | `/api/custom_fields` | Cached list of `{id, name, data_type}`; refreshed hourly server-side |
| `GET` | `/api/filter-tag` | Return `{tag: MANUAL_TAG}` — exposes the trigger tag to the UI |
| `GET` | `/api/prompts` | Read all current prompt templates (the contents of `prompts/`) |
| `POST` | `/api/prompts` | Save edited prompt templates back to disk; reload immediately |
| `GET` | `/api/settings` | Read `config/settings.json` (custom-fields toggles) |
| `POST` | `/api/settings` | Persist `Settings` back to disk |

## OCR

| Method | Path | Purpose |
|---|---|---|
| `POST` | `/api/documents/:id/ocr` | Submit a new OCR job. Body: `OCROptions`. Returns `{job_id}`. |
| `GET` | `/api/documents/:id/ocr_pages` | List per-page OCR results from the local SQLite store |
| `POST` | `/api/documents/:id/ocr_pages/:pageIndex/reocr` | Re-OCR a single page (synchronous in a goroutine — cancellable) |
| `DELETE` | `/api/documents/:id/ocr_pages/:pageIndex/reocr` | Cancel an in-flight re-OCR |
| `GET` | `/api/jobs/ocr` | List every job in `JobStore`, sorted by `CreatedAt` |
| `GET` | `/api/jobs/ocr/:job_id` | One job's status: `pending` / `in_progress` / `completed` / `failed` / `cancelled`, plus `PagesDone` / `TotalPages` / `Result` |
| `POST` | `/api/ocr/jobs/:job_id/stop` | Cancel an in-flight OCR job |
| `GET` | `/api/experimental/ocr` | Returns `{enabled: bool}` — whether an OCR provider is configured |

`OCROptions` (from [types.go](../../repos-folder/paperless-gpt/types.go)):

```go
type OCROptions struct {
    UploadPDF       bool   // Upload the searchable PDF to paperless-ngx
    ReplaceOriginal bool   // Delete the original document after upload (DANGEROUS)
    CopyMetadata    bool   // Copy title/tags/correspondent/created date
    LimitPages      int    // Per-job page cap (overrides OCR_LIMIT_PAGES)
    ProcessMode     string // "image" | "pdf" | "whole_pdf"; empty = app default
}
```

Jobs are queued onto a buffered `chan *Job` of capacity 100; a single worker goroutine drains the queue. Cancellation is cooperative — the job's `context.CancelFunc` is stored in `jobCancellers` and invoked by the stop endpoint.

## History / modifications

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api/modifications` | Paginated list of changes paperless-gpt made (from SQLite `modifications`) |
| `POST` | `/api/undo-modification/:id` | Reverse a specific modification by writing the old state back to paperless-ngx |
| `POST` | `/api/analyze-documents` | Ad-hoc multi-document analysis. Body: `AnalyzeDocumentsRequest { DocumentIDs, Prompt }`. Runs the user's prompt against the concatenated content of the selected documents. |

## Misc

| Method | Path | Purpose |
|---|---|---|
| `GET` | `/api/paperless-url` | Returns `{url: PAPERLESS_PUBLIC_URL or PAPERLESS_BASE_URL}` — used by the frontend to build deep links |
| `GET` | `/api/version` | `{version, commit, buildDate, platform}` from `version.go` |

## Static assets

Non-`/api/` routes serve the embedded React SPA. The same `index.html` is served for `/`, `/history`, `/experimental-ocr`, `/settings`, `/adhoc-analysis` so the client-side router takes over. `/assets/*` and `/favicon.ico` are served from `web-app/dist/` on disk (if present) or from embedded assets via `serveEmbeddedFile` ([embedded_assets.go](../../repos-folder/paperless-gpt/embedded_assets.go)).
