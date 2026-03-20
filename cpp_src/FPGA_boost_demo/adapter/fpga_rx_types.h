#pragma once

#include <cstdint>

struct FPGAEventDesc {
    uint16_t queue_id {0};
    uint16_t stock_locate {0};
    uint64_t frame_latency {0};
    uint64_t frame_start_ts {0};
    uint32_t bid_shares {0};
    uint32_t bid_price {0};
    uint32_t ask_shares {0};
    uint32_t ask_price {0};
};

struct FpgaSyncSnapshot {
    uint16_t queue_id {0};
    uint64_t prod_ptr {0};
    uint64_t dma_timestamp {0};
};
