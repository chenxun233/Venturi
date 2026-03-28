#pragma once

#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <vector>

// Single-producer/single-consumer ring buffer for hot-path trace points.
template <typename T>
class TraceBuffer {
public:
    explicit    TraceBuffer(std::size_t capacity);
    std::size_t readCapacity() const;
    uint64_t    readDropCount() const;
    bool        push(const T& record);
    bool        pop(T& record);

private:
    std::size_t _slotIndex(std::size_t idx) const;

    std::vector<T>  m_records;
    std::size_t              m_capacity_mask {0};
    alignas(64) std::atomic<std::size_t> m_head {0};
    alignas(64) std::atomic<std::size_t> m_tail {0};
    std::atomic<uint64_t>    m_drop_count {0};
};

template <typename T>
TraceBuffer<T>::TraceBuffer(std::size_t capacity)
    : m_records(capacity),
      m_capacity_mask(capacity - 1) {
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        throw std::invalid_argument("TraceBuffer capacity must be a non-zero power of two");
    }
}

template <typename T>
std::size_t TraceBuffer<T>::readCapacity() const {
    return m_records.size();
}

template <typename T>
uint64_t TraceBuffer<T>::readDropCount() const {
    return m_drop_count.load(std::memory_order_relaxed);
}

template <typename T>
bool TraceBuffer<T>::push(const T& record) {
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

template <typename T>
bool TraceBuffer<T>::pop(T& record) {
    const std::size_t head = m_head.load(std::memory_order_relaxed);
    if (head == m_tail.load(std::memory_order_acquire)) {
        return false;
    }

    record = m_records[_slotIndex(head)];
    m_head.store(head + 1, std::memory_order_release);
    return true;
}

template <typename T>
std::size_t TraceBuffer<T>::_slotIndex(std::size_t idx) const {
    return idx & m_capacity_mask;
}
