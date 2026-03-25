#include "tuner.h"

Tuner::Tuner() = default;

void Tuner::attachTracker(LatencyTracker* tracker) {
    m_tracker = tracker;
}

void Tuner::tune() {
    (void)m_tracker;
}

std::size_t Tuner::readTargetBatchSize() const {
    return m_target_batch_size;
}

void Tuner::writeTargetBatchSize(std::size_t target_batch_size) {
    m_target_batch_size = target_batch_size;
}



// void Tuner::adjustMaxBatch(std::size_t decoded_count) {
//     constexpr std::size_t kMinPollCount = 8;

//     if (decoded_count >= m_max_poll_batch && m_max_poll_batch < m_event_buffer.size()) {
//         m_max_poll_batch = std::min(m_event_buffer.size(), m_max_poll_batch +4);
//         return;
//     }

//     if (decoded_count <= (m_max_poll_batch / 4) && m_max_poll_batch > kMinPollCount) {
//         m_max_poll_batch = std::max(kMinPollCount, m_max_poll_batch -4);
//     }
// }