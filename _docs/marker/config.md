# Config

[marker/config/parser.py](../../repos-folder/marker/marker/config/parser.py) holds `ConfigParser`. It wraps click options for the CLI and produces the flat `config: dict` that every converter/builder/processor reads via `assign_config`.

## Flow

1. CLI options come in as `kwargs` to `convert_single_cli` ([scripts/convert_single.py](../../repos-folder/marker/marker/scripts/convert_single.py)).
2. `ConfigParser(kwargs).generate_config_dict()` translates a few keys (`debug` → 3 separate debug flags, `page_range` → list of ints, `config_json` → file merge, `disable_multiprocessing` → `pdftext_workers=1`, `disable_image_extraction` → `extract_images=False`) and copies the rest through.
3. `get_converter_cls`, `get_renderer`, `get_processors`, `get_llm_service` resolve full-module-path strings.
4. The converter merges this dict onto every pipeline class via `assign_config` — any `Annotated` field on any class is configurable through this dict.

## Common keys

These are the ones a frontend most likely needs to surface:

| Key | Default | Effect |
|---|---|---|
| `output_format` | `markdown` | `markdown` / `html` / `json` / `chunks` |
| `page_range` | `None` | List of zero-indexed pages (CLI parses `0,5-10,20`) |
| `force_ocr` | `False` | Re-OCR everything; fixes garbled native text |
| `strip_existing_ocr` | `False` | Drop existing OCR layer before re-OCR |
| `redo_inline_math` | `False` | Re-OCR inline math regions |
| `disable_image_extraction` / `extract_images` | `True` extract | Inline `[image]` placeholders instead of saving files |
| `use_llm` | `False` | Enable all LLM processors |
| `llm_service` | gemini | Full module path of an LLM service |
| `block_correction_prompt` | — | Free-form prompt fed to `LLMPageCorrectionProcessor` |
| `converter_cls` | `PdfConverter` | One of the four converters |
| `paginate_output` | `False` | Insert `{N}` + 48 dashes between pages in markdown |
| `force_layout_block` | `None` | Skip layout, treat every page as this block type |
| `keep_chars` | `False` | Preserve per-character info in JSON outputs |
| `disable_multiprocessing` | `False` | Sets `pdftext_workers=1` |
| `disable_tqdm` | `False` | Quiet progress bars |
| `debug` | `False` | Save layout/page images and JSON to `debug_data_folder` |
| `output_dir` | `settings.OUTPUT_DIR` | Where `marker`/`marker_single` writes output |
| `gemini_api_key` / `claude_api_key` / `openai_api_key` / `vertex_project_id` / `azure_*` | — | LLM credentials |

`marker_single --help` (and `config --help`) enumerate every `Annotated` field — pulled by [marker/config/crawler.py](../../repos-folder/marker/marker/config/crawler.py) and [printer.py](../../repos-folder/marker/marker/config/printer.py).

## `--config_json`

Lets you keep a JSON file of overrides and load it in one shot. Merged on top of the CLI dict, so individual flags still win.

## Programmatic use

```python
from marker.config.parser import ConfigParser

cp = ConfigParser({"output_format": "json", "use_llm": True, "page_range": [0, 1, 2]})
config = cp.generate_config_dict()
```

Then feed `config`, `cp.get_processors()`, `cp.get_renderer()`, `cp.get_llm_service()` to `PdfConverter`.
