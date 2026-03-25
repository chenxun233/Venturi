#include "trace_buffer.h"

TraceBuffer::TraceBuffer(std::size_t capacity)
    : m_records(capacity),
      m_capacity_mask(capacity - 1) {
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        throw std::invalid_argument("TraceBuffer capacity must be a non-zero power of two");
    }
}

std::size_t TraceBuffer::readCapacity() const {
    return m_records.size();
}

uint64_t TraceBuffer::readDropCount() const {
    return m_drop_count.load(std::memory_order_relaxed);
}

bool TraceBuffer::push(const TimeRecord& record) {
    const std::size_t tail = m_tail.load(std::memory_order_relaxed);
    const std::size_t head = m_head.load(std::memory_order_acquire);
    if (tail - head == readCapacity()) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    m_records[_slotIndex(tail)] = record;
    m_tail.store(tail + 1, std::memory_order_release);
    return true;
}

bool TraceBuffer::pop(TimeRecord& record) {
    const std::size_t head = m_head.load(std::memory_order_relaxed);
    if (head == m_tail.load(std::memory_order_acquire)) {
        return false;
    }

    record = m_records[_slotIndex(head)];
    m_head.store(head + 1, std::memory_order_release);
    return true;
}

std::size_t TraceBuffer::_slotIndex(std::size_t idx) const {
    return idx & m_capacity_mask;
}
