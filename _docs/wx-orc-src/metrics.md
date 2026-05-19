# Metrics

Five vertical bars on the left: CPU, RAM, GPU, VRAM, TEMP.
Updated by a 1 Hz timer in `MainFrame`.

## Bar widget

[`VBar`](../../wx-ocr-src/src/VBar.h) is a `wxPanel` painted by hand
([VBar.cpp:26-69](../../wx-ocr-src/src/VBar.cpp#L26-L69)):

- Center-top: the label ("12%", "4.1G", "55°", "n/a").
- Center-bottom: the name ("CPU", "RAM", …).
- Between them: a dark track with a colored fill, height proportional
  to `frac_` (clamped to `[0, 1]`).

Constructed with one color per metric
([main.cpp:249-253](../../wx-ocr-src/src/main.cpp#L249-L253)):
CPU blue, RAM green, GPU orange, VRAM purple, TEMP red.

## Tick handler

[`MainFrame::OnMetricsTick`](../../wx-ocr-src/src/main.cpp#L403-L419):

```cpp
MetricsSample s;
if (!(worker_.mode() == Worker::Mode::Remote && worker_.getRemoteMetrics(s))) {
    s = collector_.collect();
}
// push into the 5 VBars
```

So in Remote mode, the bars show the *remote* host whenever the
metrics cache has data; otherwise (Local mode, or Remote with no cached
sample yet) they show the local host.

## Local collection

[`MetricsCollector::collect`](../../wx-ocr-src/src/Metrics.cpp#L99-L145):

- **CPU**: `GetSystemTimes` for idle/kernel/user FILETIMEs, compute a
  delta over the previous sample, busy = `(total − idle) / total`.
- **RAM**: `GlobalMemoryStatusEx`. `used = total − avail`.
- **GPU/VRAM/temp**: shells out to
  `nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu
   --format=csv,noheader,nounits` via [`runHidden`](../../wx-ocr-src/src/Metrics.cpp#L23-L57)
  (pipe + `CREATE_NO_WINDOW`). Parses the first CSV line.

If nvidia-smi isn't available or fails, `hasGPU = false` and the GPU
bars render "n/a".

## Remote collection

Two paths feed `Worker::storeRemoteMetrics`
([Worker.cpp:230-245](../../wx-ocr-src/src/Worker.cpp#L230-L245)):

### SSE during an active request

The Python worker has a daemon thread
[`_metrics_loop`](../../wx-ocr-src/worker.py#L128-L144) that wakes when
`_metrics_active` is set (i.e. inside a request) and emits one
`type:"metrics"` event per second via
[`_emit_metrics_sample`](../../wx-ocr-src/worker.py#L105-L125). It uses
`psutil` for CPU/RAM and the same nvidia-smi shell-out for GPU.

In Remote mode these arrive inline on the SSE stream. The C++ SSE
parse loop in `requestRemote` routes any `type:"metrics"` event into
`storeRemoteMetrics`
([Worker.cpp:548-552](../../wx-ocr-src/src/Worker.cpp#L548-L552)).

### Polled GET while idle

Between requests, the worker thread isn't emitting. So
[`pollRemoteMetricsHttp`](../../wx-ocr-src/src/Worker.cpp#L578-L587)
runs `GET /v1/metrics` every 2 s on a background thread that's started
by `setMode(Remote)`. The endpoint
[`http_metrics`](../../wx-ocr-src/worker.py#L1047-L1069) returns the
same JSON shape as the SSE event.

## The sample struct

[`MetricsSample`](../../wx-ocr-src/src/Metrics.h#L3-L12):

```cpp
double cpuPct, ramPct, ramUsedGB;
double gpuPct, vramPct, vramUsedMB, tempC;
bool hasGPU;
```

`storeRemoteMetrics` deserializes the JSON into this struct so the
local/remote paths converge on the same downstream code.
