# Providers (`marker/providers/`)

A provider reads a source file and exposes a uniform interface to the builders:

```python
class BaseProvider:
    page_range: list[int] | None
    filepath: str
    def get_images(idxs, dpi) -> list[PIL.Image]
    def get_page_bbox(idx) -> PolygonBox | None
    def get_page_lines(idx) -> list[Line]   # used by LineBuilder
    def get_page_refs(idx) -> list[Reference]
```

A `ProviderOutput` bundles `(line, spans, chars?)`; `ProviderPageLines` is
`{page_idx: [ProviderOutput, ...]}`.

## Selecting a provider — `registry.py`

`provider_from_filepath(filepath)` sniffs with `filetype.match()`, falls back
to extension matching, then to "is this HTML?", then defaults to
`PdfProvider`.

| Type    | Class                | Notes |
|---------|----------------------|-------|
| PDF     | `PdfProvider`        | Uses `pypdfium2` (rendering / refs) + `pdftext` (text extraction). |
| Image   | `ImageProvider`      | Single-page; image is loaded with PIL and treated as a page. |
| EPUB    | `EpubProvider`       | Converts EPUB → HTML → PDF via WeasyPrint, then delegates. |
| HTML    | `HTMLProvider`       | Renders HTML → PDF via WeasyPrint (uses bundled fonts). |
| DOCX    | `DocumentProvider`   | Converts via mammoth / similar to HTML → PDF. |
| XLSX    | `SpreadSheetProvider`| Converts each sheet to a PDF-rendered page. |
| PPTX    | `PowerPointProvider` | Renders slides as pages. |

The non-PDF providers ultimately produce something `PdfProvider`-shaped so
that downstream builders need only one path.

## PdfProvider (key flags)

- `page_range` — subset of pages to convert (`--page_range 0,5-10`).
- `flatten_pdf` — flatten form fields / annotations before reading text.
- `force_ocr` — pretend the PDF has no text; everything goes through OCR.
- `strip_existing_ocr` — drop existing OCR text but keep digital text.
- `ocr_invalid_chars`, `ocr_space_threshold`, `ocr_newline_threshold`,
  `ocr_alphanum_threshold`, `image_threshold` — heuristics that flag pages
  with bad embedded text so OCR kicks in.
- `disable_links` — drop link annotations.
- `keep_chars` — keep per-character bounding boxes (mirrored on OcrBuilder).

## Font CSS

`BaseProvider.get_font_css()` returns a WeasyPrint `CSS` object that uses
`settings.FONT_PATH` (`GoNotoCurrent-Regular.ttf` by default). The font is
downloaded on first run via `marker.util.download_font()` (called from
`BaseConverter.__init__`).
