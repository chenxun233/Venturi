#pragma once

#include "fpga_dev.h"

class FpgaRxAdapter {
public:
    explicit FpgaRxAdapter(FPGADev& device);
    uint16_t queueCount() const;
    bool pollOne(uint16_t queue_id, FPGAEventDesc& out);
    std::size_t pollBatch(uint16_t queue_id, FPGAEventDesc* out, std::size_t max_count);

private:
    FPGADev& m_device;
};
