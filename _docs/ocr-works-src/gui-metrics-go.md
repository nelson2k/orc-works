# src/gui/metrics.go

System metrics sampler. Produces one `sample` per call, used by `runMetricsLoop` in [main.go](../../src/gui/main.go) on a 1 s ticker.

## sample

```go
type sample struct {
    cpuPct     float64 // 0..100
    ramPct     float64 // 0..100, used/total
    ramUsedGB  float64
    gpuPct     float64 // 0..100, util.gpu
    vramPct    float64 // 0..100, memory.used / memory.total * 100
    vramUsedMB float64
    tempC      float64 // GPU package temp in °C
    hasGPU     bool
}
```

## Sources

- **CPU**: `github.com/shirou/gopsutil/v4/cpu.Percent(0, false)` — single overall %. Interval 0 returns the delta since the last call, so the loop primes it with one call before the ticker starts.
- **RAM**: `gopsutil/v4/mem.VirtualMemory()` for `UsedPercent` and `Used` (bytes → GB).
- **GPU / VRAM / TEMP**: shell out to `nvidia-smi` with
  `--query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu --format=csv,noheader,nounits`.
  Take the first line, split on commas. On any error (no driver, no GPU) `hasGPU=false` and the three GPU bars display `n/a`.

## Windows-only detail

`hideWindow(cmd)` sets `SysProcAttr{HideWindow: true}` so the `nvidia-smi` shell-out doesn't flash a console window every second. This pins the file to Windows builds — fine for now since the rest of the project (PowerShell launcher, `venv\Scripts\python.exe`) is Windows-only.
