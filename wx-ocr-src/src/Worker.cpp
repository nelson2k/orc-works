#include "Worker.h"

#include <cstring>
#include <stdexcept>

#ifdef _WIN32

namespace {
std::wstring widen(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

// Returns "<exe-dir>\..\" — i.e. the wx-ocr-src/ root when the exe lives
// at wx-ocr-src/build/orcgui.exe.
std::wstring projectRoot() {
    wchar_t buf[MAX_PATH] = {0};
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n == 0 || n == MAX_PATH) return L"..\\";
    std::wstring path(buf, n);
    auto slash = path.find_last_of(L"\\/");
    std::wstring exeDir = (slash == std::wstring::npos) ? L"." : path.substr(0, slash);
    return exeDir + L"\\..\\";
}
}

bool Worker::ensureStarted() {
    if (started_) return true;

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE inRead = nullptr, inWrite = nullptr;
    HANDLE outRead = nullptr, outWrite = nullptr;
    if (!CreatePipe(&inRead, &inWrite, &sa, 0)) return false;
    SetHandleInformation(inWrite, HANDLE_FLAG_INHERIT, 0);
    if (!CreatePipe(&outRead, &outWrite, &sa, 0)) {
        CloseHandle(inRead); CloseHandle(inWrite);
        return false;
    }
    SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0);

    HANDLE nulHandle = CreateFileW(L"NUL", GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ,
                                   &sa, OPEN_EXISTING, 0, nullptr);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = inRead;
    si.hStdOutput = outWrite;
    si.hStdError = nulHandle != INVALID_HANDLE_VALUE ? nulHandle : outWrite;

    PROCESS_INFORMATION pi{};

    std::wstring root = projectRoot();
    std::wstring cmd = L"\"" + root + L"venv\\Scripts\\python.exe\" \""
                       + root + L"worker.py\"";
    std::vector<wchar_t> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back(L'\0');

    BOOL ok = CreateProcessW(
        nullptr, cmdBuf.data(),
        nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW,
        nullptr, nullptr,
        &si, &pi
    );

    CloseHandle(inRead);
    CloseHandle(outWrite);
    if (nulHandle != INVALID_HANDLE_VALUE) CloseHandle(nulHandle);

    if (!ok) {
        CloseHandle(inWrite);
        CloseHandle(outRead);
        return false;
    }

    CloseHandle(pi.hThread);
    process_ = pi.hProcess;
    childIn_ = inWrite;
    childOut_ = outRead;
    readBuf_.resize(64 * 1024);
    started_ = true;
    return true;
}

bool Worker::writeLine(const std::string& line) {
    DWORD written = 0;
    std::string buf = line;
    if (buf.empty() || buf.back() != '\n') buf.push_back('\n');
    if (!WriteFile(childIn_, buf.data(), (DWORD)buf.size(), &written, nullptr)) return false;
    return written == buf.size();
}

bool Worker::readLine(std::string& out) {
    while (true) {
        auto nl = readAccum_.find('\n');
        if (nl != std::string::npos) {
            out.assign(readAccum_, 0, nl);
            readAccum_.erase(0, nl + 1);
            if (!out.empty() && out.back() == '\r') out.pop_back();
            return true;
        }
        DWORD got = 0;
        BOOL ok = ReadFile(childOut_, readBuf_.data(), (DWORD)readBuf_.size(), &got, nullptr);
        if (!ok || got == 0) return false;
        readAccum_.append(readBuf_.data(), got);
    }
}

nlohmann::json Worker::request(const nlohmann::json& req, ProgressCallback onProgress) {
    std::lock_guard<std::mutex> lock(mu_);
    if (!ensureStarted()) {
        throw std::runtime_error("failed to start worker process");
    }

    std::string payload = req.dump();
    if (!writeLine(payload)) {
        throw std::runtime_error("failed to write to worker stdin");
    }

    while (true) {
        std::string line;
        if (!readLine(line)) {
            throw std::runtime_error("worker closed stdout");
        }
        nlohmann::json resp;
        try {
            resp = nlohmann::json::parse(line);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("decode worker reply: ") + e.what()
                                     + " (raw: " + line + ")");
        }
        auto t = resp.value("type", "");
        if (t == "progress") {
            if (onProgress) onProgress(resp);
            continue;
        }
        return resp;
    }
}

void Worker::cancel() {
    // Hard-stop: kill the worker process. The in-flight request() will
    // throw on EOF; the next request() will lazily relaunch the worker.
    if (!started_) return;
    if (process_ != INVALID_HANDLE_VALUE) {
        TerminateProcess(process_, 1);
        WaitForSingleObject(process_, 1000);
    }
    if (childIn_ != INVALID_HANDLE_VALUE) {
        CloseHandle(childIn_);
        childIn_ = INVALID_HANDLE_VALUE;
    }
    if (childOut_ != INVALID_HANDLE_VALUE) {
        CloseHandle(childOut_);
        childOut_ = INVALID_HANDLE_VALUE;
    }
    if (process_ != INVALID_HANDLE_VALUE) {
        CloseHandle(process_);
        process_ = INVALID_HANDLE_VALUE;
    }
    started_ = false;
    readAccum_.clear();
}

void Worker::shutdown() {
    std::lock_guard<std::mutex> lock(mu_);
    if (!started_) return;

    const char quit[] = "{\"cmd\":\"quit\"}\n";
    DWORD w = 0;
    WriteFile(childIn_, quit, (DWORD)sizeof(quit) - 1, &w, nullptr);

    CloseHandle(childIn_);
    childIn_ = INVALID_HANDLE_VALUE;

    WaitForSingleObject(process_, 3000);
    CloseHandle(process_);
    CloseHandle(childOut_);
    process_ = INVALID_HANDLE_VALUE;
    childOut_ = INVALID_HANDLE_VALUE;
    started_ = false;
}

Worker::~Worker() { shutdown(); }

#else
#error "Worker is currently Windows-only"
#endif
