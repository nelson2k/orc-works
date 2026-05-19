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

    Worker() = default;
    ~Worker();

    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    nlohmann::json request(const nlohmann::json& req, ProgressCallback onProgress = nullptr);
    void shutdown();
    void cancel();

private:
    bool ensureStarted();
    bool writeLine(const std::string& line);
    bool readLine(std::string& out);

    std::mutex mu_;
    bool started_ = false;

#ifdef _WIN32
    HANDLE childIn_ = INVALID_HANDLE_VALUE;
    HANDLE childOut_ = INVALID_HANDLE_VALUE;
    HANDLE process_ = INVALID_HANDLE_VALUE;
    std::vector<char> readBuf_;
    std::string readAccum_;
#endif
};
