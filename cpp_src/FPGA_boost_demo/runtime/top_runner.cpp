#include "top_runner.h"

#include "../engine/fpga_rx_engine.h"
#include "../latency/latency_tracker.h"
#include "../sync/regression.h"
#include "../sync/sync_handler.h"
#include "../../common/log.h"

#include <chrono>
#include <cstdio>
#include <thread>

namespace {
constexpr unsigned long long kQ32Denominator = 1ULL << 32;

void printRegressionStatus(const RegressionPara& para,
                           const FpgaSyncSnapshot& snapshot,
                           bool regression_frozen) {
    if (para.has_para) {
        std::printf("RegressionPara a_q32=%llu a_exact=%llu/%llu b_ns=%lld frozen=%u snapshot_fpga_tick=%llu snapshot_host_time_ns=%llu snapshot_interval_ns=%llu\n",
                    static_cast<unsigned long long>(para.a_q32),
                    static_cast<unsigned long long>(para.a_q32),
                    kQ32Denominator,
                    static_cast<long long>(para.b_ns),
                    regression_frozen ? 1U : 0U,
                    static_cast<unsigned long long>(snapshot.fpga_tick),
                    static_cast<unsigned long long>(snapshot.host_time_ns),
                    static_cast<unsigned long long>(snapshot.interval_ns));
    } else {
        std::printf("RegressionPara not ready yet snapshot_fpga_tick=%llu snapshot_host_time_ns=%llu snapshot_interval_ns=%llu\n",
                    static_cast<unsigned long long>(snapshot.fpga_tick),
                    static_cast<unsigned long long>(snapshot.host_time_ns),
                    static_cast<unsigned long long>(snapshot.interval_ns));
    }
    std::fflush(stdout);
}

} // namespace

TopRunner::TopRunner(FPGADev& device)
    : m_device(device) {
}

void TopRunner::addRxEngine(FPGARxEngine& engine, bool use_sync_path) {
    m_rx_engines.push_back(RxRuntimeConfig {&engine, use_sync_path});
}

void TopRunner::addSyncController(SyncHandler& sync_handler) {
    m_sync_controllers.push_back(&sync_handler);
}

void TopRunner::addTxEngine(TxEngine& engine) {
    m_tx_engines.push_back(&engine);
}

void TopRunner::attachLatencyTracker(LatencyTracker* tracker) {
    m_latency_tracker = tracker;
}

void TopRunner::attachRegression(Regression* regression) {
    m_regression = regression;
}

void TopRunner::attachTuner(Tuner* tuner) {
    m_tuner = tuner;
}

void TopRunner::setControlLoopSleep(std::chrono::microseconds interval) {
    m_control_loop_sleep = interval;
}

void TopRunner::run() {
    (void)m_device;
    (void)m_tuner;
    if (m_rx_engines.size() != 2) {
        error("TopRunner expects exactly 2 RX engines, got %zu", m_rx_engines.size());
        return;
    }

    if (m_sync_controllers.size() != 1) {
        error("TopRunner expects exactly 1 SyncHandler, got %zu", m_sync_controllers.size());
        return;
    }

    if (m_latency_tracker == nullptr || m_regression == nullptr) {
        error("TopRunner is missing required modules: tracker=%p regression=%p",
              static_cast<void*>(m_latency_tracker),
              static_cast<void*>(m_regression));
        return;
    }

    std::size_t sync_path_count = 0;
    for (std::size_t engine_idx = 0; engine_idx < m_rx_engines.size(); ++engine_idx) {
        if (m_rx_engines[engine_idx].engine == nullptr) {
            error("RX engine slot %zu is null", engine_idx);
            return;
        }

        m_rx_engines[engine_idx].engine->attachTraceBuffer(m_latency_tracker->readBuffer(engine_idx));
        if (m_rx_engines[engine_idx].use_sync_path) {
            ++sync_path_count;
        }
    }

    if (sync_path_count != 1) {
        error("TopRunner expects exactly one sync RX path, got %zu", sync_path_count);
        return;
    }

    m_latency_tracker->attachRegression(m_regression);
    m_latency_tracker->setMeasurementEnabled(true);
    m_capture_signal.generation.store(0, std::memory_order_release);
    m_capture_signal.ready.store(false, std::memory_order_release);

    m_running.store(true, std::memory_order_release);

    std::thread control_thread([this]() {
        bool freeze_logged = false;
        auto last_regression_print = std::chrono::steady_clock::now();
        while (m_running.load(std::memory_order_acquire)) {
            m_sync_controllers[0]->run(m_capture_signal);
            m_regression->run(m_capture_signal.ready, m_sync_snapshot);
            const bool regression_frozen = m_regression->isFrozen();

            const auto now = std::chrono::steady_clock::now();
            if (now - last_regression_print >= std::chrono::seconds(1)) {
                const RegressionPara para = m_regression->readParaSnapshot();
                const FpgaSyncSnapshot snapshot = m_sync_snapshot;
                printRegressionStatus(para, snapshot, regression_frozen);
                last_regression_print = now;
            }

            if (regression_frozen && !freeze_logged) {
                std::printf("Regression converged; parameters frozen\n");
                std::fflush(stdout);
                freeze_logged = true;
            }

            m_latency_tracker->run();
            std::this_thread::sleep_for(m_control_loop_sleep);
        }
    });

    std::vector<std::thread> rx_threads;
    rx_threads.reserve(m_rx_engines.size());
    for (std::size_t engine_idx = 0; engine_idx < m_rx_engines.size(); ++engine_idx) {
        rx_threads.emplace_back([this, engine_idx]() {
            const bool use_sync_path = m_rx_engines[engine_idx].use_sync_path;
            FPGARxEngine& engine = *m_rx_engines[engine_idx].engine;
            uint64_t last_generation = 0;

            while (m_running.load(std::memory_order_acquire)) {
                const uint64_t capture_generation =
                    m_capture_signal.generation.load(std::memory_order_acquire);
                const bool get_time = capture_generation != last_generation;
                if (get_time) {
                    last_generation = capture_generation;
                }

                const std::size_t count = use_sync_path
                    ? engine.pollBatchSync(MAX_POLL_RECORDS,
                                           get_time,
                                           m_capture_signal.ready,
                                           m_sync_snapshot)
                    : engine.pollBatch(MAX_POLL_RECORDS, get_time);

                if (count == 0) {
                    std::this_thread::yield();
                    continue;
                }
            }
        });
    }

    for (std::thread& thread : rx_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    m_running.store(false, std::memory_order_release);
    if (control_thread.joinable()) {
        control_thread.join();
    }
}

void TopRunner::stop() {
    m_running.store(false, std::memory_order_release);
}

std::size_t TopRunner::readRxEngineCount() const {
    return m_rx_engines.size();
}

std::size_t TopRunner::readTxEngineCount() const {
    return m_tx_engines.size();
}
