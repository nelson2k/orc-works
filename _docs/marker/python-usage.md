# Python usage

Minimal example:

```python
from marker.converters.pdf import PdfConverter
from marker.models import create_model_dict
from marker.output import text_from_rendered

converter = PdfConverter(artifact_dict=create_model_dict())
rendered = converter("/path/to/file.pdf")
text, ext, images = text_from_rendered(rendered)
```

`text_from_rendered` returns `(str, "md"|"html"|"json", {filename: PIL.Image})`. See [marker/output.py](../../repos-folder/marker/marker/output.py).

## With explicit config

```python
from marker.converters.pdf import PdfConverter
from marker.models import create_model_dict
from marker.config.parser import ConfigParser

cp = ConfigParser({
    "output_format": "json",
    "use_llm": True,
    "page_range": [0, 1, 2],
    "gemini_api_key": "...",
})

converter = PdfConverter(
    config=cp.generate_config_dict(),
    artifact_dict=create_model_dict(),
    processor_list=cp.get_processors(),
    renderer=cp.get_renderer(),
    llm_service=cp.get_llm_service(),
)
rendered = converter("/path/to/file.pdf")
```

## Re-use models across files

`create_model_dict()` is expensive (loads 5 surya predictors onto GPU). Build it once, reuse across many `PdfConverter()` instances. Sharing the *same* `PdfConverter` across calls is also fine — `__call__` is stateless apart from `page_count`.

## In-memory input

`PdfConverter.__call__` accepts `str` *or* `io.BytesIO`. `filepath_to_str` will dump the BytesIO to a temp file and clean up.

```python
from io import BytesIO
rendered = converter(BytesIO(pdf_bytes))
```

## Block-level manipulation

```python
from marker.schema import BlockTypes

document = converter.build_document("/path/to/file.pdf")
forms = document.contained_blocks((BlockTypes.Form,))
tables = document.contained_blocks((BlockTypes.Table,))
```

`build_document` returns the unrendered `Document`. You can mutate it (override blocks, drop pages, etc.) then call `converter.renderer(document)` yourself.

## Tables only

```python
from marker.converters.table import TableConverter

tc = TableConverter(artifact_dict=create_model_dict(),
                    config={"output_format": "json"})
rendered = tc("/path/to/file.pdf")
```

## OCR only

```python
from marker.converters.ocr import OCRConverter

oc = OCRConverter(artifact_dict=create_model_dict(),
                  config={"keep_chars": True})
rendered = oc("/path/to/scan.pdf")
```
