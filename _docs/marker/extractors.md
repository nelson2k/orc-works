# Extractors (`marker/extractors/`)

Used only by `ExtractionConverter` (see [converters.md](converters.md)).
They turn the rendered markdown of a document into structured JSON that
matches a user-supplied `page_schema`.

## `BaseExtractor` (`__init__.py`)

Holds an `llm_service` and `max_concurrency=3`. Exposes `extract_image(...)`
for grabbing page images (low-res by default to save tokens). Subclasses must
implement `__call__(document, ...)`.

## Two-stage extraction

### `PageExtractor` (`page.py`)

For each page of markdown:

1. Sends `(page_markdown, page_schema)` to the LLM with a long prompt that
   asks the model to write *detailed notes* (not to fill the schema yet) so
   that values that span pages can be reconciled later.
2. Returns a `PageExtractionSchema(description, detailed_notes)` per page.

Tunables: `extraction_page_chunk_size=3`, `page_schema` (string JSON schema).

### `DocumentExtractor` (`document.py`)

Receives the list of per-page `PageExtractionSchema` notes and asks the LLM
to produce the final `DocumentExtractionSchema(analysis, document_json)` —
the actual structured output.

## Flow inside `ExtractionConverter.__call__`

1. Force `paginate_output=True` and `output_format=markdown`.
2. Run the PDF pipeline to get markdown (or skip if `existing_markdown` is
   supplied).
3. Split markdown on `{n}-{48 dashes}` page separator.
4. Resolve the LLM service (default Gemini) if not already set.
5. Run `PageExtractor(pages)` and then `DocumentExtractor(notes)`.
6. `ExtractionRenderer` wraps the analysis + JSON + original markdown into
   `ExtractionOutput`.

## Quick recipe

```python
from pydantic import BaseModel
from marker.converters.extraction import ExtractionConverter
from marker.models import create_model_dict
from marker.config.parser import ConfigParser

class Item(BaseModel):
    name: str
    price: float

cfg = ConfigParser({"page_schema": Item.model_json_schema(), "use_llm": True})
converter = ExtractionConverter(
    artifact_dict=create_model_dict(),
    config=cfg.generate_config_dict(),
    llm_service=cfg.get_llm_service(),
)
out = converter("invoice.pdf")  # out.document_json is your JSON
```
