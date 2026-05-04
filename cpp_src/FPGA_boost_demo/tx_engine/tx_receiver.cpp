#include "tx_receiver.h"

#include "../common/thread_affinity.h"

#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdio>
#include <mutex>
#include <stdexcept>

namespace {

uint16_t readBigEndian16(const uint8_t* bytes) {
    return static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8U) |
                                 static_cast<uint16_t>(bytes[1]));
}

uint32_t readBigEndian32(const uint8_t* bytes) {
    return (static_cast<uint32_t>(bytes[0]) << 24U) |
           (static_cast<uint32_t>(bytes[1]) << 16U) |
           (static_cast<uint32_t>(bytes[2]) << 8U) |
           static_cast<uint32_t>(bytes[3]);
}

} // namespace

TxReceiver::TxReceiver(std::size_t pending_capacity, std::size_t sent_record_capacity)
    : m_sent_records(sent_record_capacity),
      m_pending_slots(pending_capacity),
      m_pending_mask(pending_capacity - 1U) {
    if (pending_capacity == 0 || (pending_capacity & (pending_capacity - 1U)) != 0) {
        throw std::invalid_argument("TxReceiver pending_capacity must be a non-zero power of two");
    }
}

TxReceiver::~TxReceiver() {
    stop();
    _closeRecvFd();
}

void TxReceiver::attachQueueIdx(uint16_t queue_idx) {
    m_queue_idx = queue_idx;
}

void TxReceiver::setWorkerCpu(int cpu_id) {
    m_worker_cpu = cpu_id;
}

bool TxReceiver::acceptSentOrder(const TxSentOrderRecord& record) {
    if (m_sent_records.push(record)) {
        return true;
    }

    const std::lock_guard<std::mutex> lock(m_state_mutex);
    ++m_stats.ref_drops;
    return false;
}

void TxReceiver::updateConnectionInfo(const TxConnectionInfo& info) {
    const std::lock_guard<std::mutex> lock(m_state_mutex);
    if (info.kind == TxConnectionKind::Connected) {
        _closeRecvFd();
        m_recv_fd = info.fd;
        m_generation = info.generation;
        m_frame_buffer.clear();
        ++m_stats.connected;
        return;
    }

    if (info.generation != m_generation) {
        return;
    }
    _closeRecvFd();
    m_generation = 0;
    m_frame_buffer.clear();
    ++m_stats.disconnected;
}

bool TxReceiver::pollOnce() {
    const std::lock_guard<std::mutex> lock(m_state_mutex);
    const uint64_t sent_before = m_stats.sent;
    _drainSentOrders();
    const bool read_frames = _readSocketFrames();
    return read_frames || m_stats.sent != sent_before;
}

void TxReceiver::start() {
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    m_thread = std::thread(&TxReceiver::_run, this);
}

