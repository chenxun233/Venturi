#pragma once

#include "../common/spsc_ring_queue.h"
#include "../common/shared_types.h"

#include <atomic>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class LogPrinter {
public:
    explicit LogPrinter(uint16_t queue_num, std::size_t capacity = 1024);
    ~LogPrinter();

    bool pushLatencyLog(const LatencyLogRecord& record);
    bool pushExecutionLog(const ExecutionLogRecord& execution);
    bool pushTxLog(const TxLogRecord& record);
    void setWorkerCpu(int cpu_id);
    void setLatencyWarmupRecords(uint64_t record_count);
    void start();
    void stop();
    uint64_t readDropCount() const;

private:
    enum class LatencyField : uint8_t {
        FrameStartToDmaEmit = 0,
        BatchDuration,
        BatchEndToStrategyStart,
        StrategyStartToTxExecutionAccepted,
        TxExecutionAcceptedToTxEnqueue,
        TxEnqueueToTxSendEnter,
        TxSendEnterToTxSendSyscallEnter,
        TxSendSyscallEnterToTxSend,
        Count
    };

    struct LatencySummaryStats {
        std::vector<int64_t> samples {};
    };

    bool _pushLatencyLogRecord(const LatencyLogRecord& record);
    bool _pushExecutionLogRecord(const ExecutionLogRecord& execution);
    bool _pushTxLogRecord(const TxLogRecord& record);
    bool _drainLatencyRecord();
    bool _drainExecutionLogRecord();
    bool _drainTxLogRecord();
    void _drainRemaining();
    void _recordLatencySamples(const LatencyLogRecord& record);
    void _run();
    void _printLatencySummary();
    void _printLatencyQueueSummary(uint16_t queue_idx);
    void _printLatencyRecord(const LatencyLogRecord& record);
    void _printExecutionLogRecord(const ExecutionLogRecord& execution);
    void _printTxLogRecord(const TxLogRecord& record);

    uint16_t m_queue_num {0};
    std::vector<std::unique_ptr<SpscRingQueue<LatencyLogRecord>>> m_latency_log_queues;
    std::vector<std::unique_ptr<SpscRingQueue<ExecutionLogRecord>>> m_execution_log_queues;
    std::vector<std::unique_ptr<SpscRingQueue<TxLogRecord>>> m_tx_log_queues;
    uint16_t m_next_latency_queue_idx {0};
    uint16_t m_next_execution_log_queue_idx {0};
    uint16_t m_next_tx_log_queue_idx {0};
    std::atomic<uint64_t> m_drop_count {0};
    std::atomic<bool> m_running {false};
    uint64_t m_latency_warmup_records {0};
    std::vector<uint64_t> m_latency_seen_counts {};
    std::vector<std::array<LatencySummaryStats, static_cast<std::size_t>(LatencyField::Count)>>
        m_latency_summary_stats;
    bool m_latency_summary_printed {false};
    int m_worker_cpu {-1};
    std::thread m_thread;
};
