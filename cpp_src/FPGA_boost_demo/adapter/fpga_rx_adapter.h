#pragma once

#include "fpga_rx_types.h"
#include "../driver/fpga_dev.h"
#include <vector>

// Translation-only adapter above FPGADev. Callers own any persistent buffering or analysis state.
class FPGARxDataAdaptor {
public:
    explicit    FPGARxDataAdaptor(FPGADev& device);
    uint16_t    queueCount() const;
    bool        pollOne(uint16_t queue_id, FPGAEventDesc& out);
    std::size_t pollBatch(uint16_t queue_id, FPGAEventDesc* out, std::size_t max_count);
    bool        readSyncSnapshot(uint16_t queue_id, FpgaSyncSnapshot& out);

private:

    struct QueueState {
        bool validated {false};
        uint64_t cons_ptr {0};
        FPGADev::RxQueueContext queue_ctx {};
    };
    // Validates the queue once and prepares the queue-local state the hot path needs.
    bool _prepareQueue(uint16_t queue_id);
    static FPGAEventDesc    _decodeRawRecord(const FPGADev::RawRxRecordView& record);
    static FpgaSyncSnapshot _decodeRawSyncSnapshot(const FPGADev::RawSyncSnapshot& snapshot);

    FPGADev& m_device;
    std::vector<QueueState> m_queue_states;
};
