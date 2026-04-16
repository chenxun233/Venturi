#include "../common/thread_affinity.h"

#include <gtest/gtest.h>

TEST(ThreadAffinityTest, buildSingleCpuSetMarksOnlyRequestedCpu) {
    const cpu_set_t mask = buildSingleCpuSet(3);

    EXPECT_TRUE(CPU_ISSET(3, &mask));
    EXPECT_EQ(CPU_COUNT(&mask), 1);
}

TEST(ThreadAffinityTest, clearCpuSetEntriesRemovesRequestedBitsOnly) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    CPU_SET(1, &mask);
    CPU_SET(4, &mask);

    clearCpuSetEntries(mask, {0, 4});

    EXPECT_FALSE(CPU_ISSET(0, &mask));
    EXPECT_TRUE(CPU_ISSET(1, &mask));
    EXPECT_FALSE(CPU_ISSET(4, &mask));
    EXPECT_EQ(CPU_COUNT(&mask), 1);
}

TEST(ThreadAffinityTest, hasAnyCpuSetEntriesDetectsEmptyAndNonEmptyMasks) {
    cpu_set_t empty_mask;
    CPU_ZERO(&empty_mask);
    EXPECT_FALSE(hasAnyCpuSetEntries(empty_mask));

    const cpu_set_t non_empty_mask = buildSingleCpuSet(2);
    EXPECT_TRUE(hasAnyCpuSetEntries(non_empty_mask));
}

TEST(ThreadAffinityTest, readCurrentThreadCpuSetReturnsANonEmptyMask) {
    const cpu_set_t mask = readCurrentThreadCpuSet();
    EXPECT_TRUE(hasAnyCpuSetEntries(mask));
}
