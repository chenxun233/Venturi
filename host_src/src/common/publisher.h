#pragma once

#include <atomic>
#include <memory>
#include <utility>

template <typename T>
class Publisher {
public:
    explicit Publisher(T initial)
        : m_current(std::make_shared<const T>(std::move(initial))) {
    }

    void publish(T value) {
        m_current.store(std::make_shared<const T>(std::move(value)),
                        std::memory_order_release);
    }

    std::shared_ptr<const T> load() const {
        return m_current.load(std::memory_order_acquire);
    }

private:
    std::atomic<std::shared_ptr<const T>> m_current;
};
