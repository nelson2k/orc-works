#include "Worker.h"

#include <cstring>
#include <memory>
#include <stdexcept>

#ifdef _WIN32

#include <winhttp.h>

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

std::wstring envOr(const wchar_t* name, const wchar_t* fallback) {
    wchar_t buf[1024];
    DWORD n = GetEnvironmentVariableW(name, buf, 1024);
    if (n == 0 || n >= 1024) return fallback;
    return std::wstring(buf, n);
}

// --- WinHTTP helpers ---------------------------------------------------------

struct ParsedUrl {
    std::wstring host;
    int port = 80;
    std::wstring path;
    bool https = false;
};

bool parseUrl(const std::wstring& url, ParsedUrl& out) {
    URL_COMPONENTSW uc{};
    uc.dwStructSize = sizeof(uc);
    wchar_t host[256] = {0};
    wchar_t path[1024] = {0};
    uc.lpszHostName = host; uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path; uc.dwUrlPathLength = 1024;
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &uc)) return false;
    out.host = host;
    out.port = uc.nPort;
    out.path = path;
    out.https = (uc.nScheme == INTERNET_SCHEME_HTTPS);
    return true;
}

struct HInternetCloser {
    void operator()(void* h) const { if (h) WinHttpCloseHandle(h); }
};
using HInternet = std::unique_ptr<void, HInternetCloser>;

