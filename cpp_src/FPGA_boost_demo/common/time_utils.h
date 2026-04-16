#pragma once

#include <cstdint>
#include <ctime>
#include <stdexcept>

#if defined(__i386__) || defined(__x86_64__)
#include <x86intrin.h>
#endif

struct HostTickScale {
    uint64_t tsc_hz {0};
    bool use_clock_fallback {false};
};

inline uint64_t readClockMonotonicRawNsSlow() {
    timespec ts {};
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0) {
        throw std::runtime_error("clock_gettime(CLOCK_MONOTONIC_RAW) failed");
    }
    return (static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL) +
           static_cast<uint64_t>(ts.tv_nsec);
}

inline uint64_t readInvariantTscTicks() {
#if defined(__i386__) || defined(__x86_64__)
    unsigned int aux = 0;
    return __rdtscp(&aux);
#else
    return 0;
#endif
}

inline HostTickScale calibrateHostTickScale() {
    HostTickScale scale {};

#if defined(__i386__) || defined(__x86_64__)
    constexpr uint64_t kCalibrationWindowNs = 20000000ULL;

    const uint64_t start_ns = readClockMonotonicRawNsSlow();
    const uint64_t start_tick = readInvariantTscTicks();

    uint64_t end_ns = start_ns;
    uint64_t end_tick = start_tick;
    while ((end_ns - start_ns) < kCalibrationWindowNs) {
        end_ns = readClockMonotonicRawNsSlow();
        end_tick = readInvariantTscTicks();
    }

    const uint64_t delta_ns = end_ns - start_ns;
    const uint64_t delta_tick = end_tick - start_tick;
    if (delta_ns != 0 && delta_tick != 0) {
        scale.tsc_hz = static_cast<uint64_t>(
            (static_cast<__uint128_t>(delta_tick) * 1000000000ULL) / delta_ns);
        if (scale.tsc_hz != 0) {
            return scale;
        }
    }
#endif

    scale.use_clock_fallback = true;
    return scale;
}

inline const HostTickScale& readHostTickScale() {
    static const HostTickScale scale = calibrateHostTickScale();
    return scale;
}

inline uint64_t readMonotonicRawNs() {
    const HostTickScale& scale = readHostTickScale();
    if (scale.use_clock_fallback) {
        return readClockMonotonicRawNsSlow();
    }
    return readInvariantTscTicks();
}
