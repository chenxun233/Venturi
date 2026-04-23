#include "../common/fixed_circular_buffer.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>

namespace {

TEST(FixedCircularBufferTest, pushBackAndPopFrontPreserveOrderAcrossWraparound) {
    RingBuffer<uint32_t, 4> buffer;

    EXPECT_TRUE(buffer.pushBack(1U));
    EXPECT_TRUE(buffer.pushBack(2U));
    EXPECT_TRUE(buffer.pushBack(3U));
    EXPECT_EQ(buffer.readFront(), 1U);
    EXPECT_TRUE(buffer.eraseFront());
    EXPECT_EQ(buffer.readFront(), 2U);
    EXPECT_TRUE(buffer.eraseFront());

    EXPECT_TRUE(buffer.pushBack(4U));
    EXPECT_TRUE(buffer.pushBack(5U));
    EXPECT_EQ(buffer.readSize(), 3U);
    EXPECT_EQ(buffer.readFront(), 3U);
    EXPECT_TRUE(buffer.eraseFront());
    EXPECT_EQ(buffer.readFront(), 4U);
    EXPECT_TRUE(buffer.eraseFront());
    EXPECT_EQ(buffer.readFront(), 5U);
    EXPECT_TRUE(buffer.eraseFront());
    EXPECT_TRUE(buffer.isEmpty());
}

TEST(FixedCircularBufferTest, writeCopiesWrappedByteRangesAndPopFrontNAdvancesPrefix) {
    RingBuffer<uint8_t, 8> buffer;
    const std::array<uint8_t, 5> first {1, 2, 3, 4, 5};
    const std::array<uint8_t, 4> second {6, 7, 8, 9};
    std::array<uint8_t, 4> prefix {};

    EXPECT_TRUE(buffer.write(first.data(), first.size()));
    EXPECT_TRUE(buffer.eraseFrontN(3));
    EXPECT_TRUE(buffer.write(second.data(), second.size()));
    EXPECT_EQ(buffer.readSize(), 6U);
    EXPECT_EQ(buffer.readAt(0), 4U);
    EXPECT_EQ(buffer.readAt(1), 5U);
    EXPECT_EQ(buffer.readAt(2), 6U);
    EXPECT_EQ(buffer.readAt(5), 9U);

    EXPECT_TRUE(buffer.copyFrom(0, prefix.data(), prefix.size()));

    EXPECT_EQ(prefix[0], 4U);
    EXPECT_EQ(prefix[1], 5U);
    EXPECT_EQ(prefix[2], 6U);
    EXPECT_EQ(prefix[3], 7U);
}

TEST(FixedCircularBufferTest, writeRejectsOverflowWithoutPartiallyAppending) {
    RingBuffer<uint8_t, 4> buffer;
    const std::array<uint8_t, 4> full {1, 2, 3, 4};
    const std::array<uint8_t, 2> overflow {5, 6};

    EXPECT_TRUE(buffer.write(full.data(), full.size()));
    EXPECT_FALSE(buffer.write(overflow.data(), overflow.size()));
    EXPECT_EQ(buffer.readSize(), 4U);
    EXPECT_EQ(buffer.readFront(), 1U);
}

} // namespace
