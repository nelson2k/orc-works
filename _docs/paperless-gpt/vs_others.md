# paperless-gpt vs marker / MinerU / chandra

**Different category.** marker, MinerU, and chandra all turn a *PDF on
disk* into *markdown on disk*. paperless-gpt turns a *paperless-ngx
document* into a *better-organised paperless-ngx document*.

| Aspect | paperless-gpt | marker | MinerU | chandra |
|--------|---------------|--------|--------|---------|
| Primary purpose | **Augment paperless-ngx** | PDF → markdown | PDF/DOCX/PPTX → markdown/JSON | PDF → markdown (VLM) |
| Standalone? | No (needs paperless-ngx) | Yes | Yes | Yes |
| Inputs | Whatever's in paperless-ngx | PDF/image/DOCX/etc. | PDF/image/DOCX/PPTX/XLSX | PDF/image |
| OCR engine | **Outsourced** (LLM / Azure / Google / Mistral / Docling) | Surya + heuristics | PaddleOCR / VLM | Qwen 3.5 VL |
| LLM purpose | Title / tag / correspondent / dates / custom fields / re-OCR | Optional cleanup | Optional VLM backend | Whole pipeline |
| Output | paperless-ngx fields + optional searchable PDF | `.md` + figures | `.md` / JSON + figures | `.md` |
| Language | Go | Python | Python | Python |
| Self-hostable? | Yes (Docker) | Yes | Yes | Yes (16+ GB GPU) |
| License | MIT | GPL-3.0 | Custom-Apache-2 | Apache-2.0 |

## Where they compete (a little)

Only the **OCR-quality dimension**:

- paperless-gpt with `OCR_PROVIDER=llm` + `gpt-4o` is one of the
  cleanest ways to re-OCR a messy scan — the upstream README's
  receipt and FedEx-invoice side-by-sides are genuinely impressive.
- marker / MinerU / chandra do this too, but as part of a full
  PDF→markdown pipeline rather than as a step inside a DMS.

If you just want "clean OCR output for one PDF", marker or chandra
will be faster end-to-end because paperless-gpt requires you to push
the doc through paperless-ngx first.

## Could you stack them?

Yes:

1. **marker / chandra / MinerU** as a one-time bulk converter for your
   archive → markdown for RAG / search.
2. **paperless-gpt** as the *live* layer on new scans going into
   paperless-ngx, doing both the metadata enrichment and (optionally)
   the OCR re-pass.

## Honest take for this repo

paperless-gpt is **out of scope for `017-ocr-works`** — your goal is
book-corpus conversion (3D graphics texts → markdown), not document
management. Keep paperless-gpt in the reference shelf for the day you
stand up paperless-ngx for invoices/bills/contracts; until then,
marker on the RTX 4070 is the right tool.

The interesting bit to **steal** from paperless-gpt is:

- The hOCR → searchable PDF pipeline (`ocrchestra` library) — if you
  ever want your converted books to also be selectable PDFs and not
  just markdown.
- The "ask the LLM nicely" OCR prompt — short, polite, and
  surprisingly effective. Worth comparing against marker's LLM
  cleanup prompts.
