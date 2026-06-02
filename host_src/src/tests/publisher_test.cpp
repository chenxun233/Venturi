#include "../common/publisher.h"

#include <gtest/gtest.h>

namespace {

struct PublisherPayload {
    int value {0};
    bool ready {false};
};

} // namespace

TEST(PublisherTest, exposesInitialValueImmediately) {
    Publisher<PublisherPayload> publisher(PublisherPayload {
        .value = 7,
        .ready = true
    });

    const auto snapshot = publisher.load();
    ASSERT_NE(snapshot, nullptr);
    EXPECT_EQ(snapshot->value, 7);
    EXPECT_TRUE(snapshot->ready);
}

TEST(PublisherTest, exposesNewSnapshotAfterPublish) {
    Publisher<PublisherPayload> publisher(PublisherPayload {});
    const auto old_snapshot = publisher.load();

    publisher.publish(PublisherPayload {
        .value = 42,
        .ready = true
    });

    const auto new_snapshot = publisher.load();
    ASSERT_NE(new_snapshot, nullptr);
    EXPECT_EQ(new_snapshot->value, 42);
    EXPECT_TRUE(new_snapshot->ready);
    EXPECT_EQ(old_snapshot->value, 0);
    EXPECT_FALSE(old_snapshot->ready);
}
