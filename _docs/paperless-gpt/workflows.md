# Workflows

paperless-gpt is driven by two trigger tags plus an opt-in OCR-tagging step. This page walks through each end-to-end flow.

## 1. Tag-driven auto metadata

```
User uploads PDF to paperless-ngx
  → paperless-ngx OCRs (its own engine) → indexes it
  → User (or a paperless-ngx rule) adds the `paperless-gpt-auto` tag
  → paperless-gpt's background poller (10 s) sees the tag
  → For each document:
      - Skip if it already has `paperless-gpt-ocr-auto`
      - Run prompt templates through the configured LLM:
          title_prompt        (if AUTO_GENERATE_TITLE)
          tag_prompt          (if AUTO_GENERATE_TAGS)
          correspondent_prompt (if AUTO_GENERATE_CORRESPONDENTS)
          document_type_prompt (if AUTO_GENERATE_DOCUMENT_TYPE)
          created_date_prompt  (if AUTO_GENERATE_CREATED_DATE)
          custom_field_prompt  (if settings.json says so + IDs selected)
      - PATCH the document back to paperless-ngx
      - Remove the `paperless-gpt-auto` tag
      - Record the change in the local SQLite `modifications` table
```

`AUTO_TAG` is removed after the LLM run, regardless of result, so the same document isn't reprocessed in a loop.

## 2. Manual review

```
User adds the `paperless-gpt` tag to a document
  → paperless-gpt UI's "Document Processor" page lists it (GET /api/documents)
  → User opens the document, picks which fields to generate
  → UI calls POST /api/generate-suggestions
  → User reviews proposed title/tags/etc., edits if needed
  → UI calls PATCH /api/update-documents
  → paperless-gpt:
      - applies the changes via paperless-ngx REST API
      - removes the `paperless-gpt` tag
      - records modifications
```

## 3. Tag-driven auto OCR

```
User adds the `paperless-gpt-ocr-auto` tag to a document
  → Background poller picks it up
  → App.ProcessDocumentOCR is called:
      - If PDF_SKIP_EXISTING_OCR=true and the PDF already has a text layer → skip
      - Otherwise, run the configured OCR provider
        (in image/pdf/whole_pdf mode)
      - Save per-page text into SQLite (`ocr_pages`)
      - If the provider implements HOCRCapable (only google_docai):
          - Build hOCR HTML; optionally save to /app/hocr
          - Apply or Assemble a searchable PDF; optionally save to /app/pdf
          - If PDF_UPLOAD=true: create a new document in paperless-ngx
          - If PDF_REPLACE=true: delete the original after upload
          - Add the `paperless-gpt-ocr-complete` tag to the upload (if PDF_OCR_TAGGING)
      - Remove the `paperless-gpt-ocr-auto` tag
```

## 4. Manual OCR with per-page review

```
User opens the "Experimental OCR" page in the UI
  → Selects a document → POST /api/documents/:id/ocr with OCROptions
  → paperless-gpt enqueues a Job (UUID, status pending)
  → Worker picks it up, processes pages one by one,
    persisting each to `ocr_pages` so the UI's poll on
    GET /api/documents/:id/ocr_pages shows live progress
  → User can stop the job via POST /api/ocr/jobs/:job_id/stop
  → User can re-OCR a single page via POST /api/documents/:id/ocr_pages/:pageIndex/reocr
    (cancellable via DELETE on the same path)
  → When all pages are done, if upload was requested,
    the searchable PDF is built and uploaded as in flow 3
```

## 5. Ad-hoc multi-document analysis

```
User opens "Adhoc Analysis"
  → Selects a few documents → enters a free-form prompt
  → POST /api/analyze-documents { document_ids, prompt }
  → paperless-gpt concatenates the documents' content,
    renders the adhoc-analysis_prompt.tmpl with the user's prompt
    and the documents block, then calls the LLM
  → Returns the LLM's response to the UI
```

Nothing is written back to paperless-ngx in this flow — it's read-only analysis.

## 6. History / undo

```
Every metadata change paperless-gpt makes is recorded
  in the SQLite `modifications` table
  (old → new value, document ID, timestamp, kind).

User opens the History page
  → GET /api/modifications (paginated)
  → User clicks "Undo" on a row
  → POST /api/undo-modification/:id
  → paperless-gpt PATCHes paperless-ngx with the old value
  → Marks the modification as undone
```

## OCR-complete tag interaction

`PDF_OCR_COMPLETE_TAG` (`paperless-gpt-ocr-complete` by default) is the marker used to track "this document has been through OCR". The interactions:

- After a successful enhanced-PDF upload, the tag is applied to the new document (when `PDF_OCR_TAGGING=true`).
- When `PDF_SKIP_EXISTING_OCR=true`, a document carrying this tag is short-circuited at the top of `ProcessDocumentOCR`.
- The auto-metadata flow in `processAutoTagDocuments` skips documents that also have `AUTO_OCR_TAG`, so a document waiting for OCR doesn't get its metadata regenerated mid-pipeline.

These interactions are why `AUTO_TAG`, `AUTO_OCR_TAG`, and `PDF_OCR_COMPLETE_TAG` should all be different — sharing names causes loops.

## Failure handling

- **Background poller** uses exponential backoff from 10 s up to 1 h on errors, then resets to 10 s on the next success. A polling cycle that processed nothing also sleeps 10 s before trying again.
- **LLM calls** go through `NewRateLimitedLLM` ([llm_client.go](../../repos-folder/paperless-gpt/llm_client.go)) — token-bucket rate limit + `MaxRetries` retries with exponential backoff capped at `BackoffMaxWait`.
- **OCR jobs** record errors in `Job.Result` and set status `failed`. Per-page errors don't abort the whole job — failed pages are logged and the rest continue.
- **PDF upload** is gated by a safety check: if fewer pages were OCR'd than the original document has (e.g. because `OCR_LIMIT_PAGES` cut it short), the PDF assembly is skipped entirely with a warning. This prevents silent data loss when `PDF_REPLACE=true`.
- **Original-deletion** (`PDF_REPLACE`) polls paperless-ngx's task-status endpoint for up to 60 seconds (12 retries × 5 s) waiting for the new upload to finish processing before issuing the delete. If the task fails, the original is preserved.
