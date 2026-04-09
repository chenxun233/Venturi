#include "../sync/sync_handler.h"
#include "../sync/regression.h"
#include "../driver/fake_fpga_dev.h"

#include <gtest/gtest.h>

TEST(SyncHandlerTest, initSyncStopsWhenRegressionFreezes) {
    FakeFPGADev device(1);
    device.setSyncSnapshot(0, 0, 1000, 1000000, 1000);

    Regression regression;
    SyncHandler sync_handler(64);
    FpgaSyncSnapshot snapshot {};

    EXPECT_TRUE(sync_handler.initSync(device, regression, snapshot, 64, 2000));
    EXPECT_TRUE(regression.isFrozen());
}
