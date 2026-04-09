#include "fake_fpga_dev.h"

FakeFPGADev::FakeFPGADev(std::size_t queue_count)
    : m_queue_states(queue_count) {
}

void FakeFPGADev::setRawSlots(uint16_t que_idx, const std::vector<RawSlot>& slots) {
    m_queue_states[que_idx].slots = slots;
}

void FakeFPGADev::setProdPtr(uint16_t que_idx, uint64_t prod_ptr) {
    m_queue_states[que_idx].prod_ptr = prod_ptr;
}

void FakeFPGADev::setSyncSnapshot(uint16_t que_idx,
                                  uint64_t prod_ptr,
                                  uint64_t fpga_tick,
                                  uint64_t host_time_ns,
                                  uint64_t interval_ns) {
    QueueState& queue_state = m_queue_states[que_idx];
    queue_state.prod_ptr = prod_ptr;
    queue_state.fpga_tick = fpga_tick;
    queue_state.host_time_ns = host_time_ns;
    queue_state.interval_ns = interval_ns;
}

bool FakeFPGADev::readSyncTimestamp(FpgaSyncSnapshot& snapshot) const {
    if (m_queue_states.empty()) {
        return false;
    }

    const QueueState& queue_state = m_queue_states.front();
    snapshot.fpga_tick = queue_state.fpga_tick;
    snapshot.host_time_ns = queue_state.host_time_ns;
    snapshot.interval_ns = queue_state.interval_ns;
    return true;
}

uint64_t FakeFPGADev::lastWrittenConsPtr(uint16_t que_idx) const {
    return m_queue_states[que_idx].last_written_cons_ptr;
}

void FakeFPGADev::_readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const {
    prod_ptr = m_queue_states[que_idx].prod_ptr;
}

uint64_t FakeFPGADev::_readDropCount(uint16_t que_idx) const {
    return m_queue_states[que_idx].drop_count;
}

void FakeFPGADev::_readProdPtrSnapshot(uint16_t que_idx,
                                     uint64_t& prod_ptr,
                                     uint64_t& fpga_tick,
                                     uint64_t& host_time_ns,
                                     uint64_t& interval,
                                     bool get_time) {
    const QueueState& queue_state = m_queue_states[que_idx];
    prod_ptr = queue_state.prod_ptr;
    if (get_time) {
        fpga_tick = queue_state.fpga_tick;
        host_time_ns = queue_state.host_time_ns;
        interval = queue_state.interval_ns;
    }
}

const uint8_t* FakeFPGADev::_pollOneRaw(uint16_t que_idx, uint64_t cons_ptr) const {
    const QueueState& queue_state = m_queue_states[que_idx];
    const std::size_t slot_count = queue_state.slots.size();
    if (slot_count == 0) {
        return nullptr;
    }
    const std::size_t slot_idx = static_cast<std::size_t>(cons_ptr % slot_count);
    return queue_state.slots[slot_idx].data();
}

void FakeFPGADev::_writeConsPtr(uint16_t que_idx, uint64_t cons_ptr) {
    m_queue_states[que_idx].last_written_cons_ptr = cons_ptr;
}
