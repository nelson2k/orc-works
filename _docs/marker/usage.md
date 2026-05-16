# Usage Recipes

## Install

```bash
pip install marker-pdf            # PDFs only
pip install marker-pdf[full]      # +DOCX/XLSX/PPTX/EPUB/HTML via mammoth/weasyprint/...
```

## Single file (CLI)

```bash
marker_single doc.pdf
marker_single doc.pdf --output_format json --page_range 0,5-10
marker_single doc.pdf --use_llm                       # Gemini by default
marker_single doc.pdf --use_llm --llm_service marker.services.claude.ClaudeService \
                     --claude_api_key sk-...
marker_single doc.pdf --force_ocr --redo_inline_math  # highest quality math
marker_single doc.pdf --debug                         # dumps debug data into output dir
```

## Batch (CLI)

```bash
marker /path/to/input_folder --output_dir out --workers 4
NUM_DEVICES=4 NUM_WORKERS=15 marker_chunk_convert pdf_in md_out  # multi-GPU
```

## Single file (Python)

```python
from marker.converters.pdf import PdfConverter
from marker.models import create_model_dict
from marker.output import text_from_rendered

converter = PdfConverter(artifact_dict=create_model_dict())
rendered = converter("doc.pdf")
text, ext, images = text_from_rendered(rendered)   # ext is "md"
```

## With custom config

```python
from marker.config.parser import ConfigParser
from marker.converters.pdf import PdfConverter
from marker.models import create_model_dict

cfg = ConfigParser({"output_format": "json", "use_llm": True})
converter = PdfConverter(
    config=cfg.generate_config_dict(),
    artifact_dict=create_model_dict(),
    processor_list=cfg.get_processors(),
    renderer=cfg.get_renderer(),
    llm_service=cfg.get_llm_service(),
)
rendered = converter("doc.pdf")
```

## Tables only

```bash
marker_single doc.pdf --use_llm \
  --converter_cls marker.converters.table.TableConverter \
  --output_format json
# add --force_layout_block Table if the entire PDF is tables (e.g. scanned forms)
```

## OCR only (with character bboxes)

```bash
marker_single doc.pdf \
  --converter_cls marker.converters.ocr.OCRConverter --keep_chars
```

## Structured extraction (beta)

```python
from pydantic import BaseModel
class Links(BaseModel):
    links: list[str]

from marker.config.parser import ConfigParser
cfg = ConfigParser({"page_schema": Links.model_json_schema(), "use_llm": True})

from marker.converters.extraction import ExtractionConverter
from marker.models import create_model_dict
converter = ExtractionConverter(
    artifact_dict=create_model_dict(),
    config=cfg.generate_config_dict(),
    llm_service=cfg.get_llm_service(),
)
out = converter("doc.pdf")
print(out.document_json)
```

## Walking blocks

```python
from marker.converters.pdf import PdfConverter
from marker.models import create_model_dict
from marker.schema import BlockTypes

converter = PdfConverter(artifact_dict=create_model_dict())
document = converter.build_document("doc.pdf")
forms = document.contained_blocks((BlockTypes.Form,))
```

## API server

```bash
pip install -U uvicorn fastapi python-multipart
marker_server --port 8001          # docs at localhost:8001/docs
```
