#include "../latency/trace_buffer.h"

#include <gtest/gtest.h>

TEST(TraceBufferOverwriteTest, pushDropOldestOverwritesOldestRecord) {
    TraceBuffer<uint32_t> buffer(2);
    ASSERT_TRUE(buffer.pushDropOldest(1));
    ASSERT_TRUE(buffer.pushDropOldest(2));
    EXPECT_FALSE(buffer.pushDropOldest(3));

    uint32_t value = 0;
    ASSERT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 2U);
}
