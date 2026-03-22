#pragma once
#include "../decoder/fpga_rx_decoder.h"

class BasicStrategy {
public:
    BasicStrategy() = default;
    virtual ~BasicStrategy() = default;
    virtual void onEvents(const FPGAEventDesc* event, std::size_t count) = 0;
};