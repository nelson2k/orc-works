# Quickstart

## CLI (one shot)

```
docling https://arxiv.org/pdf/2206.01062
```

Defaults: input format sniffed from file/URL, output format Markdown, dropped in the current directory as `<stem>.md`. No model warm-up needed beyond first run (which downloads layout / OCR weights).

For a VLM-only run:

```
docling --pipeline vlm --vlm-model granite_docling https://arxiv.org/pdf/2206.01062
```

## Python — convert one document

```python
from docling.document_converter import DocumentConverter

source = "https://arxiv.org/pdf/2408.09869"   # path, URL, or DocumentStream
converter = DocumentConverter()
result = converter.convert(source)
print(result.document.export_to_markdown())
```

`result` is a `ConversionResult`. Useful attributes:

- `result.document` — a `DoclingDocument` (from `docling-core`) with `export_to_markdown()`, `export_to_html()`, `export_to_dict()`, `save_as_json()`, `save_as_yaml()`, `save_as_doctags()`, `save_as_vtt()`, `iterate_items()` …
- `result.status` — `ConversionStatus` (`SUCCESS` / `PARTIAL_SUCCESS` / `FAILURE` / `SKIPPED` / `PENDING` / `STARTED`).
- `result.errors` — list of `ErrorItem(component_type, module_name, error_message)`.
- `result.confidence` — `ConfidenceReport` with per-page and aggregated `layout_score`, `parse_score`, `table_score`, `ocr_score`.
- `result.pages` — populated only by paginated pipelines.

## Python — batch convert

```python
from pathlib import Path
from docling.document_converter import DocumentConverter

converter = DocumentConverter()
paths = list(Path("docs/").glob("*.pdf"))
for res in converter.convert_all(paths, max_file_size=20 * 1024 * 1024):
    print(res.document.export_to_markdown()[:120])
```

`convert_all` is a generator. `raises_on_error=True` (default) raises on the first failure; pass `False` to keep going and inspect `res.errors` per item.

## Python — restrict formats / customize options

```python
from docling.datamodel.base_models import InputFormat
from docling.datamodel.pipeline_options import PdfPipelineOptions
from docling.document_converter import DocumentConverter, PdfFormatOption

converter = DocumentConverter(
    allowed_formats=[InputFormat.PDF, InputFormat.DOCX],
    format_options={
        InputFormat.PDF: PdfFormatOption(
            pipeline_options=PdfPipelineOptions(
                do_ocr=True,
                do_table_structure=True,
            )
        ),
    },
)
```

`PdfPipelineOptions` and its siblings (`VlmPipelineOptions`, `AsrPipelineOptions`, `ConvertPipelineOptions`) live in `docling/datamodel/pipeline_options.py`. Every field is `Annotated[..., Field(description=...)]` so `--help` and IDE tooltips are self-documenting.

## Convert a string

```python
from docling.datamodel.base_models import InputFormat
from docling.document_converter import DocumentConverter

result = DocumentConverter().convert_string(
    "# Hello\nSome text.",
    format=InputFormat.MD,
)
```

Only `InputFormat.MD` and `InputFormat.HTML` are accepted by `convert_string`. Strings are wrapped in a `DocumentStream` internally.

## Convert an in-memory stream

```python
from io import BytesIO
from docling.datamodel.base_models import DocumentStream
from docling.document_converter import DocumentConverter

buf = BytesIO(b"<html><body>Hello</body></html>")
stream = DocumentStream(name="page.html", stream=buf)
result = DocumentConverter().convert(stream)
```
