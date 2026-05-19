// bar_latency_probe.cpp
//
// Standalone micro-benchmark for the PCIe MMIO read used by the venturi
// hot path. Loops `readProdPtr(queue=0)` N times, captures TSC ticks
// per call, converts to nanoseconds via a one-shot calibration, and
// prints a distribution + coarse histogram.
//
// Goal: isolate the cost of a single uncacheable 64-bit BAR0 load
// from everything else in `pollDecodedBatch`. If the p99 here matches
// the p99 of BATCH_DURATION, PCIe MMIO jitter is the dominant cause.
//
// Usage: sudo ./bar_latency_probe <pci_address> [iterations]
//   e.g.: sudo ./bar_latency_probe 0000:01:00.0 1000000

#include "fpga_dev.h"
#include "../common/time_utils.h"
#include "../common/thread_affinity.h"
#include "../../common/log.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kDefaultIterations = 1000000;
constexpr std::size_t kWarmupIterations = 1000;
constexpr int kProbeCpu = 2;

void printPercentiles(std::vector<uint64_t>& samples_ns) {
    std::sort(samples_ns.begin(), samples_ns.end());
    const std::size_t n = samples_ns.size();
    if (n == 0) {
        std::printf("(no samples)\n");
        return;
    }

    auto pct = [&](double p) -> uint64_t {
        const std::size_t idx = static_cast<std::size_t>(
            static_cast<double>(n - 1) * (p / 100.0));
        return samples_ns[idx];
    };

    std::printf("\nreadProdPtr latency (n=%zu)\n", n);
    std::printf("%-10s %12s\n", "percentile", "ns");
    std::printf("%-10s %12llu\n", "min",  (unsigned long long)samples_ns.front());
    std::printf("%-10s %12llu\n", "p50",  (unsigned long long)pct(50.0));
    std::printf("%-10s %12llu\n", "p90",  (unsigned long long)pct(90.0));
    std::printf("%-10s %12llu\n", "p99",  (unsigned long long)pct(99.0));
    std::printf("%-10s %12llu\n", "p99.9",(unsigned long long)pct(99.9));
    std::printf("%-10s %12llu\n", "max",  (unsigned long long)samples_ns.back());
}

void printHistogram(const std::vector<uint64_t>& samples_ns) {
    // Coarse log-scale buckets: 0-100, 100-200, 200-400, 400-800, 800-1600, ...
    constexpr std::array<uint64_t, 9> bucket_edges {
        100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600,
    };
    std::array<std::size_t, bucket_edges.size() + 1> buckets {};

    for (uint64_t v : samples_ns) {
        std::size_t b = bucket_edges.size();
        for (std::size_t i = 0; i < bucket_edges.size(); ++i) {
            if (v < bucket_edges[i]) { b = i; break; }
        }
        ++buckets[b];
    }

    std::printf("\nhistogram (ns ranges, count, %% of total):\n");
    const std::size_t n = samples_ns.size();
    uint64_t lo = 0;
    for (std::size_t i = 0; i < bucket_edges.size(); ++i) {
        const uint64_t hi = bucket_edges[i];
        const double pct = (100.0 * static_cast<double>(buckets[i])) /
                           static_cast<double>(n == 0 ? 1 : n);
        std::printf("  [%6llu, %6llu) : %10zu  (%5.2f%%)\n",
                    (unsigned long long)lo, (unsigned long long)hi,
                    buckets[i], pct);
        lo = hi;
    }
    const double pct_tail =
        (100.0 * static_cast<double>(buckets.back())) /
        static_cast<double>(n == 0 ? 1 : n);
    std::printf("  [%6llu,    inf) : %10zu  (%5.2f%%)\n",
                (unsigned long long)lo, buckets.back(), pct_tail);
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::fprintf(stderr,
                     "Usage: %s <pci_address> [iterations]\n"
                     "  e.g.: sudo %s 0000:01:00.0 1000000\n",
                     argv[0], argv[0]);
        return 1;
    }

    const std::string pci_addr = argv[1];
    const std::size_t iterations =
        (argc == 3) ? static_cast<std::size_t>(std::strtoull(argv[2], nullptr, 10))
                    : kDefaultIterations;
    if (iterations == 0) {
        std::fprintf(stderr, "iterations must be > 0\n");
        return 1;
    }

    // Pin to the same kind of P-core that rx_thread0 uses, so the probe
    // measures from the same TSC and the same uncore-side conditions.
    try {
        pinCurrentThreadToCpu(kProbeCpu);
    } catch (const std::exception& ex) {
        std::fprintf(stderr,
                     "Warning: failed to pin probe thread to CPU %d: %s\n",
                     kProbeCpu, ex.what());
    }

    info("BAR latency probe: pci=%s iterations=%zu probe_cpu=%d",
         pci_addr.c_str(), iterations, kProbeCpu);

    std::unique_ptr<FPGADev> dev;
    try {
        dev = std::make_unique<FPGADev>(pci_addr);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "Failed to create FPGA device: %s\n", ex.what());
        return 1;
    }
    if (!dev->initHardware()) {
        std::fprintf(stderr,
                     "initHardware failed. Check VFIO binding and PCI address.\n");
        return 1;
    }

    const HostTickScale scale = calibrateHostTickScale();
    if (scale.use_clock_fallback || scale.tsc_hz == 0U) {
        std::fprintf(stderr, "TSC calibration failed; cannot run probe.\n");
        return 1;
    }
    info("TSC hz = %llu (%.3f GHz)",
         (unsigned long long)scale.tsc_hz,
         static_cast<double>(scale.tsc_hz) / 1e9);

    // Warmup: drain any cold-start oddities and let HWP pin frequency.
    for (std::size_t i = 0; i < kWarmupIterations; ++i) {
        uint64_t prod = 0;
        dev->readProdPtr(0, prod);
        asm volatile("" : : "r"(prod) : "memory");
    }

    std::vector<uint64_t> samples_ticks;
    samples_ticks.reserve(iterations);

    // Measurement loop. Single 64-bit MMIO read between two rdtscp's,
    // back to back, no other work. Compiler barrier prevents reordering.
    for (std::size_t i = 0; i < iterations; ++i) {
        const uint64_t t0 = readInvariantTscTicks();
        uint64_t prod = 0;
        dev->readProdPtr(0, prod);
        asm volatile("" : : "r"(prod) : "memory");
        const uint64_t t1 = readInvariantTscTicks();
        samples_ticks.push_back(t1 - t0);
    }

    std::vector<uint64_t> samples_ns;
    samples_ns.reserve(samples_ticks.size());
    for (uint64_t ticks : samples_ticks) {
        samples_ns.push_back(
            (static_cast<__uint128_t>(ticks) * 1000000000ULL) / scale.tsc_hz);
    }

    printPercentiles(samples_ns);
    printHistogram(samples_ns);

    return 0;
}
