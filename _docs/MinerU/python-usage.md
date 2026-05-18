# Python usage

`mineru` is a library too. The high-level entry points called by both the CLI and the FastAPI server live in [mineru/cli/common.py](../../repos-folder/MinerU/mineru/cli/common.py).

## High-level functions

```python
from mineru.cli.common import do_parse, aio_do_parse, read_fn
```

- `do_parse(...)` — synchronous wrapper around the backend pipeline. Takes input bytes/path, lang, backend, method, formula/table toggles, etc.
- `aio_do_parse(...)` — `async` counterpart.
- `read_fn(...)` — reads an input file or URI into bytes (handles PDF, images, office formats).

## Backend-level entry points

```python
# Pipeline
from mineru.backend.pipeline.pipeline_analyze import doc_analyze
# VLM
from mineru.backend.vlm.vlm_analyze import doc_analyze, aio_doc_analyze
# Hybrid (requires `pipeline` extras)
from mineru.backend.hybrid.hybrid_analyze import doc_analyze, aio_doc_analyze
# Office (no model)
from mineru.backend.office.docx_analyze import office_docx_analyze
from mineru.backend.office.pptx_analyze import office_pptx_analyze
from mineru.backend.office.xlsx_analyze import office_xlsx_analyze
```

Each `doc_analyze` returns the per-document "middle JSON" structure (and side artifacts). The renderer functions live in `*_middle_json_mkcontent.py`:

```python
from mineru.backend.pipeline.pipeline_middle_json_mkcontent import union_make as pipeline_union_make
from mineru.backend.vlm.vlm_middle_json_mkcontent      import union_make as vlm_union_make
from mineru.backend.office.office_middle_json_mkcontent import union_make as office_union_make
```

`union_make(middle_json, make_mode, ...)` returns one of:

| `MakeMode` | Returns |
|---|---|
| `MM_MD`           | The Markdown string |
| `MM_MD_PURE`      | Markdown with image refs stripped |
| `MM_STANDARD_FORMAT` / `MM_CONTENT_LIST` | The `content_list` JSON list |

`MakeMode` is in `mineru/utils/enum_class.py`.

## Minimal pipeline example

```python
from pathlib import Path
from mineru.cli.common import do_parse, read_fn
from mineru.data.data_reader_writer import FileBasedDataWriter

writer = FileBasedDataWriter("./output")
pdf_bytes = read_fn(Path("input.pdf"))

do_parse(
    output_dir=Path("./output"),
    pdf_file_names=["input"],
    pdf_bytes_list=[pdf_bytes],
    p_lang_list=["en"],
    backend="pipeline",
    parse_method="auto",
    formula_enable=True,
    table_enable=True,
)
```

## VLM via HTTP client

Skips local model loading entirely; useful when you already have an OpenAI-compatible VLM server (e.g. `mineru-vllm-server` on a separate machine).

```python
do_parse(
    output_dir=Path("./output"),
    pdf_file_names=["input"],
    pdf_bytes_list=[pdf_bytes],
    p_lang_list=["en"],
    backend="vlm-http-client",
    server_url="http://my-vllm-host:30000/v1",
)
```

## Threading

`pipeline_analyze.py` is thread-safe (per `ModelSingleton`), and 3.0.0 advertises multi-threaded concurrent inference. The FastAPI server does this via an `asyncio.Semaphore` (`MINERU_API_MAX_CONCURRENT_REQUESTS`).

For inside-process concurrency, prefer `aio_do_parse` and gather tasks under an `asyncio.gather` or `asyncio.Queue`.

## Custom data sinks

Replace `FileBasedDataWriter` with your own subclass of `DataWriter` from `mineru.data.data_reader_writer.base` to push outputs into S3, a database, etc. Each artifact (Markdown, middle JSON, content list, images) is written with a relative path; intercept inside `write(path, data)`.
