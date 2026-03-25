#pragma once

#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <vector>

// Single-producer/single-consumer ring buffer for hot-path trace points.
class TraceBuffer {
public:
    explicit    TraceBuffer(std::size_t capacity);
    std::size_t readCapacity() const;
    uint64_t    readDropCount() const;
    bool        push(const TimeRecord& record);
    bool        pop(TimeRecord& record);

private:
    std::size_t _slotIndex(std::size_t idx) const;

    std::vector<TimeRecord>  m_records;
    std::size_t              m_capacity_mask {0};
    alignas(64) std::atomic<std::size_t> m_head {0};
    alignas(64) std::atomic<std::size_t> m_tail {0};
    std::atomic<uint64_t>    m_drop_count {0};
};
