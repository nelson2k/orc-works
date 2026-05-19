#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include <nlohmann/json.hpp>

class Worker {
public:
    using ProgressCallback = std::function<void(const nlohmann::json&)>;

    enum class Mode { Local, Remote };

    Worker() = default;
    ~Worker();

    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    nlohmann::json request(const nlohmann::json& req, ProgressCallback onProgress = nullptr);
    void shutdown();
    void cancel();

    // Swap the backend. If the worker is already started, it is shut down so
    // the next request() lazily relaunches against the new transport.
    void setMode(Mode m);
    Mode mode() const { return mode_; }

private:
    bool ensureStarted();
    bool writeLine(const std::string& line);
    bool readLine(std::string& out);
    std::wstring buildCommandLine() const;

    std::mutex mu_;
    bool started_ = false;
    Mode mode_ = Mode::Local;

#ifdef _WIN32
    HANDLE childIn_ = INVALID_HANDLE_VALUE;
    HANDLE childOut_ = INVALID_HANDLE_VALUE;
    HANDLE process_ = INVALID_HANDLE_VALUE;
    std::vector<char> readBuf_;
    std::string readAccum_;
#endif
};
