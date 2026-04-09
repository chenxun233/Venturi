#include "../driver/fake_fpga_dev.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../rx_engine/fpga_rx_engine.h"

#include <gtest/gtest.h>

TEST(FpgaRxEngineTest, pollsDecodedBatchWithoutOwningLatencyOrRegressionSideEffects) {
    FakeFPGADev device(1);
    device.setRawSlots(0, {FakeFPGADev::RawSlot {}});
    device.setProdPtr(0, 1U);

    FPGARxDecoder decoder;
    FPGARxEngine engine(device, decoder, 0);

    FpgaSyncSnapshot snapshot {};
    FirstEventMask mask {};
    const std::size_t count = engine.pollDecodedBatchSync(mask, MAX_POLL_RECORDS, true, snapshot);

    EXPECT_EQ(count, 1U);
}
