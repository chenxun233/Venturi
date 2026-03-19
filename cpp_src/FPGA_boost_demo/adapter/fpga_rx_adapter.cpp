#include "fpga_rx_adapter.h"

FpgaRxAdapter::FpgaRxAdapter(FPGADev& device)
    : m_device(device) {
}

uint16_t FpgaRxAdapter::queueCount() const {
    return m_device.rxQueueCount();
}

bool FpgaRxAdapter::pollOne(uint16_t queue_id, FPGAEventDesc& out) {
    return m_device.pollDecodedRecord(queue_id, out);
}

std::size_t FpgaRxAdapter::pollBatch(uint16_t queue_id, FPGAEventDesc* out, std::size_t max_count) {
    return m_device.pollDecodedRecords(queue_id, out, max_count);
}
