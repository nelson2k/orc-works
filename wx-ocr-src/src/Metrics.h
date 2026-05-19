#pragma once

struct MetricsSample {
    double cpuPct = 0;
    double ramPct = 0;
    double ramUsedGB = 0;
    double gpuPct = 0;
    double vramPct = 0;
    double vramUsedMB = 0;
    double tempC = 0;
    bool hasGPU = false;
};

class MetricsCollector {
public:
    MetricsCollector();
    MetricsSample collect();

private:
    unsigned long long lastIdle_ = 0;
    unsigned long long lastKernel_ = 0;
    unsigned long long lastUser_ = 0;
    bool haveLastCpu_ = false;
};
