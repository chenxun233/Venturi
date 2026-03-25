#pragma once
#include "../common/shared_types.h"
#include <cstdint>

class SyncHandler {
public:
    explicit  SyncHandler(uint64_t trigger_period = 64);
    void      run(CapSignal& cap_signal);

private:
    uint64_t  m_trigger_period {0};
    uint64_t  m_trigger_countdown {0};
};
