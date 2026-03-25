#pragma once

#include <cstddef>

class LatencyTracker;

class Tuner {
public:
    Tuner();

    void attachTracker(LatencyTracker* tracker);
    void tune();
    std::size_t readTargetBatchSize() const;
    void writeTargetBatchSize(std::size_t target_batch_size);

private:
    LatencyTracker* m_tracker {nullptr};
    std::size_t m_target_batch_size {8};
};
