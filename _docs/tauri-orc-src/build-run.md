# Build and run

All commands run from inside [tauri-orc-src/](../../tauri-orc-src/).

## First-time setup

JS deps:

```
cd tauri-orc-src
npm install
```

Python sidecar venv (mirrors the rtx-4070 runbook in [../rtx-4070.txt](../rtx-4070.txt)):

```
cd marker-code
python -m venv venv
venv\Scripts\Activate.ps1
pip install --upgrade pip
pip install torch torchvision --index-url https://download.pytorch.org/whl/cu128
pip install marker-pdf[full] python-dotenv openai
pip install -r requirements-server.txt
```

Drop an OpenAI key into `marker-code/local.env` if you plan to use the LLM path:

```
OPENAI_API_KEY=sk-proj-...
TORCH_DEVICE=cuda
```

Today `server.py` does not auto-load this file; you have to export the env vars manually before starting the Tauri app or `uvicorn` (see [gaps.md](gaps.md)).

## Dev loop

```
npm run tauri dev
```

That triggers `beforeDevCommand: npm run dev` (Vite on port 1420), then `cargo tauri dev` opens a window pointed at `http://localhost:1420`. The React app's `useEffect` calls `marker_start` over Tauri IPC, the Rust side spawns `uvicorn server:app` on `127.0.0.1:7423`, the frontend polls `/health` until ready, and the convert button lights up.

## Production build

```
npm run build      # tsc && vite build → tauri-orc-src/dist/
npm run tauri build
```

`cargo tauri build` packages the app per `tauri.conf.json` (`bundle.targets: "all"`). The frontend is served from `dist/` via the Tauri shell; Vite is not in the loop at runtime.

The bundle ships the React app but not `marker-code/venv/` — the marker venv is too large and CUDA-specific to package. Plan for a separate install step for the Python side.

## Standalone marker server (without Tauri)

Useful for debugging the sidecar in isolation:

```
cd marker-code
venv\Scripts\python.exe -m uvicorn server:app --host 127.0.0.1 --port 7423
```

Then `curl http://127.0.0.1:7423/health` or POST a PDF to `/convert` directly.

## Resetting

```
# Re-install JS deps
rm -rf tauri-orc-src/node_modules
cd tauri-orc-src && npm install

# Re-install Python deps
rm -rf tauri-orc-src/marker-code/venv
# then redo the venv steps above
```

The marker model cache (`~/.cache/datalab/datalab/models/`, ~3 GB) is also safe to delete and will re-download on next run.
