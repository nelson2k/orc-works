# Remote-mode FastAPI worker

`worker.py --http` starts a FastAPI app (uvicorn) on
`OCR_HTTP_HOST:OCR_HTTP_PORT` (defaults `0.0.0.0:9000`).
([worker.py:1074-1078](../../wx-ocr-src/worker.py#L1074-L1078))

In production this runs on a 4070 box at `192.168.10.200:9000` under
systemd as `ocr-worker.service`.

## Endpoints

[`_build_http_app`](../../wx-ocr-src/worker.py#L982-L1071) wires up:

| Method + path | Behavior |
| --- | --- |
| `GET  /v1/health`  | `{"status":"ok"}` |
| `POST /v1/render`  | Blocking JSON in / JSON out. Body is `{path, page, dpi}`. Response is the same `{type:"image", page, pages, png_base64}` shape Local emits. |
| `POST /v1/ocr`     | SSE stream. Body is `{path, page, engine, use_llm?}`. Response yields one `data: <json>\n\n` event per progress / metrics message, then a final `{type:"text",...}` or `{type:"error",...}`. |
| `GET  /v1/metrics` | One-shot host snapshot: `{cpu_pct, ram_pct, ram_used_gb, has_gpu, gpu_pct, vram_used_mb, vram_total_mb, vram_pct, temp_c}`. |

## SSE event shape

The worker runs `_dispatch_cmd("ocr", body)` on a thread; its `send()`
calls are routed through `_send_ctx.target` (set per request) which
pushes onto an `asyncio.Queue`. The streaming generator pulls from the
queue and emits `data: <json>\n\n` lines until a sentinel.
([worker.py:999-1045](../../wx-ocr-src/worker.py#L999-L1045))

So the SSE wire format is **single-data-line per event**, blank-line
terminated:

```
data: {"type":"progress","kind":"stage","name":"layout"}

data: {"type":"metrics","cpu_pct":18.0,...}

data: {"type":"text","engine":"marker","page":0,"text":"...","saved_to":"..."}

```

## Headers

Response sets `Cache-Control: no-cache` and `X-Accel-Buffering: no` so
proxies don't buffer.
([worker.py:1041-1045](../../wx-ocr-src/worker.py#L1041-L1045))

## Why HTTP mode skips stdout isolation

The dup'd-FD-1 trick from the Local protocol would break uvicorn's
logging — and there's no stdin/stdout protocol to protect anymore. So
`worker.py` only redirects when `--http` is **not** in argv
([worker.py:29-38](../../wx-ocr-src/worker.py#L29-L38)).

## Cancellation

There is **no cooperative cancel protocol**. When the client closes the
HTTP connection mid-request, the SSE generator's `finally` simply
awaits the worker thread to finish naturally; the events drain into a
queue nobody is reading.
([worker.py:1034-1039](../../wx-ocr-src/worker.py#L1034-L1039))

So pressing Stop in Remote mode aborts the *client side* (closing the
WinHTTP request handle to abort the read loop —
[Worker.cpp:566-576](../../wx-ocr-src/src/Worker.cpp#L566-L576)) but the
server continues processing.

## Process lifecycle

Persistence across orcgui sessions is the point: close orcgui, the
worker stays up with models warm.

- The `llama-server` VLM subprocess is started with
  `start_new_session=True` so it's not in the worker's process group,
  and is cleaned up by `atexit.register(_unload_vlm_http)` on a clean
  exit ([worker.py:534-562](../../wx-ocr-src/worker.py#L534-L562),
  [worker.py:752](../../wx-ocr-src/worker.py#L752)).
- Restart the systemd unit to wipe model state.
