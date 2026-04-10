#include "../sync/FPGA_regression.h"

#include <gtest/gtest.h>

#include <cmath>
#include <type_traits>
#include <vector>

static_assert(std::is_member_function_pointer_v<decltype(&FPGARegression::run)>);
static_assert(std::is_member_function_pointer_v<decltype(&FPGARegression::tryAcceptSnapshot)>);

namespace {

constexpr uint64_t kOffsetNs = 123456789ULL;
constexpr long double kSlopeNsPerTick = 6.25L;
constexpr uint64_t kAcceptedIntervalNs = 2000ULL;

uint64_t readHostNs(uint64_t fpga_tick) {
    return static_cast<uint64_t>(
        std::llround(static_cast<long double>(kOffsetNs) +
                     static_cast<long double>(fpga_tick) * kSlopeNsPerTick));
}

FpgaSyncSnapshot makeSnapshot(uint64_t fpga_tick, uint64_t interval_ns = 1000ULL) {
    return FpgaSyncSnapshot {
        .fpga_tick = fpga_tick,
        .host_time_ns = readHostNs(fpga_tick),
        .interval_ns = interval_ns
    };
}

struct SequencedSyncDevice {
    std::vector<FpgaSyncSnapshot> snapshots {};
    mutable std::size_t read_count {0};

    bool readSyncTimestamp(FpgaSyncSnapshot& snapshot) const {
        if (read_count >= snapshots.size()) {
            return false;
        }
        snapshot = snapshots[read_count];
        ++read_count;
        return true;
    }
};

void feedStableSnapshots(FPGARegression& regression) {
    for (uint64_t fpga_tick : {1000000ULL, 2000000ULL, 3000000ULL, 4000000ULL, 5000000ULL}) {
        ASSERT_TRUE(regression.tryAcceptSnapshot(makeSnapshot(fpga_tick), kAcceptedIntervalNs));
        regression.updateRegression();
    }
}

} // namespace

TEST(RegressionTest, freezesAfterConfiguredStableRawEstimates) {
    FPGARegression regression;

    feedStableSnapshots(regression);

    EXPECT_TRUE(regression.isFrozen());

    const RegressionStatusLogRecord status = regression.readStatusLogRecord();
    ASSERT_TRUE(status.has_para);
    EXPECT_NEAR(status.a_ns_per_tick, static_cast<double>(kSlopeNsPerTick), 5e-5);
}

TEST(RegressionTest, returnsFalseForConversionBeforeFreeze) {
    FPGARegression regression;
    uint64_t host_time_ns = 0;

    ASSERT_TRUE(regression.tryAcceptSnapshot(makeSnapshot(1000000ULL), kAcceptedIntervalNs));

    EXPECT_FALSE(regression.convertFpgaToHostTime(1000000ULL, host_time_ns));
}

TEST(RegressionTest, refreshesLatestAnchorWithoutChangingFrozenSlope) {
    FPGARegression regression;
    feedStableSnapshots(regression);

    const RegressionStatusLogRecord status_before = regression.readStatusLogRecord();
    const FpgaSyncSnapshot anchor {
        .fpga_tick = 7000000ULL,
        .host_time_ns = readHostNs(7000000ULL) + 37ULL,
        .interval_ns = 1000ULL
    };

    ASSERT_TRUE(regression.tryAcceptSnapshot(anchor, kAcceptedIntervalNs));

    const RegressionStatusLogRecord status_after = regression.readStatusLogRecord();
    EXPECT_DOUBLE_EQ(status_after.a_ns_per_tick, status_before.a_ns_per_tick);

    uint64_t converted_host_time_ns = 0;
    ASSERT_TRUE(regression.convertFpgaToHostTime(anchor.fpga_tick, converted_host_time_ns));
    EXPECT_EQ(converted_host_time_ns, anchor.host_time_ns);
}

TEST(RegressionTest, convertsTicksBeforeAndAfterLatestAnchor) {
    FPGARegression regression;
    feedStableSnapshots(regression);

    const FpgaSyncSnapshot anchor {
        .fpga_tick = 7000000ULL,
        .host_time_ns = readHostNs(7000000ULL) + 37ULL,
        .interval_ns = 1000ULL
    };
    ASSERT_TRUE(regression.tryAcceptSnapshot(anchor, kAcceptedIntervalNs));

    uint64_t before_anchor_host_ns = 0;
    uint64_t after_anchor_host_ns = 0;

    ASSERT_TRUE(regression.convertFpgaToHostTime(6500000ULL, before_anchor_host_ns));
    ASSERT_TRUE(regression.convertFpgaToHostTime(7500000ULL, after_anchor_host_ns));

    EXPECT_NEAR(static_cast<double>(before_anchor_host_ns),
                static_cast<double>(anchor.host_time_ns) - 500000.0 * static_cast<double>(kSlopeNsPerTick),
                1.0);
    EXPECT_NEAR(static_cast<double>(after_anchor_host_ns),
                static_cast<double>(anchor.host_time_ns) + 500000.0 * static_cast<double>(kSlopeNsPerTick),
                1.0);
}

TEST(RegressionTest, initSyncStopsWhenRegressionFreezes) {
    SequencedSyncDevice device {};
    device.snapshots = {
        makeSnapshot(1000000ULL),
        makeSnapshot(2000000ULL),
        makeSnapshot(3000000ULL),
        makeSnapshot(4000000ULL),
        makeSnapshot(5000000ULL),
        makeSnapshot(6000000ULL),
    };

    FPGARegression regression(64);

    EXPECT_TRUE(regression.initSync(device, 64, kAcceptedIntervalNs));
    EXPECT_TRUE(regression.isFrozen());
    EXPECT_LE(device.read_count, 5U);
}

TEST(RegressionTest, rejectsInvalidIntervalsBeforeAndAfterFreeze) {
    FPGARegression regression;

    EXPECT_FALSE(regression.tryAcceptSnapshot(makeSnapshot(1000000ULL, 0ULL), kAcceptedIntervalNs));
    EXPECT_FALSE(regression.tryAcceptSnapshot(makeSnapshot(1000000ULL, 4000ULL), kAcceptedIntervalNs));

    feedStableSnapshots(regression);
    ASSERT_TRUE(regression.isFrozen());

    EXPECT_FALSE(regression.tryAcceptSnapshot(makeSnapshot(7000000ULL, 0ULL), kAcceptedIntervalNs));
    EXPECT_FALSE(regression.tryAcceptSnapshot(makeSnapshot(7000000ULL, 4000ULL), kAcceptedIntervalNs));
    EXPECT_TRUE(regression.tryAcceptSnapshot(makeSnapshot(7000000ULL, 1000ULL), kAcceptedIntervalNs));
}

TEST(RegressionTest, runRequestsCaptureAtConfiguredCadence) {
    FPGARegression regression(3);
    CapSignal capture_signal {};

    regression.run(capture_signal);
    EXPECT_TRUE(capture_signal.request.exchange(false));

    regression.run(capture_signal);
    EXPECT_FALSE(capture_signal.request.exchange(false));

    regression.run(capture_signal);
    EXPECT_FALSE(capture_signal.request.exchange(false));

    regression.run(capture_signal);
    EXPECT_TRUE(capture_signal.request.exchange(false));
}
