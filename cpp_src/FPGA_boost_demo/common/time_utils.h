#pragma once

#include <cstdint>
#include <ctime>

inline uint64_t readMonotonicRawNs() {
    timespec ts {};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL) +
           static_cast<uint64_t>(ts.tv_nsec);
}
