#include "Metrics.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace {

#ifdef _WIN32
unsigned long long ftToU64(const FILETIME& ft) {
    return (static_cast<unsigned long long>(ft.dwHighDateTime) << 32)
           | static_cast<unsigned long long>(ft.dwLowDateTime);
}

bool runHidden(const wchar_t* cmd, std::string& out) {
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    HANDLE rd = nullptr, wr = nullptr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) return false;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = wr;
    si.hStdError = wr;
    si.hStdInput = nullptr;

    PROCESS_INFORMATION pi{};
    std::wstring buf = cmd;
    std::vector<wchar_t> cmdBuf(buf.begin(), buf.end());
    cmdBuf.push_back(L'\0');
    BOOL ok = CreateProcessW(nullptr, cmdBuf.data(), nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(wr);
    if (!ok) { CloseHandle(rd); return false; }

    char chunk[4096];
    DWORD got = 0;
    while (ReadFile(rd, chunk, sizeof(chunk), &got, nullptr) && got > 0) {
        out.append(chunk, got);
    }
    CloseHandle(rd);
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

bool readNvidia(double& util, double& memUsed, double& memTotal, double& temp) {
    std::string out;
    if (!runHidden(L"nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu --format=csv,noheader,nounits", out)) {
        return false;
    }
    auto nl = out.find('\n');
    std::string line = (nl == std::string::npos) ? out : out.substr(0, nl);
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
    if (line.empty()) return false;

    auto parse = [](const std::string& s) {
        return std::atof(s.c_str());
    };

    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= line.size()) {
        auto comma = line.find(',', start);
        std::string tok = line.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        size_t a = tok.find_first_not_of(" \t");
        size_t b = tok.find_last_not_of(" \t");
        if (a == std::string::npos) tok.clear();
        else tok = tok.substr(a, b - a + 1);
        parts.push_back(tok);
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    if (parts.size() < 4) return false;
    util = parse(parts[0]);
    memUsed = parse(parts[1]);
    memTotal = parse(parts[2]);
    temp = parse(parts[3]);
    return true;
}
#endif

}

MetricsCollector::MetricsCollector() {}

MetricsSample MetricsCollector::collect() {
    MetricsSample s;

#ifdef _WIN32
    FILETIME idleFT, kernelFT, userFT;
    if (GetSystemTimes(&idleFT, &kernelFT, &userFT)) {
        unsigned long long idle = ftToU64(idleFT);
        unsigned long long kernel = ftToU64(kernelFT);
        unsigned long long user = ftToU64(userFT);
        if (haveLastCpu_) {
            unsigned long long dIdle = idle - lastIdle_;
            unsigned long long dKernel = kernel - lastKernel_;
            unsigned long long dUser = user - lastUser_;
            unsigned long long total = dKernel + dUser;
            if (total > 0) {
                double busy = (double)(total - dIdle) / (double)total;
                s.cpuPct = busy * 100.0;
            }
        }
        lastIdle_ = idle;
        lastKernel_ = kernel;
        lastUser_ = user;
        haveLastCpu_ = true;
    }

    MEMORYSTATUSEX mem{};
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        double used = (double)(mem.ullTotalPhys - mem.ullAvailPhys);
        s.ramUsedGB = used / (1024.0 * 1024.0 * 1024.0);
        if (mem.ullTotalPhys > 0) {
            s.ramPct = 100.0 * used / (double)mem.ullTotalPhys;
        }
    }

    double util = 0, memUsed = 0, memTotal = 0, temp = 0;
    if (readNvidia(util, memUsed, memTotal, temp)) {
        s.hasGPU = true;
        s.gpuPct = util;
        s.vramUsedMB = memUsed;
        if (memTotal > 0) s.vramPct = 100.0 * memUsed / memTotal;
        s.tempC = temp;
    }
#endif

    return s;
}
