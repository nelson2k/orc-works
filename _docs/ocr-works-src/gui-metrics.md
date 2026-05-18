# gui/metrics.go

Single-sample system telemetry. Called once per second from
`runMetricsLoop` in [main.go](gui-main.md).

## sample struct (lines 13–22)

Plain value type, returned by `collect()`. Fields:

| field        | meaning                          |
|--------------|----------------------------------|
| `cpuPct`     | CPU utilization (0–100)          |
| `ramPct`     | system RAM used %                |
| `ramUsedGB`  | system RAM used (GiB)            |
| `gpuPct`     | NVIDIA GPU utilization (0–100)   |
| `vramPct`    | VRAM used % (derived)            |
| `vramUsedMB` | VRAM used (MiB)                  |
| `tempC`      | GPU temperature (°C)             |
| `hasGPU`     | whether nvidia-smi succeeded     |

## collect() (lines 24–45)

- CPU: `cpu.Percent(0, false)` — interval=0 means "diff since last
  call". The first call is meaningless, so `main` primes the loop
  with a throw-away `collect()` before the ticker starts.
- RAM: `mem.VirtualMemory()`.
- GPU: optional, via `readNvidia()`. Missing GPU is not an error —
  `hasGPU` stays false and the bars show `n/a`.

## readNvidia (lines 54–86)

Shells out to:

```
nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu \
           --format=csv,noheader,nounits
```

Parses the first newline-delimited row (multi-GPU systems are
collapsed to the first card). Any parse failure returns `ok=false`.

## hideWindow (lines 88–90)

Sets `SysProcAttr.HideWindow = true` so launching `nvidia-smi` every
second doesn't flash a console window on Windows. Same import
(`syscall`) is the reason this file is Windows-flavored — on non-
Windows builds you'd want a build tag, but the project currently
targets Windows only.

## Failure modes

- `nvidia-smi` not on PATH → `exec.Command` errors → `hasGPU=false`.
- AMD/Intel GPU → no NVML, same outcome.
- gopsutil's CPU/RAM calls return zero values silently on permission
  errors; the bars will read 0 % rather than panic.
