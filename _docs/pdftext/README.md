# PDFText — Docs Index

PDFText (by Datalab / Vik Paruchuri) is a thin Python wrapper around
[pypdfium2](https://github.com/pypdfium2-team/pypdfium2) that extracts plain
text or structured blocks/lines/spans/chars from PDFs. Apache-2.0 licensed
(the point — it's a PyMuPDF replacement that isn't AGPL).

Source repo: `repos-folder/pdftext/`.

## Pipeline at a glance

```
pdf path
  └─ pypdfium2.PdfDocument
      └─ for each page in page_range:
          ├─ get_chars (pypdfium2 char-by-char + font/bbox/rotation)
          ├─ deduplicate_chars (drop duplicates from overlapping text objects)
          ├─ get_spans (group by font/rotation/no-break)
          ├─ get_lines (group by linebreak markers + y-position)
          ├─ assign_scripts (super/subscript detection)
          └─ get_blocks (cluster lines by median x/y gap)
      └─ (optional) add_links_and_refs (split spans on link bboxes, build cross-refs)
      └─ postprocess (ligatures, control chars, hyphens, optional reading-order sort)
      └─ output: list[Page] dict OR plain text OR table cells
```

## Pages

- [overview.md](overview.md) — what pdftext is, module map
- [install.md](install.md) — pip and poetry install
- [cli.md](cli.md) — `pdftext` CLI flags
- [api.md](api.md) — `plain_text_output`, `dictionary_output`, `table_output`
- [pipeline.md](pipeline.md) — chars → spans → lines → blocks in detail
- [schema.md](schema.md) — `Bbox`, `Char`, `Span`, `Line`, `Block`, `Page`, `Link`, `Reference`
- [postprocessing.md](postprocessing.md) — hyphens, ligatures, control chars, reading-order sort
- [links.md](links.md) — link annotation extraction and span splitting
- [tables.md](tables.md) — text inside externally-detected table bboxes
- [settings.md](settings.md) — tiny: just `WORKER_PAGE_THRESHOLD`
- [vs_others.md](vs_others.md) — vs PyMuPDF, pdfplumber, marker's use of pdftext
