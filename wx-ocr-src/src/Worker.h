#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include <nlohmann/json.hpp>

#include "Metrics.h"

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

    // The remote worker streams type:"metrics" events on the same JSON pipe
    // during requests; the parent stashes the last one here for the metrics
    // timer to read in Remote mode. Returns false when no sample has arrived
    // since the last shutdown / mode change.
    bool getRemoteMetrics(MetricsSample& out);

    // --- Server-side Extract PDF batch (Remote mode) ---
    //
    // The remote runs the page loop so the work survives the GUI closing.
    // Extract PDF posts /v1/extract, then streams /v1/jobs/{id}/stream to
    // see what's happening. On relaunch, currentRemoteJob() reveals any
    // job-in-flight so the GUI can re-attach via streamRemoteJob().
    std::string startRemoteBatch(const std::string& pdfPath,
                                 const std::string& engine,
                                 int startPage, int endPage);
    // Blocks until the job emits {"type":"done"} or the connection drops.
    // metrics events are absorbed into the local cache; everything else
    // (page_image, page_complete, progress, error, done) is forwarded.
    void streamRemoteJob(const std::string& jobId,
                         std::function<void(const nlohmann::json&)> eventCb);
    void cancelRemoteJob(const std::string& jobId);
    // Returns the parsed JSON for /v1/jobs/current — `null` if no job.
    nlohmann::json currentRemoteJob();
    // Aborts an in-flight SSE stream (closes the underlying handle). Does
    // NOT cancel the job on the server; pair with cancelRemoteJob() for that.
    void abortRemoteStream();

private:
    // One blocking GET /v1/metrics → cache. Called from the polling thread,
    // never from the UI thread.
    void pollRemoteMetricsHttp();
    void startRemoteMetricsPolling();
    void stopRemoteMetricsPolling();

    // --- shared ---
    void storeRemoteMetrics(const nlohmann::json& m);

    // --- Local (subprocess + stdin/stdout JSON) ---
    bool ensureStartedLocal();
    bool writeLine(const std::string& line);
    bool readLine(std::string& out);
    nlohmann::json requestLocal(const nlohmann::json& req, ProgressCallback onProgress);
    void shutdownLocal();
    void cancelLocal();

    // --- Remote (HTTP + SSE) ---
    nlohmann::json requestRemote(const nlohmann::json& req, ProgressCallback onProgress);
    void cancelRemote();

    std::mutex mu_;
    bool started_ = false;
    Mode mode_ = Mode::Local;

    std::mutex metricsMu_;
    MetricsSample lastRemoteMetrics_;
    bool haveRemoteMetrics_ = false;

    // Background thread that polls /v1/metrics every couple of seconds while
    // mode_ == Remote, so the bars stay live between requests without ever
    // blocking the UI thread.
    std::thread metricsPollThread_;
    std::atomic<bool> metricsPollStop_{false};
    std::mutex metricsPollWaitMu_;
    std::condition_variable metricsPollWaitCv_;

#ifdef _WIN32
    // Local-mode subprocess
    HANDLE childIn_ = INVALID_HANDLE_VALUE;
    HANDLE childOut_ = INVALID_HANDLE_VALUE;
    HANDLE process_ = INVALID_HANDLE_VALUE;
    std::vector<char> readBuf_;
    std::string readAccum_;

    // Remote-mode WinHTTP. The active request handle is exposed so
    // cancel() can close it from another thread and abort the read.
    void* httpActiveReq_ = nullptr;   // HINTERNET (kept void* to keep
                                      // winhttp.h out of the header)
    std::mutex httpActiveReqMu_;
    // Separate handle for the long-lived /v1/jobs/{id}/stream connection,
    // so cancelling the per-page request doesn't kill the batch stream.
    void* httpStreamReq_ = nullptr;
    std::mutex httpStreamReqMu_;
#endif
};
