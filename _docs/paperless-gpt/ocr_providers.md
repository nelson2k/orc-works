# OCR Providers

paperless-gpt has five OCR backends, all implementing the
`ocr.Provider` interface (see [ocr/provider.go](../../repos-folder/paperless-gpt/ocr/provider.go)).
Pick one via `OCR_PROVIDER`.

| Provider | `OCR_PROVIDER` value | hOCR? | Self-hosted? | Cost |
|----------|---------------------|-------|--------------|------|
| LLM (vision) | `llm` | ❌ | Yes (Ollama) | Free or per-token |
| Azure Document Intelligence | `azure` | ❌ | No | Per-page |
| Google Document AI | `google_docai` | ✅ | No | Per-page |
| Mistral OCR | `mistral_ocr` | ❌ | No | Per-page |
| Docling Server | `docling` | ❌ | Yes | Free |

## 1. LLM (default)

Push each page image through a vision-capable LLM. The OCR prompt is in
[default_prompts/ocr_prompt.tmpl](../../repos-folder/paperless-gpt/default_prompts/ocr_prompt.tmpl)
— effectively *"transcribe the text in this image, preserve formatting,
use markdown without code blocks."*

```yaml
OCR_PROVIDER: "llm"
VISION_LLM_PROVIDER: "openai"     # or ollama / mistral / anthropic
VISION_LLM_MODEL: "gpt-4o"        # or minicpm-v / claude-sonnet-4-5 / etc.
```

Best for messy/handwritten scans where Tesseract chokes. Cost scales
with model — `minicpm-v` on local Ollama is free, `gpt-4o` is ~$0.01/page.

## 2. Azure Document Intelligence

Microsoft's enterprise OCR. Defaults to the `prebuilt-read` model;
switch to `prebuilt-layout` for table-aware markdown output.

```yaml
OCR_PROVIDER: "azure"
AZURE_DOCAI_ENDPOINT: "https://your-resource.cognitiveservices.azure.com/"
AZURE_DOCAI_KEY: "..."
AZURE_DOCAI_MODEL_ID: "prebuilt-read"
AZURE_DOCAI_OUTPUT_CONTENT_FORMAT: "text"   # or "markdown" (layout model only)
```

## 3. Google Document AI

**The only provider that emits hOCR**, so it's also the only one that
can drive the [searchable PDF](enhanced_pdf.md) generation feature.

```yaml
OCR_PROVIDER: "google_docai"
GOOGLE_PROJECT_ID: "your-project"
GOOGLE_LOCATION: "us"             # or "eu"
GOOGLE_PROCESSOR_ID: "..."
GOOGLE_APPLICATION_CREDENTIALS: "/app/credentials.json"
```

Needs a service-account JSON mounted into the container.

## 4. Mistral OCR

Mistral's hosted OCR endpoint. Supports all three process modes
(image / pdf / whole_pdf). See
[docs/mistral_llm.md](../../repos-folder/paperless-gpt/docs/mistral_llm.md)
upstream for the dedicated guide.

```yaml
OCR_PROVIDER: "mistral_ocr"
MISTRAL_API_KEY: "..."
```

## 5. Docling Server

Self-hosted [Docling](https://github.com/DS4SD/docling) instance.
Supports two pipelines:

- **`vlm`** (default): visual-language-model pipeline
- **`standard`**: classical pipeline with a swappable OCR engine
  (`easyocr` by default, also `tesseract`, `macocr`)

```yaml
OCR_PROVIDER: "docling"
DOCLING_URL: "http://docling:5001"
DOCLING_OCR_PIPELINE: "vlm"
# DOCLING_OCR_ENGINE: "easyocr"   # only when pipeline=standard
```