void TxReceiver::stop() {
    m_running.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

TxReceiverStats TxReceiver::readStats() const {
    const std::lock_guard<std::mutex> lock(m_state_mutex);
    TxReceiverStats stats = m_stats;
    stats.pending = _countPending();
    return stats;
}

void TxReceiver::printSummary() const {
    const TxReceiverStats stats = readStats();
    std::printf("TxReceiver queue=%u sent=%llu accepted=%llu filled=%llu rejected=%llu "
                "pending=%llu malformed=%llu ref_drops=%llu connected=%llu disconnected=%llu\n",
                static_cast<unsigned int>(m_queue_idx),
                static_cast<unsigned long long>(stats.sent),
                static_cast<unsigned long long>(stats.accepted),
                static_cast<unsigned long long>(stats.filled),
                static_cast<unsigned long long>(stats.rejected),
                static_cast<unsigned long long>(stats.pending),
                static_cast<unsigned long long>(stats.malformed),
                static_cast<unsigned long long>(stats.ref_drops),
                static_cast<unsigned long long>(stats.connected),
                static_cast<unsigned long long>(stats.disconnected));
    std::fflush(stdout);
}

void TxReceiver::_drainSentOrders() {
    TxSentOrderRecord record {};
    while (m_sent_records.pop(record)) {
        ++m_stats.sent;
        _insertPendingOrder(record);
    }
}

bool TxReceiver::_readSocketFrames() {
    bool did_work = false;
    while (_appendSocketBytes()) {
        did_work = true;
    }

    return _parseBufferedFrames() || did_work;
}

bool TxReceiver::_appendSocketBytes() {
    if (m_recv_fd < 0) {
        return false;
    }

    std::array<uint8_t, 4096> bytes {};
    const ssize_t rc = ::recv(m_recv_fd,
                              bytes.data(),
                              bytes.size(),
                              MSG_DONTWAIT);
    if (rc > 0) {
        m_frame_buffer.insert(m_frame_buffer.end(), bytes.begin(), bytes.begin() + rc);
        return true;
    }

    if (rc == 0) {
        _markDisconnected();
        return true;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
        return false;
    }

    _markDisconnected();
    return true;
}

bool TxReceiver::_parseBufferedFrames() {
    bool did_work = false;
    std::size_t offset = 0;
    while (m_frame_buffer.size() - offset >= 2U) {
        const uint16_t encoded_length = readBigEndian16(m_frame_buffer.data() + offset);
        const std::size_t frame_size = static_cast<std::size_t>(encoded_length) + 2U;
        if (encoded_length == 0) {
            ++m_stats.malformed;
            offset += 2U;
            did_work = true;
            continue;
        }
        if (m_frame_buffer.size() - offset < frame_size) {
            break;
        }

        did_work = _parseSoupFrame(m_frame_buffer.data() + offset, frame_size) || did_work;
        offset += frame_size;
    }

    if (offset > 0) {
        m_frame_buffer.erase(m_frame_buffer.begin(),
                             m_frame_buffer.begin() + static_cast<std::ptrdiff_t>(offset));
    }
    return did_work;
}

bool TxReceiver::_parseSoupFrame(const uint8_t* frame, std::size_t frame_size) {
    if (frame_size < 3U) {
        ++m_stats.malformed;
        return true;
    }

    switch (frame[2]) {
        case static_cast<uint8_t>('A'):
        case static_cast<uint8_t>('H'):
        case static_cast<uint8_t>('J'):
            return true;
        case static_cast<uint8_t>('S'):
            return _handleSequencedData(frame, frame_size);
        default:
            ++m_stats.malformed;
            return true;
    }
}

bool TxReceiver::_handleSequencedData(const uint8_t* frame, std::size_t frame_size) {
    if (frame_size < 4U) {
        ++m_stats.malformed;
        return true;
    }

    const uint8_t ouch_type = frame[3];
    switch (ouch_type) {
        case static_cast<uint8_t>('A'):
            if (frame_size < 16U) {
                ++m_stats.malformed;
                return true;
            }
            _handleAccepted(readBigEndian32(frame + 12));
            return true;
        case static_cast<uint8_t>('E'):
            if (frame_size < 20U) {
                ++m_stats.malformed;
                return true;
            }
            _handleExecuted(readBigEndian32(frame + 12), readBigEndian32(frame + 16));
            return true;
        case static_cast<uint8_t>('J'):
            if (frame_size < 16U) {
                ++m_stats.malformed;
                return true;
            }
            _handleRejected(readBigEndian32(frame + 12));
            return true;
        default:
            ++m_stats.malformed;
            return true;
    }
}

void TxReceiver::_handleAccepted(uint32_t user_ref_num) {
    PendingSlot* slot = _lookupPendingSlot(user_ref_num);
    if (slot == nullptr) {
        ++m_stats.malformed;
        return;
    }
    if (!slot->accepted) {
        slot->accepted = true;
        ++m_stats.accepted;
    }
}

void TxReceiver::_handleExecuted(uint32_t user_ref_num, uint32_t executed_shares) {
    PendingSlot* slot = _lookupPendingSlot(user_ref_num);
    if (slot == nullptr) {
        ++m_stats.malformed;
        return;
    }

    slot->filled_shares += executed_shares;
    if (!slot->rejected && slot->filled_shares >= slot->record.shares) {
        ++m_stats.filled;
        _clearPendingSlot(*slot);
    }
}

void TxReceiver::_handleRejected(uint32_t user_ref_num) {
    PendingSlot* slot = _lookupPendingSlot(user_ref_num);
    if (slot == nullptr) {
        ++m_stats.malformed;
        return;
    }
    if (!slot->rejected) {
        slot->rejected = true;
        ++m_stats.rejected;
    }
    _clearPendingSlot(*slot);
}

void TxReceiver::_insertPendingOrder(const TxSentOrderRecord& record) {
    PendingSlot& slot = m_pending_slots[_computePendingSlotIndex(record.user_ref_num)];
    if (slot.occupied) {
        ++m_stats.ref_drops;
        return;
    }

    slot.occupied = true;
    slot.accepted = false;
    slot.rejected = false;
    slot.filled_shares = 0;
    slot.record = record;
}

void TxReceiver::_clearPendingSlot(PendingSlot& slot) {
    slot = PendingSlot {};
}

std::size_t TxReceiver::_computePendingSlotIndex(uint32_t user_ref_num) const {
    return static_cast<std::size_t>(user_ref_num) & m_pending_mask;
}

TxReceiver::PendingSlot* TxReceiver::_lookupPendingSlot(uint32_t user_ref_num) {
    PendingSlot& slot = m_pending_slots[_computePendingSlotIndex(user_ref_num)];
    if (!slot.occupied || slot.record.user_ref_num != user_ref_num) {
        return nullptr;
    }
    return &slot;
}

const TxReceiver::PendingSlot* TxReceiver::_lookupPendingSlot(uint32_t user_ref_num) const {
    const PendingSlot& slot = m_pending_slots[_computePendingSlotIndex(user_ref_num)];
    if (!slot.occupied || slot.record.user_ref_num != user_ref_num) {
        return nullptr;
    }
    return &slot;
}

uint64_t TxReceiver::_countPending() const {
    uint64_t pending = 0;
    for (const PendingSlot& slot : m_pending_slots) {
        if (slot.occupied) {
            ++pending;
        }
    }
    return pending;
}

void TxReceiver::_closeRecvFd() {
    if (m_recv_fd >= 0) {
        ::close(m_recv_fd);
        m_recv_fd = -1;
    }
}

void TxReceiver::_markDisconnected() {
    _closeRecvFd();
    m_generation = 0;
    m_frame_buffer.clear();
    ++m_stats.disconnected;
}

void TxReceiver::_run() {
    if (m_worker_cpu >= 0) {
        pinCurrentThreadToCpu(m_worker_cpu);
    }

    while (m_running.load(std::memory_order_acquire)) {
        if (!pollOnce()) {
            std::this_thread::yield();
        }
    }
}
