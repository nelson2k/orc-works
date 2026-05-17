# Scripts / CLI

[marker/scripts/](../../repos-folder/marker/marker/scripts/). Installed as console entry points by `pyproject.toml`:

| Command | Module | What it does |
|---|---|---|
| `marker_single` | [convert_single.py](../../repos-folder/marker/marker/scripts/convert_single.py) | Convert one file, write output to `output_dir/<basename>/`. |
| `marker` | [convert.py](../../repos-folder/marker/marker/scripts/convert.py) | Convert every file in a folder, multi-worker. `--workers` autodetected from VRAM. |
| `marker_chunk_convert` | [chunk_convert.py](../../repos-folder/marker/marker/scripts/chunk_convert.py) | Multi-GPU sharded folder conversion via `NUM_DEVICES`/`NUM_WORKERS` env vars. Wraps the `.sh` script. |
| `marker_gui` | [run_streamlit_app.py](../../repos-folder/marker/marker/scripts/run_streamlit_app.py) | Streamlit playground at `streamlit_app.py`. Needs `streamlit streamlit-ace`. |
| `marker_extract` | same module | Streamlit playground for structured extraction at `extraction_app.py`. |
| `marker_server` | [server.py](../../repos-folder/marker/marker/scripts/server.py) | FastAPI server with a single `/marker` POST endpoint. `--port 8001`. Needs `uvicorn fastapi python-multipart`. |

`marker_single FOO.pdf` is the smallest possible invocation. All four conversion commands share `ConfigParser.common_options` for the bulk of their flags.

## API server

```
pip install -U uvicorn fastapi python-multipart
marker_server --port 8001
```

POST to `http://localhost:8001/marker` with JSON body `{ "filepath": "..." }` plus any of the common options. Returns the rendered output and metadata. README explicitly says it isn't robust — fine for local dev / a sidecar process, not for production traffic.

## Custom click printer

[marker/config/printer.py](../../repos-folder/marker/marker/config/printer.py) — `CustomClickPrinter` reflects on every pipeline class to enumerate `Annotated` fields and prints them as a help subcommand: `marker_single config --help`. Useful while building a settings UI — it's basically a free schema.
