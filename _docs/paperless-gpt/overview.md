# Overview

## What it is

paperless-gpt is a **sidecar for paperless-ngx**. paperless-ngx is the
self-hosted document-management system (think: scan invoices/receipts/
bills, dump them in a folder, get a searchable archive). paperless-gpt
adds an LLM layer on top.

## What it does

For documents already in paperless-ngx:

1. **Title generation** — replace "scan_20240801_001.pdf" with "ACME
   Invoice #4471, 2024-08-01".
2. **Tag suggestion** — pick from your existing tags, optionally create
   new ones.
3. **Correspondent generation** — extract the sender/issuer name.
4. **Created-date extraction** — parse the actual document date from
   text, not the scan date.
5. **Document-type classification** — pick from existing types.
6. **Custom-field extraction** — fill arbitrary paperless-ngx custom
   fields (invoice total, due date, account number, etc.) using
   per-field prompts.
7. **Re-OCR with a vision LLM** — replace Tesseract's output with
   gpt-4o / Claude / minicpm-v / Mistral OCR / Azure DocAI / Google
   DocAI / Docling output. Often much cleaner on messy scans (see the
   FedEx-receipt side-by-side in the upstream README).
8. **Searchable PDF generation** — for providers that emit hOCR
   (currently Google DocAI), produce a PDF with a transparent text
   layer on top of the original page images.
9. **Ad-hoc analysis** — run a custom prompt across a selection of
   documents (summarise, extract X, etc.).

## How users trigger it

Tag-driven. Inside paperless-ngx you slap one of these tags on a
document:

- `paperless-gpt` → queue for manual review in paperless-gpt's web UI
- `paperless-gpt-auto` → auto-apply suggested title/tags/correspondent
- `paperless-gpt-ocr-auto` → auto re-OCR

paperless-gpt polls paperless-ngx, processes matching documents, and
writes results back via the paperless-ngx REST API.

## Who it's for

Self-hosters who already run paperless-ngx and want LLM-quality
metadata + OCR without manually editing every document. Not a good fit
for one-off "convert this PDF to markdown" tasks — that's what marker /
MinerU / chandra are for. See [vs_others.md](vs_others.md).
