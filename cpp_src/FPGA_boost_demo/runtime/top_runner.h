#pragma once

#include "../common/shared_types.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <vector>

class FPGADev;
class FPGARxEngine;
class LatencyTracker;
class Regression;
class SyncHandler;
class Tuner;
class TxEngine;

class TopRunner {
public:
    explicit TopRunner(FPGADev& device);

    void addRxEngine(FPGARxEngine& engine, bool use_sync_path);
    void addSyncController(SyncHandler& sync_handler);
    void addTxEngine(TxEngine& engine);
    void attachLatencyTracker(LatencyTracker* tracker);
    void attachRegression(Regression* regression);
    void attachTuner(Tuner* tuner);
    void setControlLoopSleep(std::chrono::microseconds interval);
    void run();
    void stop();
    std::size_t readRxEngineCount() const;
    std::size_t readTxEngineCount() const;

private:
    struct RxRuntimeConfig {
        FPGARxEngine* engine {nullptr};
        bool use_sync_path {false};
    };

    FPGADev& m_device;
    std::vector<RxRuntimeConfig> m_rx_engines;
    std::vector<SyncHandler*> m_sync_controllers;
    std::vector<TxEngine*> m_tx_engines;
    LatencyTracker* m_latency_tracker {nullptr};
    Regression* m_regression {nullptr};
    Tuner* m_tuner {nullptr};
    std::atomic<bool> m_running {false};
    CapSignal m_capture_signal {};
    FpgaSyncSnapshot m_sync_snapshot {};
    std::chrono::microseconds m_control_loop_sleep {std::chrono::microseconds(100)};
};
