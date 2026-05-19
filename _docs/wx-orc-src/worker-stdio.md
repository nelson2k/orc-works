# Local-mode JSON protocol

When the backend dropdown is set to Local, orcgui spawns
`venv\Scripts\python.exe worker.py` and talks to it over two anonymous
pipes. One JSON object per line.

## Transport

- Parent → child: writes to child stdin, one JSON object per line
  ([Worker.cpp:320-326](../../wx-ocr-src/src/Worker.cpp#L320-L326)).
- Child → parent: writes to FD 1 (the protocol stream — see below),
  one JSON object per line. Parent buffers and splits on `\n`
  ([Worker.cpp:328-342](../../wx-ocr-src/src/Worker.cpp#L328-L342)).

## Why the worker dup's FD 1

Native libs (MuPDF, libcms, …) sometimes write warnings straight to FD 1.
That would corrupt the JSON line protocol. The worker does this at
startup ([worker.py:29-38](../../wx-ocr-src/worker.py#L29-L38)):

1. `_proto_fd = os.dup(1)` — keep a private FD for protocol writes.
2. `dup2(NUL, 1)` — replace real FD 1 with NUL so leaks are swallowed.
3. `sys.stdout = open(os.devnull, "w")` — same for Python's wrapper.

`send()` writes through the dup'd FD via `_proto_stream`, behind a
`threading.Lock`.

## Commands (parent → child)

```jsonc
{ "cmd": "render", "path": "...", "page": 0, "dpi": 120 }
{ "cmd": "ocr",    "path": "...", "page": 0, "engine": "auto" }
{ "cmd": "quit" }
```

Dispatched by [`_dispatch_cmd`](../../wx-ocr-src/worker.py#L926-L942).
The accepted engines: `auto`, `digital`, `marker`, `marker_llm`, `vlm`,
`mineru`. See [engines.md](engines.md).

`quit` is sent by [`shutdownLocal`](../../wx-ocr-src/src/Worker.cpp#L381-L398)
on app close.

## Messages (child → parent)

Streamed during a request:

```jsonc
{ "type": "progress", "kind": "stage", "name": "loading_models" }
{ "type": "progress", "kind": "tqdm",  "desc": "...", "n": 5, "total": 12, "event": "tick" }
{ "type": "progress", "kind": "image", "page": 0, "png_base64": "..." }
{ "type": "metrics", "cpu_pct": 12, "ram_pct": 41, ... }
```

Final message per request (one of):

```jsonc
{ "type": "text",  "engine": "marker", "page": 0, "text": "...", "saved_to": "..." }
{ "type": "image", "page": 0, "pages": 27, "png_base64": "..." }   // for render
{ "type": "error", "message": "...", "traceback": "..." }
```

Parser side: [`Worker::requestLocal`](../../wx-ocr-src/src/Worker.cpp#L344-L379)
keeps reading lines, treats `progress` as a callback and `metrics` as a
metrics-cache update, and returns on the first non-progress/non-metrics
JSON line.

## tqdm patch

[`_EventTqdm`](../../wx-ocr-src/worker.py#L178-L197) replaces
`tqdm.tqdm` *before* marker/surya import. Every `update()`/`close()`
re-emits the current state as a `progress kind:"tqdm"` event with
`event` ∈ {`start`,`tick`,`end`}, and the underlying tqdm output is
redirected to `os.devnull` so it can't leak.

## Cancellation

`Worker::cancelLocal()` calls `TerminateProcess`, waits 1s, closes the
pipe handles, and resets state
([Worker.cpp:400-420](../../wx-ocr-src/src/Worker.cpp#L400-L420)).
The next request lazily restarts a fresh subprocess. Not cooperative —
any half-loaded model is dropped on the floor.
