# Providers

A `Provider` is the file-format adapter. It exposes `get_images(idxs, dpi)`, `get_page_bbox(idx)`, `get_page_refs(idx)`, and a `page_lines: Dict[int, List[ProviderOutput]]` cache where `ProviderOutput` = `{line, spans, chars}`. See [marker/providers/__init__.py](../../repos-folder/marker/marker/providers/__init__.py).

Selection is done by `provider_from_filepath` in [marker/providers/registry.py](../../repos-folder/marker/marker/providers/registry.py): sniffs magic bytes via `filetype`, falls back to extension, finally to PDF.

| Source | Provider | Notes |
|---|---|---|
| PDF | `PdfProvider` | pdftext + pypdfium2; the only one that produces native lines by default |
| Image (jpg/png/…) | `ImageProvider` | Forces OCR — no native text |
| DOCX | `DocumentProvider` | Renders to PDF via weasyprint, then delegates |
| XLSX | `SpreadSheetProvider` | Same trick |
| PPTX | `PowerPointProvider` | Same trick |
| EPUB | `EpubProvider` | Same trick |
| HTML | `HTMLProvider` | Same trick |

Non-PDF providers depend on the `[full]` extra (mammoth, openpyxl, python-pptx, ebooklib, weasyprint).

## `PdfProvider`

[providers/pdf.py](../../repos-folder/marker/marker/providers/pdf.py). The most-used path. Key config (all `Annotated` so they appear under `--help`):

- `page_range` — `None` = whole doc; CLI accepts `0,5-10,20`.
- `pdftext_workers=4` — parallel native text extraction.
- `flatten_pdf=True` — flattens form fields so they render.
- `force_ocr=False` — skip pdftext entirely; only collect page bboxes.
- `strip_existing_ocr=False` — drop existing OCR text layer, re-OCR via surya.
- `disable_links=False` — drop hyperlinks.
- `keep_chars=False` — preserve per-character info.
- `ocr_space_threshold` / `ocr_newline_threshold` / `ocr_alphanum_threshold` — heuristics that detect "bad" native text and force OCR.
- `image_threshold=0.65` — if an image covers more than 65% of the page, page is treated as image-only.

Native text comes from `pdftext.extraction.dictionary_output`, which yields words/chars with bboxes. The provider normalizes them into `Line` → `Span` → `Char` and caches per page.

Page images come from `pypdfium2.PdfDocument.render` at the requested DPI. `DocumentBuilder` requests two DPIs: 96 lowres (layout, line detection) and 192 highres (recognition).
