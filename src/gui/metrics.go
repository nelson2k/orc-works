package main

import (
	"os/exec"
	"strconv"
	"strings"
	"syscall"

	"github.com/shirou/gopsutil/v4/cpu"
	"github.com/shirou/gopsutil/v4/mem"
)

type sample struct {
	cpuPct     float64
	ramPct     float64
	ramUsedGB  float64
	gpuPct     float64
	vramPct    float64
	vramUsedMB float64
	tempC      float64
	hasGPU     bool
}

func collect() sample {
	var s sample

	if pcts, err := cpu.Percent(0, false); err == nil && len(pcts) > 0 {
		s.cpuPct = pcts[0]
	}
	if vm, err := mem.VirtualMemory(); err == nil {
		s.ramPct = vm.UsedPercent
		s.ramUsedGB = float64(vm.Used) / (1024 * 1024 * 1024)
	}

	if gpu, ok := readNvidia(); ok {
		s.hasGPU = true
		s.gpuPct = gpu.util
		if gpu.memTotal > 0 {
			s.vramPct = 100 * gpu.memUsed / gpu.memTotal
		}
		s.vramUsedMB = gpu.memUsed
		s.tempC = gpu.temp
	}
	return s
}

type nvSample struct {
	util     float64
	memUsed  float64
	memTotal float64
	temp     float64
}

func readNvidia() (nvSample, bool) {
	cmd := exec.Command(
		"nvidia-smi",
		"--query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu",
		"--format=csv,noheader,nounits",
	)
	hideWindow(cmd)
	out, err := cmd.Output()
	if err != nil {
		return nvSample{}, false
	}
	line := strings.TrimSpace(string(out))
	if line == "" {
		return nvSample{}, false
	}
	if i := strings.IndexByte(line, '\n'); i >= 0 {
		line = line[:i]
	}
	parts := strings.Split(line, ",")
	if len(parts) < 4 {
		return nvSample{}, false
	}
	parse := func(s string) float64 {
		f, _ := strconv.ParseFloat(strings.TrimSpace(s), 64)
		return f
	}
	return nvSample{
		util:     parse(parts[0]),
		memUsed:  parse(parts[1]),
		memTotal: parse(parts[2]),
		temp:     parse(parts[3]),
	}, true
}

func hideWindow(cmd *exec.Cmd) {
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
}
