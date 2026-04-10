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

void appendStableSnapshots(SequencedSyncDevice& device,
                           std::size_t count,
                           uint64_t first_tick = 1000ULL,
                           uint64_t tick_step = 200ULL) {
    device.snapshots.reserve(count);
    for (std::size_t idx = 0; idx < count; ++idx) {
        const uint64_t fpga_tick = first_tick + static_cast<uint64_t>(idx) * tick_step;
        device.snapshots.push_back(FpgaSyncSnapshot {
            .fpga_tick = fpga_tick,
            .host_time_ns = readHostNs(fpga_tick),
            .interval_ns = 1000ULL
        });
    }
}

} // namespace

TEST(RegressionTest, exposesRegressionStatusAfterSnapshotUpdates) {
    SequencedSyncDevice device {};
    appendStableSnapshots(device, 256);
    FPGARegression regression;

    ASSERT_TRUE(regression.initSync(device, device.snapshots.size(), kAcceptedIntervalNs));

    const RegressionStatusLogRecord status = regression.readStatusLogRecord();
    ASSERT_TRUE(status.has_para);
    EXPECT_NEAR(status.a_ns_per_tick, static_cast<double>(kSlopeNsPerTick), 5e-4);
}

TEST(RegressionTest, convertsHostTimeUsingCurrentSnapshotAnchor) {
    SequencedSyncDevice device {};
    appendStableSnapshots(device, 256);
    FPGARegression regression;

    ASSERT_TRUE(regression.initSync(device, device.snapshots.size(), kAcceptedIntervalNs));

    uint64_t host_time_ns = 0;
    const uint64_t converted_tick = device.snapshots.back().fpga_tick + 200ULL;
    ASSERT_TRUE(regression.convertFpgaToHostTime(converted_tick, host_time_ns));
    EXPECT_NEAR(static_cast<double>(host_time_ns),
                static_cast<double>(readHostNs(converted_tick)),
                10.0);
}

TEST(RegressionTest, initSyncStopsWhenRegressionFreezes) {
    SequencedSyncDevice device {};
    appendStableSnapshots(device, 256);
    FPGARegression regression(64);

    EXPECT_TRUE(regression.initSync(device, device.snapshots.size(), kAcceptedIntervalNs));
    EXPECT_LT(device.read_count, device.snapshots.size());
}

TEST(RegressionTest, rejectsInvalidIntervalsAndAcceptsQualifiedOnes) {
    FPGARegression regression;

    EXPECT_FALSE(regression.tryAcceptSnapshot(FpgaSyncSnapshot {
        .fpga_tick = 1000,
        .host_time_ns = 500000,
        .interval_ns = 0
    }, kAcceptedIntervalNs));

    EXPECT_FALSE(regression.tryAcceptSnapshot(FpgaSyncSnapshot {
        .fpga_tick = 1200,
        .host_time_ns = 501250,
        .interval_ns = 4000
    }, kAcceptedIntervalNs));

    EXPECT_TRUE(regression.tryAcceptSnapshot(FpgaSyncSnapshot {
        .fpga_tick = 1400,
        .host_time_ns = 502500,
        .interval_ns = 1000
    }, kAcceptedIntervalNs));
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