// One blocking POST. Reads the whole body and returns it as a string.
// Throws on transport failure.
std::string httpPost(const std::wstring& baseUrl,
                     const std::wstring& path,
                     const std::string& body,
                     const wchar_t* contentType = L"application/json",
                     int timeoutMs = 30000) {
    ParsedUrl u;
    if (!parseUrl(baseUrl, u)) {
        throw std::runtime_error("invalid OCR_REMOTE_HTTP_URL");
    }

    HInternet session(WinHttpOpen(L"orcgui/1.0",
                                  WINHTTP_ACCESS_TYPE_NO_PROXY,
                                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session) throw std::runtime_error("WinHttpOpen failed");
    WinHttpSetTimeouts(session.get(), timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    HInternet conn(WinHttpConnect(session.get(), u.host.c_str(), (INTERNET_PORT)u.port, 0));
    if (!conn) throw std::runtime_error("WinHttpConnect failed");

    HInternet req(WinHttpOpenRequest(conn.get(), L"POST", path.c_str(),
                                     nullptr, WINHTTP_NO_REFERER,
                                     WINHTTP_DEFAULT_ACCEPT_TYPES,
                                     u.https ? WINHTTP_FLAG_SECURE : 0));
    if (!req) throw std::runtime_error("WinHttpOpenRequest failed");

    std::wstring headers = L"Content-Type: ";
    headers += contentType;

    if (!WinHttpSendRequest(req.get(),
                            headers.c_str(), (DWORD)headers.size(),
                            (LPVOID)body.data(), (DWORD)body.size(),
                            (DWORD)body.size(), 0)) {
        throw std::runtime_error("WinHttpSendRequest failed");
    }
    if (!WinHttpReceiveResponse(req.get(), nullptr)) {
        throw std::runtime_error("WinHttpReceiveResponse failed");
    }

    std::string out;
    char buf[16 * 1024];
    DWORD avail = 0;
    while (WinHttpQueryDataAvailable(req.get(), &avail) && avail > 0) {
        DWORD got = 0;
        DWORD toRead = avail > sizeof(buf) ? (DWORD)sizeof(buf) : avail;
        if (!WinHttpReadData(req.get(), buf, toRead, &got) || got == 0) break;
        out.append(buf, got);
    }
    return out;
}

// Same idea but a blocking GET.
std::string httpGet(const std::wstring& baseUrl,
                    const std::wstring& path,
                    int timeoutMs = 5000) {
    ParsedUrl u;
    if (!parseUrl(baseUrl, u)) {
        throw std::runtime_error("invalid OCR_REMOTE_HTTP_URL");
    }
    HInternet session(WinHttpOpen(L"orcgui/1.0",
                                  WINHTTP_ACCESS_TYPE_NO_PROXY,
                                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session) throw std::runtime_error("WinHttpOpen failed");
    WinHttpSetTimeouts(session.get(), timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    HInternet conn(WinHttpConnect(session.get(), u.host.c_str(), (INTERNET_PORT)u.port, 0));
    if (!conn) throw std::runtime_error("WinHttpConnect failed");

    HInternet req(WinHttpOpenRequest(conn.get(), L"GET", path.c_str(),
                                     nullptr, WINHTTP_NO_REFERER,
                                     WINHTTP_DEFAULT_ACCEPT_TYPES,
                                     u.https ? WINHTTP_FLAG_SECURE : 0));
    if (!req) throw std::runtime_error("WinHttpOpenRequest failed");

    if (!WinHttpSendRequest(req.get(),
                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        throw std::runtime_error("WinHttpSendRequest failed");
    }
    if (!WinHttpReceiveResponse(req.get(), nullptr)) {
        throw std::runtime_error("WinHttpReceiveResponse failed");
    }

    std::string out;
    char buf[16 * 1024];
    DWORD avail = 0;
    while (WinHttpQueryDataAvailable(req.get(), &avail) && avail > 0) {
        DWORD got = 0;
        DWORD toRead = avail > sizeof(buf) ? (DWORD)sizeof(buf) : avail;
        if (!WinHttpReadData(req.get(), buf, toRead, &got) || got == 0) break;
        out.append(buf, got);
    }
    return out;
}

std::wstring remoteBaseUrl() {
    return envOr(L"OCR_REMOTE_HTTP_URL", L"http://192.168.10.200:9000");
}

}  // anonymous namespace

// =============================================================================
// Public API — dispatches on mode
// =============================================================================

nlohmann::json Worker::request(const nlohmann::json& req, ProgressCallback onProgress) {
    if (mode_ == Mode::Remote) return requestRemote(req, onProgress);
    return requestLocal(req, onProgress);
}

void Worker::cancel() {
    if (mode_ == Mode::Remote) { cancelRemote(); return; }
    cancelLocal();
}

void Worker::shutdown() {
    // The remote FastAPI worker has its own lifecycle (systemd / setsid).
    // Only the local subprocess is ours to tear down.
    shutdownLocal();
}

void Worker::setMode(Mode m) {
    if (mode_ == m) return;
    // Tear down whatever we currently own so the next request goes through
    // the new transport.
    shutdown();
    if (mode_ == Mode::Remote) cancelRemote();
    {
        std::lock_guard<std::mutex> lk(metricsMu_);
        haveRemoteMetrics_ = false;
    }
    mode_ = m;
}

void Worker::storeRemoteMetrics(const nlohmann::json& m) {
    MetricsSample s;
    s.cpuPct    = m.value("cpu_pct", 0.0);
    s.ramPct    = m.value("ram_pct", 0.0);
    s.ramUsedGB = m.value("ram_used_gb", 0.0);
    s.hasGPU    = m.value("has_gpu", false);
    if (s.hasGPU) {
        s.gpuPct     = m.value("gpu_pct", 0.0);
        s.vramPct    = m.value("vram_pct", 0.0);
        s.vramUsedMB = m.value("vram_used_mb", 0.0);
        s.tempC      = m.value("temp_c", 0.0);
    }
    std::lock_guard<std::mutex> lk(metricsMu_);
    lastRemoteMetrics_ = s;
    haveRemoteMetrics_ = true;
}

bool Worker::getRemoteMetrics(MetricsSample& out) {
    std::lock_guard<std::mutex> lk(metricsMu_);
    if (!haveRemoteMetrics_) return false;
    out = lastRemoteMetrics_;
    return true;
}

// =============================================================================
// Local mode — subprocess + stdin/stdout JSON
// =============================================================================

bool Worker::ensureStartedLocal() {
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

nlohmann::json Worker::requestLocal(const nlohmann::json& req, ProgressCallback onProgress) {
    std::lock_guard<std::mutex> lock(mu_);
    if (!ensureStartedLocal()) {
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
        if (line.empty()) continue;
        if (line[0] != '{' && line[0] != '[') continue;
        nlohmann::json resp;
        try {
            resp = nlohmann::json::parse(line);
        } catch (const std::exception&) {
            continue;
        }
        auto t = resp.value("type", "");
        if (t == "progress") {
            if (onProgress) onProgress(resp);
            continue;
        }
        if (t == "metrics") {
            storeRemoteMetrics(resp);
            continue;
        }
        return resp;
    }
}

void Worker::shutdownLocal() {
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

void Worker::cancelLocal() {
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

// =============================================================================
// Remote mode — HTTP + SSE against FastAPI worker
// =============================================================================

nlohmann::json Worker::requestRemote(const nlohmann::json& req, ProgressCallback onProgress) {
    auto cmd = req.value("cmd", "");
    if (cmd == "quit") {
        // We never own the remote lifecycle.
        return nlohmann::json{{"type", "ok"}};
    }

    std::wstring base = remoteBaseUrl();

    // ----- render: simple JSON POST, no streaming -----------------------------
    if (cmd == "render") {
        nlohmann::json body = req;
        body.erase("cmd");
        std::string respText = httpPost(base, L"/v1/render", body.dump(),
                                        L"application/json", 60000);
        try {
            return nlohmann::json::parse(respText);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("bad /v1/render response: ") + e.what());
        }
    }

    // ----- ocr: SSE stream, progress + metrics + result ----------------------
    if (cmd != "ocr") {
        throw std::runtime_error("unknown command for remote mode: " + cmd);
    }

    ParsedUrl u;
    if (!parseUrl(base, u)) throw std::runtime_error("invalid OCR_REMOTE_HTTP_URL");

    HInternet session(WinHttpOpen(L"orcgui/1.0",
                                  WINHTTP_ACCESS_TYPE_NO_PROXY,
                                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session) throw std::runtime_error("WinHttpOpen failed");
    // OCR can take many minutes; don't time out the read side aggressively.
    WinHttpSetTimeouts(session.get(), 10000, 10000, 10000, /*recv*/ 10 * 60 * 1000);

    HInternet conn(WinHttpConnect(session.get(), u.host.c_str(), (INTERNET_PORT)u.port, 0));
    if (!conn) throw std::runtime_error("WinHttpConnect failed");

    HInternet hreq(WinHttpOpenRequest(conn.get(), L"POST", L"/v1/ocr",
                                      nullptr, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES,
                                      u.https ? WINHTTP_FLAG_SECURE : 0));
    if (!hreq) throw std::runtime_error("WinHttpOpenRequest failed");

    nlohmann::json body = req;
    body.erase("cmd");
    std::string payload = body.dump();
    const wchar_t* headers = L"Content-Type: application/json\r\nAccept: text/event-stream";

    if (!WinHttpSendRequest(hreq.get(), headers, (DWORD)-1L,
                            (LPVOID)payload.data(), (DWORD)payload.size(),
                            (DWORD)payload.size(), 0)) {
        throw std::runtime_error("WinHttpSendRequest failed");
    }
    if (!WinHttpReceiveResponse(hreq.get(), nullptr)) {
        throw std::runtime_error("WinHttpReceiveResponse failed");
    }

    // Publish the request handle so cancel() can abort us from another thread.
    {
        std::lock_guard<std::mutex> lk(httpActiveReqMu_);
        httpActiveReq_ = hreq.get();
    }
    struct ClearActive {
        Worker* w;
        ~ClearActive() {
            std::lock_guard<std::mutex> lk(w->httpActiveReqMu_);
            w->httpActiveReq_ = nullptr;
        }
    } _clearOnExit{this};

    // SSE parse loop. Each event is delimited by a blank line ("\n\n").
    // Within an event we only care about lines that start with "data: ".
    std::string accum;
    nlohmann::json finalResult;
    bool sawResult = false;

    char buf[16 * 1024];
    while (!sawResult) {
        DWORD avail = 0;
        if (!WinHttpQueryDataAvailable(hreq.get(), &avail)) {
            throw std::runtime_error("remote stream ended (queryAvail failed)");
        }
        if (avail == 0) break;
        DWORD got = 0;
        DWORD toRead = avail > sizeof(buf) ? (DWORD)sizeof(buf) : avail;
        if (!WinHttpReadData(hreq.get(), buf, toRead, &got) || got == 0) break;
        accum.append(buf, got);

        // Drain complete events from accum.
        while (true) {
            auto sep = accum.find("\n\n");
            if (sep == std::string::npos) break;
            std::string evt = accum.substr(0, sep);
            accum.erase(0, sep + 2);

            // Pick out `data: <json>` lines. SSE allows multi-line data; we
            // join them with '\n'. In practice our worker emits one per event.
            std::string data;
            size_t p = 0;
            while (p < evt.size()) {
                auto eol = evt.find('\n', p);
                std::string line = evt.substr(p, eol == std::string::npos ? std::string::npos : eol - p);
                p = (eol == std::string::npos) ? evt.size() : eol + 1;
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (line.rfind("data:", 0) != 0) continue;
                std::string payloadPart = line.substr(5);
                if (!payloadPart.empty() && payloadPart[0] == ' ') payloadPart.erase(0, 1);
                if (!data.empty()) data.push_back('\n');
                data += payloadPart;
            }
            if (data.empty()) continue;

            nlohmann::json msg;
            try {
                msg = nlohmann::json::parse(data);
            } catch (const std::exception&) {
                continue;
            }
            auto t = msg.value("type", "");
            if (t == "progress") {
                if (onProgress) onProgress(msg);
            } else if (t == "metrics") {
                storeRemoteMetrics(msg);
            } else {
                finalResult = std::move(msg);
                sawResult = true;
                break;
            }
        }
    }

    if (!sawResult) {
        throw std::runtime_error("remote stream ended without final result");
    }
    return finalResult;
}

void Worker::cancelRemote() {
    std::lock_guard<std::mutex> lk(httpActiveReqMu_);
    if (httpActiveReq_) {
        // Closing the request handle causes the read loop to fail with a
        // sensible error. The remote worker will continue processing the
        // request to completion (no cooperative-cancel protocol yet); we
        // just stop listening.
        WinHttpCloseHandle((HINTERNET)httpActiveReq_);
        httpActiveReq_ = nullptr;
    }
}

void Worker::pollRemoteMetricsHttp() {
    if (mode_ != Mode::Remote) return;
    try {
        std::string body = httpGet(remoteBaseUrl(), L"/v1/metrics", 3000);
        auto m = nlohmann::json::parse(body);
        storeRemoteMetrics(m);
    } catch (const std::exception&) {
        // Best-effort; idle bars just stay frozen if the server is briefly down.
    }
}

Worker::~Worker() { shutdown(); }

#else
#error "Worker is currently Windows-only"
#endif
