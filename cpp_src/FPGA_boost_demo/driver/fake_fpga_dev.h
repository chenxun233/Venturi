#pragma once

#include "basic_rx_source.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

class FakeFPGADev : public BasicRxSource {
public:
    static constexpr std::size_t kSlotSizeBytes = 32;
    using RawSlot = std::array<uint8_t, kSlotSizeBytes>;

    explicit FakeFPGADev(std::size_t queue_count = 1);
    ~FakeFPGADev() override = default;

    void setRawSlots(uint16_t que_idx, const std::vector<RawSlot>& slots);
    void setProdPtr(uint16_t que_idx, uint64_t prod_ptr);
    void setSyncSnapshot(uint16_t que_idx,
                         uint64_t prod_ptr,
                         uint64_t fpga_tick,
                         uint64_t host_time_ns,
                         uint64_t interval_ns);
    uint64_t lastWrittenConsPtr(uint16_t que_idx) const;

    void _readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const override;
    uint64_t _readDropCount(uint16_t que_idx) const override;
    void _readProdPtrAndTime(uint16_t que_idx,
                            uint64_t& prod_ptr,
                            uint64_t& fpga_tick,
                            uint64_t& host_time_ns,
                            uint64_t& interval,
                            bool get_time) const override;
    const uint8_t* _pollOneRaw(uint16_t que_idx, uint64_t cons_ptr) const override;
    void _writeConsPtr(uint16_t que_idx, uint64_t cons_ptr) override;
    bool isValid() const override { return true; }

private:
    struct QueueState {
        std::vector<RawSlot> slots;
        uint64_t prod_ptr {0};
        uint64_t drop_count {0};
        uint64_t fpga_tick {0};
        uint64_t host_time_ns {0};
        uint64_t interval_ns {0};
        uint64_t last_written_cons_ptr {0};
    };

    std::vector<QueueState> m_queue_states;
};
