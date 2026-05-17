# Marker sidecar

FastAPI server at [tauri-orc-src/marker-code/](../../tauri-orc-src/marker-code/) that wraps `marker.converters.pdf.PdfConverter`. Spawned by the Rust shell on port 7423; the React app talks to it over plain HTTP.

| File | Role |
| --- | --- |
| [server.py](../../tauri-orc-src/marker-code/server.py) | Two routes: `GET /health` → `{"status": "ok"}`, and `POST /convert` accepting an `UploadFile` plus form fields `page_range` / `model` / `no_llm` / `full_vram`. Translates the fields into a marker `ConfigParser` config, builds a fresh `PdfConverter` per request (sharing a lazy-loaded `_artifact_dict` of surya models), wires the OpenAI LLM service when `no_llm=false`, applies the low-VRAM batch-size preset when `full_vram=false`, runs the converter, and returns `{"markdown": <text>}`. |
| [requirements-server.txt](../../tauri-orc-src/marker-code/requirements-server.txt) | `fastapi>=0.110`, `uvicorn[standard]>=0.27`, `python-multipart>=0.0.9`. `marker` itself is installed separately into the venv. |
| [local.env](../../tauri-orc-src/marker-code/local.env) | `OPENAI_API_KEY=...` and `TORCH_DEVICE=cuda`. Gitignored (`local.env` + `*.env` rules); never committed. **Not currently loaded by `server.py`** — see [gaps.md](gaps.md). |
| `venv/` | Local Python virtualenv (path: `marker-code/venv/`, not `.venv`). Holds marker + its deps. |

## Form fields

| Field | Type | Meaning |
| --- | --- | --- |
| `pdf` | `UploadFile` | The PDF to convert. |
| `page_range` | string | Marker's CLI syntax (e.g. `"0-9"` or `"0,5-10,20"`). Empty string = all pages. Parsed by marker's `ConfigParser.parse_range_str`. |
| `model` | string | OpenAI model name (default `gpt-4o-mini`). Passed as `openai_model`. Only used when `no_llm=false`. |
| `no_llm` | `"true"`/`"false"` | When false, the server wires `marker.services.openai.OpenAIService` and reads `OPENAI_API_KEY` from the env. |
| `full_vram` | `"true"`/`"false"` | When false (the default), applies the low-VRAM batch-size preset (see below). |

## Low-VRAM preset

Matches the legacy CLI runner — safe for a 6 GB card:

| Config key | Value |
| --- | --- |
| `layout_batch_size` | 6 |
| `detection_batch_size` | 2 |
| `recognition_batch_size` | 16 |
| `table_rec_batch_size` | 4 |
| `equation_batch_size` | 4 |

Sending `full_vram=true` skips the preset and lets marker / surya use its CUDA defaults (which on a 12 GB card like the RTX 4070 lift batch sizes ~3×).

## LLM mode

`no_llm=false` requires `OPENAI_API_KEY` in the process env. The server builds a config with `use_llm=true`, `openai_api_key=<env>`, `openai_model=<form field>`, and passes `llm_service="marker.services.openai.OpenAIService"` into `PdfConverter`. Missing key → HTTP 500 with a pointer to `marker-code/local.env`.

## Model caching

`get_artifact_dict()` lazy-loads the 5 surya models into a module-global dict on first `/convert`. Subsequent requests reuse the same dict. A new `PdfConverter` is constructed per request so per-request config (page range, batch sizes, LLM toggle) can vary cheaply.

This means uvicorn boot is fast (the `/health` check returns immediately) but the first `/convert` can take minutes on a fresh install while surya weights download to `~/.cache/datalab/datalab/models/`.
