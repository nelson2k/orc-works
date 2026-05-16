# Config (`marker/config/`)

How CLI flags / JSON files / env vars become the `config` dict that every
builder / processor / converter / renderer reads via `assign_config`.

## `ConfigParser` (`parser.py`)

Wraps a `cli_options: dict` and:

- `common_options(fn)` — a `click` decorator that injects every shared flag
  (`--output_dir`, `--output_format`, `--debug`, `--processors`,
  `--config_json`, `--disable_multiprocessing`, `--disable_image_extraction`,
  `--page_range`, `--converter_cls`, `--llm_service`). Used by every CLI entry
  point.
- `generate_config_dict()` — collapses CLI options into the config dict that
  the converter and friends consume. Handles:
  - `--debug` → enables `debug_pdf_images`, `debug_layout_images`,
    `debug_json`, and sets `debug_data_folder` to the output dir.
  - `--page_range "0,5-10,20"` → `parse_range_str` → `[0, 5, 6, …, 20]`.
  - `--config_json path.json` → merges JSON file into config.
  - `--disable_multiprocessing` → `pdftext_workers=1`.
  - `--disable_image_extraction` → `extract_images=False`.
  - Backfills `gemini_api_key` from `settings.GOOGLE_API_KEY` for backward
    compatibility.
- `get_llm_service()` — returns `None` unless `--use_llm`; otherwise returns a
  dotted class path (default `marker.services.gemini.GoogleGeminiService`).
- `get_renderer()` — maps `--output_format` to one of `MarkdownRenderer`,
  `JSONRenderer`, `HTMLRenderer`, `ChunkRenderer` (returned as a dotted path).
- `get_processors()` / `get_converter_cls()` — parse comma-separated dotted
  paths and resolve to classes via `strings_to_classes`.
- `get_output_folder(filepath)` / `get_base_filename(filepath)` — derive
  per-file output locations.

## `ConfigCrawler` (`crawler.py`)

Walks every subclass of the base classes (`BaseBuilder`, `BaseProcessor`,
`BaseConverter`, `BaseProvider`, `BaseRenderer`, `BaseService`,
`BaseExtractor`) and extracts every `Annotated[type, "help", ...]` attribute
plus its default. Result is a nested dict (`class_config_map`) used by the
printer to render `config --help`.

## `CustomClickPrinter` (`printer.py`)

A `click.Command` subclass. When invoked as `marker_single config --help`,
prints every discovered attribute across every class, deduping shared
attributes — this is the source of truth for "what flags exist."

## Tips

- For one-off behaviour, dump a JSON dict and pass `--config_json path.json`
  rather than maintaining a long flag list.
- Any unknown key in the JSON config is fine: `assign_config` only assigns to
  attributes that exist on the target class, so unrelated flags are silently
  ignored.
- Programmatic use: build the dict yourself and pass `config=…` to the
  converter; you can skip `ConfigParser` entirely.
