#pragma once

#include "../decoder/fpga_rx_decoder.h"
#include <cstddef>
#include <cstdint>

class TxEngine {
public:
    explicit TxEngine(uint16_t queue_idx);

    void sendBatch(const FPGAEventDesc* events, std::size_t count);
    uint16_t readQueueIndex() const;

private:
    uint16_t m_queue_idx {0};
};
