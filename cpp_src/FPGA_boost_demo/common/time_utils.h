#pragma once

#include <cstdint>
#include <ctime>
#include <stdexcept>

inline uint64_t readMonotonicRawNs() {
    timespec ts {};
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0) {
        throw std::runtime_error("clock_gettime(CLOCK_MONOTONIC_RAW) failed");
    }
    return (static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL) +
           static_cast<uint64_t>(ts.tv_nsec);
}
