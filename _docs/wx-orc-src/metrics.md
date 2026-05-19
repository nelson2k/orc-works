# Metrics

`Metrics.h` / `Metrics.cpp` — system metric sampling, polled once per
second by the timer in [main.cpp](main.md).

## Sample

```cpp
struct MetricsSample {
    double cpuPct, ramPct, ramUsedGB;
    double gpuPct, vramPct, vramUsedMB, tempC;
    bool   hasGPU;
};
```

`MetricsCollector::collect()` returns one sample.

## CPU

`GetSystemTimes(&idle, &kernel, &user)` returns cumulative `FILETIME`
counters. The collector keeps the previous values; busy fraction is

```
(dKernel + dUser - dIdle) / (dKernel + dUser)
```

The first call after construction can't compute a delta, so `cpuPct` stays
0 until the second tick.

## RAM

`GlobalMemoryStatusEx`: `ramUsedGB = (Total - Avail) / 1024³`,
`ramPct = 100 * used / Total`.

## GPU (NVIDIA only)

`readNvidia()` launches `nvidia-smi` with `CREATE_NO_WINDOW` through a
helper `runHidden()`:

```
nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu
           --format=csv,noheader,nounits
```

It reads stdout via a pipe, takes the first line, splits on `,`, trims
whitespace, and `atof`s the four fields into `gpuPct`, `vramUsedMB`,
`vramTotal` (used to derive `vramPct`), and `tempC`. `hasGPU` is only set
if the call returned at least four parsable fields.

If `nvidia-smi` isn't on PATH or returns nothing, `hasGPU` stays false and
the GPU/VRAM/TEMP bars render as `n/a` (see [main.cpp:OnMetricsTick](main.md)).

## Cost

Every tick shells out to `nvidia-smi`, which is moderately expensive
(~50–200 ms). It runs on the UI thread inside `OnMetricsTick` — fine at
1 Hz but don't crank the timer faster.
