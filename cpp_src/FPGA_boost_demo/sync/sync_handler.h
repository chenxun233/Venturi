#pragma once
#include "../common/shared_types.h"
#include "regression.h"

#include <cstddef>
#include <cstdint>

class SyncHandler {
public:
    explicit SyncHandler(uint64_t trigger_period = 64);
    void run(CapSignal& cap_signal);

    template <typename DeviceT>
    bool initSync(DeviceT& device,
                  Regression& regression,
                  FpgaSyncSnapshot& snapshot,
                  std::size_t max_attempts,
                  uint64_t accepted_interval_ns) {
        for (std::size_t attempt = 0; attempt < max_attempts; ++attempt) {
            if (!device.readSyncTimestamp(snapshot)) {
                continue;
            }
            if (snapshot.interval_ns != 0 && snapshot.interval_ns <= accepted_interval_ns) {
                regression.updateSnapshot(snapshot);
            }
            if (regression.isFrozen()) {
                return true;
            }
        }
        return false;
    }

private:
    uint64_t m_trigger_period {0};
    uint64_t m_trigger_countdown {0};
};
