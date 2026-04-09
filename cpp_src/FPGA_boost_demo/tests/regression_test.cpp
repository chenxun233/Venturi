#include "../sync/regression.h"

#include <gtest/gtest.h>
#include <cmath>

namespace {

constexpr uint64_t kOffsetNs = 123456789ULL;
constexpr long double kSlopeNsPerTick = 6.3998296L;
constexpr uint64_t kFirstTestTick = 1000000ULL;
constexpr uint64_t kLastTestTick = 12000000ULL;
constexpr uint64_t kTickStep = 500000ULL;

uint64_t readHostNs(uint64_t fpga_tick) {
    return static_cast<uint64_t>(
        std::llround(static_cast<long double>(kOffsetNs) +
                     static_cast<long double>(fpga_tick) * kSlopeNsPerTick));
}

} // namespace

TEST(RegressionTest, estimatesSlopeNearFpgaTickPeriod) {
    Regression regression;

    for (uint64_t fpga_tick = kFirstTestTick; fpga_tick <= kLastTestTick; fpga_tick += kTickStep) {
        const FpgaSyncSnapshot snapshot {
            .fpga_tick = fpga_tick,
            .host_time_ns = readHostNs(fpga_tick),
            .interval_ns = 100
        };
        regression.updateSnapshot(snapshot);
    }

    const RegressionPara para = regression.returnParaSnapshot();
    ASSERT_TRUE(para.has_para);
    EXPECT_TRUE(regression.isFrozen());

    const double a_ns_per_tick =
        static_cast<double>(para.a_q32) / static_cast<double>(1ULL << 32);
    EXPECT_NEAR(a_ns_per_tick, static_cast<double>(kSlopeNsPerTick), 5e-5);

    uint64_t host_time_ns = 0;
    ASSERT_TRUE(regression.convertFpgaToHostTime(7000000, host_time_ns));
    EXPECT_NEAR(static_cast<double>(host_time_ns),
                static_cast<double>(readHostNs(7000000)),
                1.0);
}

TEST(RegressionTest, keepsFrozenSlopeWhileRefreshingSnapshot) {
    Regression regression;

    for (uint64_t fpga_tick = kFirstTestTick; fpga_tick <= kLastTestTick; fpga_tick += kTickStep) {
        regression.updateSnapshot(FpgaSyncSnapshot {
            .fpga_tick = fpga_tick,
            .host_time_ns = readHostNs(fpga_tick),
            .interval_ns = 100
        });
    }

    ASSERT_TRUE(regression.isFrozen());
    const RegressionPara para_before = regression.returnParaSnapshot();
    const FpgaSyncSnapshot refreshed_snapshot {
        .fpga_tick = 20000,
        .host_time_ns = readHostNs(20000),
        .interval_ns = 120
    };

    regression.updateSnapshot(refreshed_snapshot);

    const RegressionPara para_after = regression.returnParaSnapshot();
    const FpgaSyncSnapshot latest_snapshot = regression.readSnapshot();
    EXPECT_EQ(para_after.a_q32, para_before.a_q32);
    EXPECT_EQ(latest_snapshot.fpga_tick, refreshed_snapshot.fpga_tick);
    EXPECT_EQ(latest_snapshot.host_time_ns, refreshed_snapshot.host_time_ns);
    EXPECT_EQ(latest_snapshot.interval_ns, refreshed_snapshot.interval_ns);
}

TEST(RegressionTest, exposesConvertedStatusForLogging) {
    Regression regression;
    for (uint64_t idx = 0; idx < 20; ++idx) {
        regression.updateSnapshot(FpgaSyncSnapshot {
            .fpga_tick = 1000 + idx * 200,
            .host_time_ns = 500000 + idx * 1625,
            .interval_ns = 1000
        });
    }

    const RegressionStatusLogRecord status = regression.readStatusLogRecord();
    EXPECT_TRUE(status.has_para);
    EXPECT_GT(status.a_ns_per_tick, 0.0);
}
