# Backends

[docling/backend/](../../repos-folder/docling/docling/backend/). A backend opens a file and exposes either pages (paginated) or a finished `DoclingDocument` (declarative).

## Type hierarchy

[abstract_backend.py](../../repos-folder/docling/docling/backend/abstract_backend.py):

```
AbstractDocumentBackend
├── PaginatedDocumentBackend     # page_count(); pages loaded lazily
│   └── PdfDocumentBackend (in pdf_backend.py)
└── DeclarativeDocumentBackend   # convert() -> DoclingDocument directly
```

All backends receive `(in_doc: InputDocument, path_or_stream, options)` and expose `is_valid()`, `supported_formats() -> set[InputFormat]`, `unload()`. `unload` closes any `BytesIO` and drops the reference.

## Declarative backends

These produce a `DoclingDocument` straight from the source — `SimplePipeline.convert()` simply calls `backend.convert()`.

| Backend | Module | Formats |
|---|---|---|
| `CsvDocumentBackend` | csv_backend.py | CSV |
| `MsWordDocumentBackend` | msword_backend.py (+ docx/) | DOCX, DOTX, DOCM, DOTM |
| `MsExcelDocumentBackend` | msexcel_backend.py | XLSX, XLSM, XLTX, XLTM |
| `MsPowerpointDocumentBackend` | mspowerpoint_backend.py | PPTX, POTX, PPSX, PPTM, POTM, PPSM |
| `HTMLDocumentBackend` | html_backend.py | HTML, HTM, XHTML |
| `MarkdownDocumentBackend` | md_backend.py | MD, TXT, TEXT, QMD, RMD |
| `AsciiDocBackend` | asciidoc_backend.py | ASCIIDOC |
| `LatexDocumentBackend` | latex_backend.py (+ latex/) | TEX, LATEX |
| `JatsDocumentBackend` | xml/jats_backend.py | NLM JATS XML articles |
| `PatentUsptoDocumentBackend` | xml/uspto_backend.py | USPTO patent XML |
| `XBRLDocumentBackend` | xml/xbrl_backend.py | XBRL financial reports |
| `DoclingJSONBackend` | json/docling_json_backend.py | Docling's own JSON dump format |
| `WebVTTDocumentBackend` | webvtt_backend.py | WebVTT subtitles |
| `NoOpBackend` | noop_backend.py | AUDIO (delegated to AsrPipeline) |
| `ImageDocumentBackend` | image_backend.py | JPG, PNG, TIFF, BMP, WEBP |

## Paginated backends

For PDFs. Provide per-page loading; the pipeline iterates pages and only loads the bits it needs.

| Backend | Module | Notes |
|---|---|---|
| `DoclingParseDocumentBackend` | docling_parse_backend.py | Default. Wraps `docling-parse` (IBM) for full PDF understanding. |
| `DoclingParseV2DocumentBackend` | docling_parse_v2_backend.py | v2 parser. |
| `DoclingParseV4DocumentBackend` | docling_parse_v4_backend.py | v4 parser — newer, faster. |
| `PyPdfiumDocumentBackend` | pypdfium2_backend.py | Lightweight, pypdfium2-only. Used when `format-pdf-pypdfium2` is the only extra installed. |
| `ManagedPdfiumDocumentBackend` | managed_pdfium_backend.py | Threadsafe wrapper around pdfium with explicit lifecycle management. |
| `MetsGbsDocumentBackend` | mets_gbs_backend.py | METS/GBS Google Books packaging — extracts page TIFFs, treats them like a PDF. |

Pick via `PdfFormatOption(backend=...)` or via CLI `--pdf-backend {dlparse_v1,dlparse_v2,dlparse_v4,pypdfium2}`. `normalize_pdf_backend()` in `pipeline_options.py` maps the CLI name to the backend class.

## XML and DOCX subfolders

`xml/` and `docx/` hold helpers split out from the main backend files — sub-parsers, namespace tables, schema fixtures. Not exported directly.

## Format detection

`_DocumentConversionInput.docs()` in [datamodel/document.py](../../repos-folder/docling/docling/datamodel/document.py) drives detection:

1. Use the extension to look up `InputFormat` via `FormatToExtensions`.
2. For ambiguous extensions (`.xml`, `.txt`), sniff content with `filetype` + magic bytes + small heuristic checks. JATS vs XBRL vs USPTO is resolved by root element.
3. URL sources are streamed to a temp file first via `resolve_source_to_path` / `resolve_source_to_stream` from `docling-core`.

## Backend options

Per-format option models live in [datamodel/backend_options.py](../../repos-folder/docling/docling/datamodel/backend_options.py):

- `PdfBackendOptions` — flatten flags, password, link extraction.
- `HTMLBackendOptions` — base URL, strip tags.
- `MarkdownBackendOptions` — math delimiters, GitHub flavored toggles.
- `LatexBackendOptions` — preamble handling.
- `MetsGbsBackendOptions` — page selection inside the METS package.
- `XBRLBackendOptions` — taxonomy resolution.

Attach them via `<Format>FormatOption(backend_options=...)`.
