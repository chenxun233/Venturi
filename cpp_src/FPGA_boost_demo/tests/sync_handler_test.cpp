#include "../sync/sync_handler.h"
#include "../sync/regression.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace {

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

} // namespace

TEST(SyncHandlerTest, initSyncStopsWhenRegressionFreezes) {
    SequencedSyncDevice device {};
    device.snapshots.reserve(32);
    for (uint64_t idx = 0; idx < 32; ++idx) {
        device.snapshots.push_back(FpgaSyncSnapshot {
            .fpga_tick = 1000 + idx * 200,
            .host_time_ns = 500000 + idx * 1250,
            .interval_ns = 1000
        });
    }

    Regression regression;
    SyncHandler sync_handler(64);
    FpgaSyncSnapshot snapshot {};

    EXPECT_TRUE(sync_handler.initSync(device, regression, snapshot, 64, 2000));
    EXPECT_TRUE(regression.isFrozen());
    EXPECT_LT(device.read_count, 64U);
}
