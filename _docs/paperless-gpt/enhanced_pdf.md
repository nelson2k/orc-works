# Enhanced PDF & hOCR

This is paperless-gpt's flagship feature for archival quality:
**produce a searchable, copy-paste-able PDF with a transparent text
layer that's accurately positioned over each word.**

## Requirements

- `OCR_PROVIDER: "google_docai"` — only Google DocAI emits the hOCR
  word-position data needed for accurate text-layer placement.
- `CREATE_LOCAL_HOCR: "true"` and/or `CREATE_LOCAL_PDF: "true"`.
- (Optional) `PDF_UPLOAD: "true"` to push the result back into
  paperless-ngx as a new document.

## How it works

1. Google DocAI returns text **and** word bounding boxes per page.
2. paperless-gpt builds an `hocr.HOCR` struct (via
   [`ocrchestra/pkg/hocr`](https://github.com/gardar/ocrchestra)) and
   renders it to an `.hocr` HTML file.
3. `pdfocr.ApplyOCR(originalPDFData, hocrDoc, config)` overlays an
   invisible text layer at the same coordinates as the printed text.
   For `image` mode it uses `AssembleWithOCR(hocrDoc, imageDataList,
   config)` instead — building a new PDF from the page images plus
   the text layer.

See [ocr.go:336-422](../../repos-folder/paperless-gpt/ocr.go#L336-L422).

## Output paths

```yaml
CREATE_LOCAL_HOCR: "true"
LOCAL_HOCR_PATH: "/app/hocr"      # mount this as a host volume
CREATE_LOCAL_PDF: "true"
LOCAL_PDF_PATH: "/app/pdf"        # mount this as a host volume
```

Filename pattern: `00012345_paperless-gpt_ocr.pdf` /
`00012345_paperless-gpt_ocr.hocr` (zero-padded document ID).

## Re-uploading to paperless-ngx

paperless-ngx doesn't let you replace a document's binary in place
via the API. paperless-gpt works around this by **uploading the
enhanced PDF as a new document** and (optionally) deleting the
original.

```yaml
PDF_UPLOAD: "true"
PDF_COPY_METADATA: "true"           # copy title/tags/correspondent/date
PDF_OCR_TAGGING: "true"             # tag the new doc
PDF_OCR_COMPLETE_TAG: "paperless-gpt-ocr-complete"
PDF_REPLACE: "false"                # ⚠ true = delete the original
```

> `PDF_REPLACE: "true"` is destructive. Back up paperless-ngx first.
> See the warning callout in the upstream README.

## Metadata that *can* be copied

- Title
- Tags (plus the OCR complete tag)
- Correspondent
- Created date

## Metadata that **cannot** be copied (paperless-ngx API limits)

- Document ID
- Added date / modified date
- Custom fields added by other plugins
- Notes / annotations

## Safety: partial-document protection

If `OCR_LIMIT_PAGES` is set lower than the doc's page count, paperless-
gpt **skips PDF generation entirely** rather than produce a truncated
copy. Set `OCR_LIMIT_PAGES: "0"` to process all pages.
