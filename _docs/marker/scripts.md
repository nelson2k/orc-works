# Scripts and CLI (`marker/scripts/`)

CLI entry points are declared in `pyproject.toml`:

| Command                 | Module                           | What it does |
|-------------------------|----------------------------------|--------------|
| `marker_single`         | `scripts.convert_single:convert_single_cli` | Convert one file with the standard pipeline. |
| `marker`                | `scripts.convert:convert_cli`    | Batch-convert a folder; supports multiprocessing across CPU / GPU workers. |
| `marker_chunk_convert`  | `scripts.chunk_convert:chunk_convert_cli` (also `chunk_convert.sh`) | Shard a folder across `NUM_DEVICES` × `NUM_WORKERS`. |
| `marker_gui`            | `scripts.run_streamlit_app:streamlit_app_cli` | Launches `streamlit_app.py` — interactive UI. |
| `marker_extract`        | `scripts.run_streamlit_app:extraction_app_cli` | Streamlit UI for structured extraction. |
| `marker_server`         | `scripts.server:server_cli`      | Launches the FastAPI server (`server.py`). |

`config --help` (registered via `CustomClickPrinter`, see [config.md](config.md))
prints every builder / processor / converter knob in a single help table.

## `convert_single.py`

Tiny — builds models once, parses CLI options through `ConfigParser`, picks a
converter class, runs it, and writes outputs with `save_output`. Logs total
runtime. Source of truth for "how to drive marker from Python."

## `convert.py`

Batch driver. Sets aggressive thread limits (`OMP_NUM_THREADS=2`,
`MKL_DYNAMIC=FALSE`, etc.) before importing torch, then uses
`torch.multiprocessing` to spread files across worker processes. Each worker
loads its own model dict in `worker_init`. Uses `utils.gpu.GPUManager` and
`utils.batch.get_batch_sizes_worker_counts` to plan throughput across GPUs.

## `chunk_convert.py` / `chunk_convert.sh`

Shell harness — given `NUM_DEVICES` GPUs and `NUM_WORKERS` per GPU, splits the
input folder and launches one `marker` per GPU pinned to a `CUDA_VISIBLE_DEVICES`.

## `server.py`

A small FastAPI service:

- `GET /` — basic HTML index.
- `GET /docs` — auto-generated OpenAPI docs.
- `POST /marker` — runs the standard pipeline on a `filepath` argument; also
  accepts uploads.

Loads the model dict on startup via FastAPI `lifespan`. Designed for small
local use, not production batch.

## Streamlit apps

- `streamlit_app.py` — interactive marker UI; same pipeline as `marker_single`
  with a sidebar for options and live previews.
- `extraction_app.py` — same idea but uses `ExtractionConverter` and lets you
  paste a JSON schema.
- `run_streamlit_app.py` — thin shim used by the CLI entry points so the
  process can be run as `python -m streamlit run <path>`.

## Other utilities

- `file_to_s3.py` — helper for uploading conversion outputs to S3.
- `common.py` — shared helpers across the scripts (option parsing,
  glob expansion, etc.).
