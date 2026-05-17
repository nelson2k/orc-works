# OCR Process Modes

Set with `OCR_PROCESS_MODE`. Controls *what* gets sent to the OCR
provider for each document.

## image (default)

Rasterise PDF → one JPEG per page → call provider once per page.

- Works with every provider.
- Page size capped by `IMAGE_MAX_PIXEL_DIMENSION` (10000),
  `IMAGE_MAX_TOTAL_PIXELS` (40M), `IMAGE_MAX_RENDER_DPI` (600), and
  `IMAGE_MAX_FILE_BYTES` (10 MB JPEG).
- Required for `llm` provider — vision LLMs only take images.

## pdf

Split PDF → one single-page PDF per page → call provider once per page.

- Preserves embedded text, vector layout, fonts.
- Only Google DocAI, Mistral OCR, and Docling accept this.
- Often more accurate than `image` mode because the provider can use
  the embedded text layer if present.

## whole_pdf

Send the entire multi-page PDF in a single provider call.

- One API call per document → cheaper.
- Watch out for provider page/size limits on big PDFs.
- Only Google DocAI, Mistral OCR, and Docling accept this.

## Provider compatibility table

| Provider | image | pdf | whole_pdf |
|----------|:-----:|:---:|:---------:|
| `llm` (OpenAI/Ollama/etc.) | ✅ | ❌ | ❌ |
| `azure` | ✅ | ❌ | ❌ |
| `google_docai` | ✅ | ✅ | ✅ |
| `mistral_ocr` | ✅ | ✅ | ✅ |
| `docling` | ✅ | ✅ | ✅ |

paperless-gpt validates the mode/provider combo at startup and refuses
to boot if it's incompatible.

## Skip already-OCR'd PDFs

In `pdf` or `whole_pdf` mode you can short-circuit documents that
already have OCR text or an OCR layer:

```yaml
OCR_PROCESS_MODE: "pdf"
PDF_SKIP_EXISTING_OCR: "true"
```

The check uses the `pdfocr.DetectOCR` from `ocrchestra` plus a tag
check for `PDF_OCR_COMPLETE_TAG`. See
[ocr.go:88-110](../../repos-folder/paperless-gpt/ocr.go#L88-L110).
