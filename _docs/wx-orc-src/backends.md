# Backends: Local vs Remote

The toolbar has a "Local" / "Remote" dropdown. Internally that maps to
[`Worker::Mode`](../../wx-ocr-src/src/Worker.h#L23):

```cpp
enum class Mode { Local, Remote };
```

## How the toggle works

[main.cpp::OnBackendChange()](../../wx-ocr-src/src/main.cpp#L605-L614)
calls `worker_.setMode(m)`.
[`Worker::setMode`](../../wx-ocr-src/src/Worker.cpp#L191-L208) does:

1. `shutdown()` — tears down whatever transport is currently owned (only
   the Local subprocess is "owned" — the Remote FastAPI worker has its own
   systemd lifecycle).
2. Cancels any in-flight remote request (`cancelRemote()`).
3. Stops the remote metrics polling thread.
4. Flips `mode_`.
5. If switching to Remote, kicks off
   [`startRemoteMetricsPolling`](../../wx-ocr-src/src/Worker.cpp#L210-L221).

The next `request()` call lazily relaunches the right transport.

## Local mode

orcgui.exe owns the Python subprocess.

- `ensureStartedLocal()` creates two anonymous pipes, marks the parent's
  ends non-inheritable, redirects child stderr to NUL, and `CreateProcessW`'s
  `venv\Scripts\python.exe worker.py` with `CREATE_NO_WINDOW`.
  ([Worker.cpp:258-318](../../wx-ocr-src/src/Worker.cpp#L258-L318))
- All commands are JSON, one line each, going to child stdin.
- All responses + progress events are JSON, one line each, coming back
  on a FD-1 dup the worker created before any native lib could printf
  on real stdout. See [worker-stdio.md](worker-stdio.md) and
  [gotchas.md](gotchas.md).

## Remote mode

orcgui.exe is just a WinHTTP client.

- Base URL is `OCR_REMOTE_HTTP_URL` env var, default
  `http://192.168.10.200:9000`
  ([Worker.cpp:165-167](../../wx-ocr-src/src/Worker.cpp#L165-L167)).
- `render` → blocking POST to `/v1/render`, response is the same JSON
  shape the Local mode returns
  ([Worker.cpp:436-446](../../wx-ocr-src/src/Worker.cpp#L436-L446)).
- `ocr` → POST to `/v1/ocr`, response is SSE (one `data: {...}` per
  line, blank-line-terminated events)
  ([Worker.cpp:448-564](../../wx-ocr-src/src/Worker.cpp#L448-L564)).

See [worker-http.md](worker-http.md) for the server side.

## File transfer (scp)

The remote worker reads the PDF from its own filesystem, so orcgui has
to copy it over first. [`PathForWorker()`](../../wx-ocr-src/src/main.cpp#L570-L603)
short-circuits in Local mode (returns the UTF-8 local path). In Remote
mode, it:

1. Hashes the open PDF against `remoteUploadedFor_` — if already uploaded,
   returns the cached remote path.
2. Otherwise builds a sanitized basename (spaces and shell-unsafe chars
   become `_`) and runs `scp.exe -q <local> <target>:/tmp/ocr-works-<base>`.
3. Target defaults to `nelson@192.168.10.200`, overridable via
   `OCR_REMOTE_SSH_TARGET`
   ([main.cpp:546-547](../../wx-ocr-src/src/main.cpp#L546-L547)).
4. Caches `(localPath → remotePath)` so the upload only happens once per
   Open.

Changing backend resets the cache
([main.cpp:611-613](../../wx-ocr-src/src/main.cpp#L611-L613)).

## Metrics flow

- **Local**: `MetricsCollector` samples the host every 1s (toolbar
  timer); the worker also streams `type:"metrics"` events on its
  stdout — orcgui parses them and stashes them, but in Local mode
  `OnMetricsTick` prefers the locally-collected sample.
- **Remote**: `OnMetricsTick` calls `worker_.getRemoteMetrics()` first;
  when that returns true, the bars show the 4070's CPU/RAM/GPU/VRAM/temp.
  Samples come from two sources:
  - SSE `type:"metrics"` events streamed inside an active `/v1/ocr` request
    ([worker.py:105-125](../../wx-ocr-src/worker.py#L105-L125))
  - `GET /v1/metrics` polled every 2s by a background thread while idle
    ([Worker.cpp:578-587](../../wx-ocr-src/src/Worker.cpp#L578-L587))

See [metrics.md](metrics.md).
