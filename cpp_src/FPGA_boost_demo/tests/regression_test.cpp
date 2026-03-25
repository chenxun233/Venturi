#include "../sync/regression.h"

#include <gtest/gtest.h>

#include <atomic>

namespace {

constexpr uint64_t kOffsetNs = 123456789ULL;
constexpr uint64_t kTickNumer = 32;
constexpr uint64_t kTickDenom = 5;

uint64_t readHostNs(uint64_t fpga_tick) {
    return kOffsetNs + ((fpga_tick * kTickNumer) / kTickDenom);
}

} // namespace

TEST(RegressionTest, estimatesSlopeNearFpgaTickPeriod) {
    Regression regression;
    std::atomic<bool> ready {false};

    for (uint64_t fpga_tick = 1000; fpga_tick <= 12000; fpga_tick += 500) {
        const FpgaSyncSnapshot snapshot {
            .fpga_tick = fpga_tick,
            .host_time_ns = readHostNs(fpga_tick),
            .interval_ns = 100
        };
        ready.store(true, std::memory_order_release);
        regression.run(ready, snapshot);
    }

    const RegressionPara para = regression.readParaSnapshot();
    ASSERT_TRUE(para.has_para);
    EXPECT_TRUE(regression.isFrozen());

    const double a_ns_per_tick =
        static_cast<double>(para.a_q32) / static_cast<double>(1ULL << 32);
    EXPECT_NEAR(a_ns_per_tick, 6.4, 1e-6);

    uint64_t host_time_ns = 0;
    ASSERT_TRUE(regression.convertFpgaToHostTime(7000, host_time_ns));
    EXPECT_NEAR(static_cast<double>(host_time_ns),
                static_cast<double>(readHostNs(7000)),
                1.0);
}
