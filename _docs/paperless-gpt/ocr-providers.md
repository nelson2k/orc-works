# OCR providers

[ocr/provider.go](../../repos-folder/paperless-gpt/ocr/provider.go) defines a single interface:

```go
type Provider interface {
    ProcessImage(ctx context.Context, imageContent []byte, pageNumber int) (*OCRResult, error)
}
```

Five concrete providers ship in the repo, each in its own file under `ocr/`. `ocr.NewProvider(config)` dispatches on the `OCR_PROVIDER` env var.

## 1. LLM-based OCR (`llm`)

[ocr/llm_provider.go](../../repos-folder/paperless-gpt/ocr/llm_provider.go).

Sends the page image to a chat-completion vision model with the rendered `ocr_prompt.tmpl` prompt. Supported vision LLM providers (via `VISION_LLM_PROVIDER`):

| Value | Backed by | Notes |
|---|---|---|
| `openai` | `langchaingo/llms/openai` | Works for Azure OpenAI too if `OPENAI_API_TYPE=azure` and `OPENAI_BASE_URL` are set |
| `ollama` | `langchaingo/llms/ollama` | Local — set `OLLAMA_HOST` |
| `mistral` | OpenAI-compatible against `https://api.mistral.ai/v1` | Set `MISTRAL_API_KEY` |
| `anthropic` | `langchaingo/llms/anthropic` | Set `ANTHROPIC_API_KEY` |
| `googleai` | Custom Gemini client in [ocr/google_ai_client.go](../../repos-folder/paperless-gpt/ocr/google_ai_client.go) | Set `GOOGLEAI_API_KEY`. Natively accepts PDF input — only LLM variant that supports `pdf` / `whole_pdf` process modes |

The vision LLM is wrapped in `NewRateLimitedLLM(...)` with the `VISION_LLM_*` env knobs (`REQUESTS_PER_MINUTE`, `MAX_RETRIES`, `BACKOFF_MAX_WAIT`).

Knobs:

- `VISION_LLM_MAX_TOKENS` — output cap; if hit, `OcrLimitHit=true` is propagated.
- `VISION_LLM_TEMPERATURE` — sampling temperature. For OpenAI GPT-5, must be `1.0`.
- `OLLAMA_OCR_TOP_K`, `OLLAMA_CONTEXT_LENGTH` — Ollama-specific overrides.
- `GOOGLEAI_THINKING_BUDGET` — disable Gemini thinking with `0`, or omit for default.

Output is plain text only — no hOCR positions, so PDF-text-layer generation is not available.

## 2. Google Document AI (`google_docai`)

[ocr/google_docai_provider.go](../../repos-folder/paperless-gpt/ocr/google_docai_provider.go). Uses `cloud.google.com/go/documentai/apiv1` against a configured `processor_id`.

| Env | Purpose |
|---|---|
| `GOOGLE_PROJECT_ID` | GCP project |
| `GOOGLE_LOCATION` | Region (`us`, `eu`) |
| `GOOGLE_PROCESSOR_ID` | Document AI processor ID |
| `GOOGLE_APPLICATION_CREDENTIALS` | Path to mounted service account JSON |

**Only provider that implements `HOCRCapable`.** Builds a `*hocr.Page` per call (using token positions from the Document AI response), then `ocr.go` assembles them into a full `*hocr.HOCR` and into a searchable PDF via `github.com/gardar/ocrchestra/pkg/pdfocr`.

Supports all three `OCR_PROCESS_MODE` values (`image`, `pdf`, `whole_pdf`).

## 3. Azure Document Intelligence (`azure`)

[ocr/azure_provider.go](../../repos-folder/paperless-gpt/ocr/azure_provider.go) + [ocr/azure_types.go](../../repos-folder/paperless-gpt/ocr/azure_types.go). Uses the Azure DocAI REST API directly (not via SDK).

