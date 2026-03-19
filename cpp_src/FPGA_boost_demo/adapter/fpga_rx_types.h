#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

struct FpgaRawRxRecord {
    static constexpr std::size_t kRecordBytes = 32;
    uint16_t queue_id {0};
    uint64_t sequence {0};
    std::array<uint8_t, kRecordBytes> bytes {};
};

struct FPGAEventDesc {
    uint16_t queue_id {0};
    uint16_t stock_locate {0};
    uint64_t frame_latency {0};
    uint64_t event_latency {0};
    uint32_t bid_shares {0};
    uint32_t bid_price {0};
    uint32_t ask_shares {0};
    uint32_t ask_price {0};
};
