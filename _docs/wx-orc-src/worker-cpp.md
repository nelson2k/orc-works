# Worker.cpp — the transport layer

[`class Worker`](../../wx-ocr-src/src/Worker.h#L19-L98) hides the
backend choice behind one method:

```cpp
nlohmann::json request(const nlohmann::json& req, ProgressCallback onProgress = nullptr);
```

## Mode dispatch

[`request`](../../wx-ocr-src/src/Worker.cpp#L175-L178) branches on
`mode_` and calls `requestLocal` or `requestRemote`. Similarly
`cancel()` and `shutdown()` dispatch — except `shutdown()` only ever
tears down the Local subprocess (we don't own the remote service).

## Local transport

### Subprocess start

[`ensureStartedLocal`](../../wx-ocr-src/src/Worker.cpp#L258-L318)
creates two `CreatePipe` pipes, marks the parent's ends non-inheritable,
opens `NUL` for stderr, and launches:

```
<exe-dir>\..\venv\Scripts\python.exe <exe-dir>\..\worker.py
```

with `CREATE_NO_WINDOW`. The 64 KB `readBuf_` is allocated for line
buffering.

### Line I/O

- [`writeLine`](../../wx-ocr-src/src/Worker.cpp#L320-L326): write
  serialized JSON + `\n` to child stdin.
- [`readLine`](../../wx-ocr-src/src/Worker.cpp#L328-L342): drain
  `readAccum_` for a `\n`; otherwise `ReadFile` more bytes and append.
  Strips trailing `\r`.

### Request loop

[`requestLocal`](../../wx-ocr-src/src/Worker.cpp#L344-L379) holds a
mutex (`mu_`) so concurrent requests serialize — orcgui only ever runs
one at a time, but the mutex is there to keep state coherent.

For each inbound line:

- skip empties / non-JSON-looking lines
- parse, route `progress` → callback, `metrics` → metrics cache
- everything else is the response — return it

### Cancel

[`cancelLocal`](../../wx-ocr-src/src/Worker.cpp#L400-L420) calls
`TerminateProcess`, closes both pipe handles, resets `started_` so the
next request relaunches a fresh subprocess.

### Shutdown

[`shutdownLocal`](../../wx-ocr-src/src/Worker.cpp#L381-L398) writes
`{"cmd":"quit"}\n`, closes stdin to signal EOF, waits 3 s for the
subprocess.

## Remote transport

### WinHTTP helpers

The anonymous namespace at the top has a `ParsedUrl` struct +
`WinHttpCrackUrl` wrapper, a unique-ptr handle closer, and two
blocking POST/GET helpers ([Worker.cpp:42-163](../../wx-ocr-src/src/Worker.cpp#L42-L163)).
Base URL is read from `OCR_REMOTE_HTTP_URL` (default
`http://192.168.10.200:9000`).

### `render`

[Worker.cpp:436-446](../../wx-ocr-src/src/Worker.cpp#L436-L446) — one
blocking POST to `/v1/render`, parse the JSON body, done.

### `ocr` — SSE parse loop

[Worker.cpp:448-564](../../wx-ocr-src/src/Worker.cpp#L448-L564) — long
form because we hand-roll SSE on top of WinHTTP:

1. `WinHttpOpen` with a generous 10-minute recv timeout (OCR is slow).
2. `WinHttpSendRequest` + `WinHttpReceiveResponse` for `POST /v1/ocr`.
3. Publish the request handle to `httpActiveReq_` (under a mutex) so
   `cancelRemote` can close it from another thread.
4. Read 16 KB at a time. Append to `accum`. Repeatedly extract complete
   events (`\n\n` separated). Within each event, pick out `data:` lines
   (trim a leading space, join multi-line with `\n` — though our worker
   only emits one data line per event). Parse the JSON.
5. Route `progress` → callback, `metrics` → metrics cache; the first
   other message is the final result and breaks the loop.

The `ClearActive` RAII guard zeroes `httpActiveReq_` on exit.

### `cancelRemote`

[Worker.cpp:566-576](../../wx-ocr-src/src/Worker.cpp#L566-L576) — closes
the live request handle, which causes `WinHttpReadData` /
`WinHttpQueryDataAvailable` to fail. The read loop throws. The remote
worker keeps processing (no cooperative cancel — see
[worker-http.md](worker-http.md)).

## Remote metrics polling

`setMode(Remote)` starts a background thread that runs
[`pollRemoteMetricsHttp`](../../wx-ocr-src/src/Worker.cpp#L578-L587)
every 2s. Each iteration does a 3 s GET `/v1/metrics` and stashes the
parsed JSON into `lastRemoteMetrics_` via
[`storeRemoteMetrics`](../../wx-ocr-src/src/Worker.cpp#L230-L245).
[`stopRemoteMetricsPolling`](../../wx-ocr-src/src/Worker.cpp#L223-L228)
signals via `metricsPollStop_` + condition variable. Best-effort: any
HTTP failure is swallowed (bars freeze briefly if the server's down).

Inside an active `/v1/ocr` request, the SSE stream also delivers
`type:"metrics"` events at ~1 Hz; the same `storeRemoteMetrics` path
consumes them.

## Destructor

`~Worker` ([Worker.cpp:589-592](../../wx-ocr-src/src/Worker.cpp#L589-L592))
stops the metrics polling thread and calls `shutdown()`.