| Env | Purpose |
|---|---|
| `AZURE_DOCAI_ENDPOINT` | e.g. `https://<resource>.cognitiveservices.azure.com/` |
| `AZURE_DOCAI_KEY` | API key |
| `AZURE_DOCAI_MODEL_ID` | Default `prebuilt-read`. `prebuilt-layout` needed for markdown output |
| `AZURE_DOCAI_TIMEOUT_SECONDS` | Default 120 |
| `AZURE_DOCAI_OUTPUT_CONTENT_FORMAT` | `text` (default) or `markdown` |

Supports only `image` process mode. Plain-text output; no hOCR.

## 4. Docling server (`docling`)

[ocr/docling_provider.go](../../repos-folder/paperless-gpt/ocr/docling_provider.go). Talks HTTP to a self-hosted [docling](https://github.com/docling-project/docling) server.

| Env | Default | Notes |
|---|---|---|
| `DOCLING_URL` | required | Base URL of the docling service |
| `DOCLING_IMAGE_EXPORT_MODE` | `embedded` | `embedded` / `placeholder` |
| `DOCLING_OCR_PIPELINE` | `vlm` | `vlm` or `standard` |
| `DOCLING_OCR_ENGINE` | `easyocr` | Only used when pipeline is `standard`; passed straight through to docling |

Supports all three process modes. Plain-text output.

## 5. Mistral OCR (`mistral_ocr`)

[ocr/mistral_provider.go](../../repos-folder/paperless-gpt/ocr/mistral_provider.go). Different from "LLM provider = mistral" — this one uses Mistral's dedicated OCR API (`mistral-ocr-latest`), not the chat-completions endpoint.

| Env | Purpose |
|---|---|
| `MISTRAL_API_KEY` | API key |
| `MISTRAL_MODEL` | Optional, defaults to `mistral-ocr-latest` |

Supports all three process modes.

## Process-mode compatibility matrix

Enforced at startup by `validateOCRProviderModeCompatibility` in [main.go](../../repos-folder/paperless-gpt/main.go); paperless-gpt fails to start with a clear error if the combination is unsupported.

| Provider | `image` | `pdf` | `whole_pdf` |
|---|:---:|:---:|:---:|
| `llm` (most vision LLMs) | ✅ | ❌ | ❌ |
| `llm` + `VISION_LLM_PROVIDER=googleai` | ✅ | ✅ | ✅ |
| `azure` | ✅ | ❌ | ❌ |
| `google_docai` | ✅ | ✅ | ✅ |
| `mistral_ocr` | ✅ | ✅ | ✅ |
| `docling` | ✅ | ✅ | ✅ |

## Process modes

- **`image`** (default) — `App.Client.DownloadDocumentAsImages` renders each PDF page to a JPEG using `github.com/gen2brain/go-fitz` (libmupdf binding), respecting the image-limit env vars. Each image then goes through `ProcessImage` individually.
- **`pdf`** — `DownloadDocumentAsPDF(_, _, pageLimit, true)` splits the original PDF into one PDF per page using `github.com/pdfcpu/pdfcpu`. Each single-page PDF goes through `ProcessImage` (page number 1-based).
- **`whole_pdf`** — the entire PDF blob is passed in one `ProcessImage` call with `pageNumber=0`. Only works with providers that handle multi-page documents natively. Watch out for API token limits on long documents.

`PDF_SKIP_EXISTING_OCR` is honored only in `pdf` and `whole_pdf` modes; it inspects the downloaded PDF for an existing text layer (via `pdfocr.DetectOCR`) before calling the provider and short-circuits if found.

## hOCR / searchable-PDF generation

Only kicks in when:

- the provider implements `HOCRCapable` (currently only `google_docai`), and
- `CREATE_LOCAL_HOCR=true` or `CREATE_LOCAL_PDF=true` or `PDF_UPLOAD=true` is set.

A safety check refuses to generate a final PDF when the count of processed pages doesn't match the document's total page count — typical when `OCR_LIMIT_PAGES` is set. This is intentional: a partial searchable PDF replacing the original would silently lose pages.

The hOCR HTML and the assembled PDF are stored under `/app/hocr` and `/app/pdf` respectively (file name `<padded-doc-id>_paperless-gpt_ocr.{hocr,pdf}`).
