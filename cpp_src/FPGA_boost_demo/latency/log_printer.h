#pragma once

#include "../common/spsc_ring_queue.h"
#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class LogPrinter {
public:
    explicit LogPrinter(uint16_t queue_num, std::size_t capacity = 1024);
    ~LogPrinter();

    bool pushTxLog(const TxLogRecord& record);
    void setWorkerCpu(int cpu_id);
    void start();
    void stop();

private:
    bool _drainTxLogRecord();
    void _drainRemaining();
    void _run();
    void _printTxLogRecord(const TxLogRecord& record);

    uint16_t m_queue_num {0};
    std::vector<std::unique_ptr<SpscRingQueue<TxLogRecord>>> m_tx_log_queues;
    uint16_t m_next_tx_log_queue_idx {0};
    std::atomic<uint64_t> m_drop_count {0};
    std::atomic<bool> m_running {false};
    int m_worker_cpu {-1};
    std::thread m_thread;
};
