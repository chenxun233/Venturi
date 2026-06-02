#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity != 0 && (Capacity & (Capacity - 1)) == 0,
                  "RingBuffer capacity must be a non-zero power of two");

public:
    bool        isEmpty() const;

    std::size_t readSize() const;
    constexpr std::size_t readCapacity() const;
    void        clear();
    bool        pushBack(const T& value);
    bool        pushBack(T&& value);
    bool        write(const T* values, std::size_t count);
    bool        eraseFront();
    bool        eraseFrontN(std::size_t count);
    T&          readFront();
    const T&    readFront() const;
    T&          readAt(std::size_t offset);
    const T&    readAt(std::size_t offset) const;
    bool        copyFrom(std::size_t offset, T* out, std::size_t count) const;

private:
    bool        _isFull() const;

private:
    // wrap and plus one.
    std::size_t _wrapIndexP1(std::size_t idx) const;
    std::array<T, Capacity> m_records {};
    std::size_t             m_head {0};
    std::size_t             m_count {0};
};

template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::isEmpty() const {
    return m_count == 0;
}

template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::_isFull() const {
    return m_count == Capacity;
}

template <typename T, std::size_t Capacity>
std::size_t RingBuffer<T, Capacity>::readSize() const {
    return m_count;
}

template <typename T, std::size_t Capacity>
constexpr std::size_t RingBuffer<T, Capacity>::readCapacity() const {
    return Capacity;
}

template <typename T, std::size_t Capacity>
void RingBuffer<T, Capacity>::clear() {
    m_head = 0;
    m_count = 0;
}

template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::pushBack(const T& value) {
    if (_isFull()) {
        return false;
    }
    m_records[_wrapIndexP1(m_head + m_count)] = value;
    ++m_count;
    return true;
}

template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::pushBack(T&& value) {
    if (_isFull()) {
        return false;
    }

    m_records[_wrapIndexP1(m_head + m_count)] = std::move(value);
    ++m_count;
    return true;
}

template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::write(const T* values, std::size_t count) {
    if (count > Capacity - m_count) {
        return false;
    }
    if (count == 0) {
        return true;
    }

    const std::size_t tail = _wrapIndexP1(m_head + m_count);
    const std::size_t first_chunk = std::min(count, Capacity - tail);
    std::copy_n(values, static_cast<std::ptrdiff_t>(first_chunk), m_records.begin() + static_cast<std::ptrdiff_t>(tail));
    if (count > first_chunk) {
        std::copy_n(values + static_cast<std::ptrdiff_t>(first_chunk),
                    static_cast<std::ptrdiff_t>(count - first_chunk),
                    m_records.begin());
    }
    m_count += count;
    return true;
}

template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::eraseFront() {
    return eraseFrontN(1);
}


template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::eraseFrontN(std::size_t count) {
    if (count > m_count) {
        return false;
    }
    if (count == 0) {
        return true;
    }

    m_head = _wrapIndexP1(m_head + count);
    m_count -= count;
    return true;
}

template <typename T, std::size_t Capacity>
T& RingBuffer<T, Capacity>::readFront() {
    return m_records[m_head];
}

template <typename T, std::size_t Capacity>
const T& RingBuffer<T, Capacity>::readFront() const {
    return m_records[m_head];
}

template <typename T, std::size_t Capacity>
T& RingBuffer<T, Capacity>::readAt(std::size_t offset) {
    return m_records[_wrapIndexP1(m_head + offset)];
}

template <typename T, std::size_t Capacity>
const T& RingBuffer<T, Capacity>::readAt(std::size_t offset) const {
    return m_records[_wrapIndexP1(m_head + offset)];
}


template <typename T, std::size_t Capacity>
bool RingBuffer<T, Capacity>::copyFrom(std::size_t offset, T* out, std::size_t count) const {
    if (offset + count > m_count) {
        return false;
    }
    if (count == 0) {
        return true;
    }

    const std::size_t start = _wrapIndexP1(m_head + offset);
    const std::size_t first_chunk = std::min(count, Capacity - start);
    std::copy_n(m_records.begin() + static_cast<std::ptrdiff_t>(start),
                static_cast<std::ptrdiff_t>(first_chunk),
                out);
    if (count > first_chunk) {
        std::copy_n(m_records.begin(),
                    static_cast<std::ptrdiff_t>(count - first_chunk),
                    out + static_cast<std::ptrdiff_t>(first_chunk));
    }
    return true;
}

template <typename T, std::size_t Capacity>
std::size_t RingBuffer<T, Capacity>::_wrapIndexP1(std::size_t idx) const {
    return idx & (Capacity - 1);
}
