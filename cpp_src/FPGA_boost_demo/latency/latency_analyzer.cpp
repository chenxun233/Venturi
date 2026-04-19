#include "latency_analyzer.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <stdexcept>

namespace {

constexpr int kStageLabelWidth = 40;

using SampleReader = int64_t (*)(const LatencyLogRecord&);

int64_t readFrameStartToDmaEmit(const LatencyLogRecord& record) {
    return static_cast<int64_t>(record.frame_start_to_dma_emit_ns);
}

int64_t readBatchDuration(const LatencyLogRecord& record) {
    return record.batch_duration_ns;
}

int64_t readBatchEndToStrategyStart(const LatencyLogRecord& record) {
    return record.batch_end_to_strategy_start_ns;
}

int64_t readStrategyStartToTxExecutionAccepted(const LatencyLogRecord& record) {
    return record.strategy_start_to_tx_execution_accepted_ns;
}

int64_t readTxExecutionAcceptedToTxEnqueue(const LatencyLogRecord& record) {
    return record.tx_execution_accepted_to_tx_enqueue_ns;
}

int64_t readTxEnqueueToTxSendEnter(const LatencyLogRecord& record) {
    return record.tx_enqueue_to_tx_send_enter_ns;
}

int64_t readTxSendEnterToTxSendSyscallEnter(const LatencyLogRecord& record) {
    return record.tx_send_enter_to_tx_send_syscall_enter_ns;
}

int64_t readTxSendSyscallEnterToTxSend(const LatencyLogRecord& record) {
    return record.tx_send_syscall_enter_to_tx_send_ns;
}

struct SummaryField {
    const char* label;
    SampleReader reader;
};

constexpr std::array<SummaryField, 8> kSummaryFields = {{
    {"frame_start_to_dma_emit_ns", &readFrameStartToDmaEmit},
    {"batch_duration_ns", &readBatchDuration},
    {"batch_end_to_strategy_start_ns", &readBatchEndToStrategyStart},
    {"strategy_start_to_tx_execution_accepted_ns",
     &readStrategyStartToTxExecutionAccepted},
    {"tx_execution_accepted_to_tx_enqueue_ns", &readTxExecutionAcceptedToTxEnqueue},
    {"tx_enqueue_to_tx_send_enter_ns", &readTxEnqueueToTxSendEnter},
    {"tx_send_enter_to_tx_send_syscall_enter_ns", &readTxSendEnterToTxSendSyscallEnter},
    {"tx_send_syscall_enter_to_tx_send_ns", &readTxSendSyscallEnterToTxSend},
}};

} // namespace

LatencyAnalyzer::LatencyAnalyzer(uint16_t queue_num)
    : m_queue_num(queue_num),
      m_completed_records(queue_num) 
{
    if (queue_num == 0) {
        throw std::invalid_argument("LatencyAnalyzer queue_num must be non-zero");
        for (uint16_t que_idx = 0; que_idx < m_queue_num; ++que_idx) {
            m_completed_records[que_idx].reserve(40000);
        }
    }
}

void LatencyAnalyzer::pushCompletedRecord(const LatencyLogRecord& record) {
    if (record.que_idx >= m_completed_records.size()) {
        return;
    }
    ++record_count;
    m_completed_records[record.que_idx].push_back(record);
}

void LatencyAnalyzer::setWarmupRecords(uint64_t warmup_records) noexcept {
    m_warmup_records = warmup_records;
}

void LatencyAnalyzer::printSummary() const {
    for (uint16_t que_idx = 0; que_idx < m_queue_num; ++que_idx) {
        _printQueueSummary(que_idx);
    }
    std::fflush(stdout);
}

void LatencyAnalyzer::_printQueueSummary(uint16_t que_idx) const {
    const std::vector<LatencyLogRecord>& records = m_completed_records[que_idx];
    const std::size_t completed_count = records.size();
    const std::size_t warmup_count =
        std::min<std::size_t>(completed_count, static_cast<std::size_t>(m_warmup_records));
    const std::size_t analyzed_count = completed_count - warmup_count;

    std::printf("Latency Summary queue=%u\n", static_cast<unsigned int>(que_idx));
    std::printf("record_count=%zu completed_records=%zu warmup=%zu analyzed=%zu\n\n",
                record_count,
                completed_count,
                warmup_count,
                analyzed_count);

    if (analyzed_count == 0U) {
        std::printf("no analyzed records\n\n");
        return;
    }

    std::printf("%-*s %12s %12s %12s %12s %12s\n",
                kStageLabelWidth,
                "stage",
                "min_ns",
                "p50_ns",
                "p90_ns",
                "p99_ns",
                "max_ns");

    for (const SummaryField& field : kSummaryFields) {
        std::vector<int64_t> samples;
        samples.reserve(analyzed_count);
        for (std::size_t idx = warmup_count; idx < completed_count; ++idx) {
            samples.push_back(field.reader(records[idx]));
        }

        std::vector<int64_t> sorted_samples = samples;
        std::sort(sorted_samples.begin(), sorted_samples.end());
        std::printf("%-*s %12lld %12lld %12lld %12lld %12lld\n",
                    kStageLabelWidth,
                    field.label,
                    static_cast<long long>(sorted_samples.front()),
                    static_cast<long long>(_readPercentile(samples, 50U, 100U)),
                    static_cast<long long>(_readPercentile(samples, 90U, 100U)),
                    static_cast<long long>(_readPercentile(samples, 99U, 100U)),
                    static_cast<long long>(sorted_samples.back()));
    }

    std::printf("\n");
}

int64_t LatencyAnalyzer::_readPercentile(std::vector<int64_t> samples,
                                         std::size_t numerator,
                                         std::size_t denominator) {
    if (samples.empty()) {
        return 0;
    }

    std::sort(samples.begin(), samples.end());
    const std::size_t last_idx = samples.size() - 1U;
    const std::size_t idx = (last_idx * numerator) / denominator;
    return samples[idx];
}
