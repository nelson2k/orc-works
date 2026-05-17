# Office formats (`mineru/backend/office/` + `mineru/model/{docx,pptx,xlsx}/`)

Native parsers for DOCX / PPTX / XLSX — no PDF round-trip. Added in 3.0
(DOCX) and 3.1 (PPTX, XLSX). End-to-end ~10–100× faster than the older
"convert to PDF then parse" flow and produces no hallucinations.

## DOCX (`backend/office/docx_analyze.py`, `model/docx/`)

- Built on `python-docx` (Microsoft's official OOXML lib via lxml).
- `package_normalizer.py` flattens `.docx` ZIP weirdness (custom XML
  namespaces, unusual relationships) into a standard layout.
- `docx_converter.py` walks the document tree → paragraphs, runs, tables,
  inline shapes, images.
- Math handling: OMML (Office Math Markup Language) → LaTeX via the
  `pylatexenc` dep.
- Output schema is the same `middle_json` used by pipeline / VLM
  backends.

## PPTX (`backend/office/pptx_analyze.py`, `model/pptx/`)

- Built on `pypptx-with-oxml` (a fork of python-pptx with deeper OXML
  exposure).
- `pptx_converter.py` walks every slide → shapes, text frames, tables,
  picture shapes.
- `xycut_pp_sorter.py` reorders shapes into reading order using an
  XY-cut algorithm (PPTX shapes are positioned absolutely; reading order
  is not stored).

## XLSX (`backend/office/xlsx_analyze.py`, `model/xlsx/`)

- Built on `openpyxl` + `pandas`.
- Each sheet becomes a `Table` block. Merged cells become `colspan` /
  `rowspan` in HTML output.
- Workbook-wide images are extracted and inlined alongside.

## Charts inside Office files

`backend/utils/office_chart.py` extracts chart data (categories, series,
values) from the underlying chartX XML and emits a structured
representation. Combined with the VLM's chart parsing on the rendered
image when available.

## Output

All three converge on the same `middle_json` format used by pipeline / VLM
backends — see [output.md](output.md). That means downstream tooling
(markdown rendering, content list, search indexing) is identical regardless
of input format.

## Backend dispatch

`cli/common.py`:
```python
office_suffixes = docx_suffixes + pptx_suffixes + xlsx_suffixes
```

When the CLI sees one of these extensions, it routes to
`office_docx_analyze` / `office_pptx_analyze` / `office_xlsx_analyze`
instead of the pipeline/vlm/hybrid PDF flow. Backend choice (`-b`) doesn't
matter for office formats — they all go through the native parser.

## Caveats

- Embedded objects (OLE), legacy `.doc`/`.ppt`/`.xls`, password-protected
  files: not supported. Convert to OOXML first.
- Equation Editor 3.0 (very old Word equations): not converted. OMML works.
- VBA macros and form fields: ignored.
