#pragma once

#include "../common/shared_types.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class LatencyAnalyzer {
public:
    explicit LatencyAnalyzer(uint16_t queue_num);

    void pushCompletedRecord(const LatencyLogRecord& record);
    void setWarmupRecords(uint64_t warmup_records) noexcept;
    void printSummary() const;

private:
    struct SummaryRow {
        const char* label {nullptr};
        std::vector<int64_t> samples {};
    };

    void _printQueueSummary(uint16_t que_idx) const;
    static int64_t _readPercentile(std::vector<int64_t> samples,
                                   std::size_t numerator,
                                   std::size_t denominator);

    uint16_t m_queue_num {0};
    uint64_t m_warmup_records {0};
    std::vector<std::vector<LatencyLogRecord>> m_completed_records;
    std::size_t record_count {0};
};
