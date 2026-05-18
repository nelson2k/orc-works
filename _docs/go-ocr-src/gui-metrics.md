# gui/metrics.go

`metrics.go` samples system telemetry once per second.

CPU and RAM come from `github.com/shirou/gopsutil/v4`:

- `cpu.Percent(0, false)`
- `mem.VirtualMemory()`

GPU metrics come from `nvidia-smi`:

```text
nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu --format=csv,noheader,nounits
```

Only the first GPU row is used. If `nvidia-smi` is missing or parsing fails,
the sample reports `hasGPU=false`, and the GUI shows `n/a` for GPU, VRAM, and
temperature.

`hideWindow` sets `SysProcAttr.HideWindow = true` so the periodic
`nvidia-smi` calls do not flash a console window on Windows.
