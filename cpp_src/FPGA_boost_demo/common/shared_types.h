#pragma once

#include <atomic>
#include <cstdint>
#include <ctime>


#define MAX_POLL_RECORDS 32

typedef struct  {
    uint64_t fpga_tick      {0};
    uint64_t host_time_ns   {0};
    uint64_t interval_ns    {0};
}FpgaSyncSnapshot;


typedef struct{
    std::atomic<bool> request {false};
} CapSignal;

typedef struct  {
    bool has_para {false};
    uint64_t a_q32 {1ULL << 32};
    int64_t b_ns {0};
}   RegressionPara;

struct FPGAEventDesc {
    uint8_t  is_first_event {0};
    uint32_t ask_price      {0};
    uint32_t ask_shares     {0};
    uint32_t bid_price      {0};
    uint32_t bid_shares     {0};
    uint64_t frame_start_tk {0};
    uint64_t event_tk       {0};
    uint16_t stock_locate   {0};
    
};

enum stage {
    FRAME_START,
    DMA_EMIT,
    DECODE,
    ANALYSIS
};

struct TimeRecord {
    uint16_t    que_idx {0};
    uint64_t    event_ts {0};
    stage       event_stage {stage::DECODE};
    uint64_t    time_captured {0};
};

struct StageLatency{

    uint16_t    que_idx {0};
    uint64_t    event_ts {0};
    stage       prev_stage {stage::DECODE};
    stage       curr_stage {stage::DECODE};
    uint64_t    latency {0};
};

struct LatencyStats {
    uint16_t    que_idx {0};
    stage       prev_stage {stage::DECODE};
    stage       curr_stage {stage::DECODE};
    uint64_t    sample_count {0};
    uint64_t    drop_count {0};
    uint64_t    max_ns {0};
    uint64_t    min_ns {0};
};

struct LatencyLogRecord {
    uint16_t    que_idx {0};
    uint64_t    event_ts {0};
    uint64_t    frame_start_to_dma_emit_ns {0};
    int64_t     dma_emit_to_decode_ns {0};
};

enum class OrderIntentAction : uint8_t {
    None,
    Buy,
    Sell
};

struct OrderIntentPayload {
    OrderIntentAction action {OrderIntentAction::None};
    uint32_t price {0};
    uint32_t shares {0};
};

struct OrderIntent {
    uint16_t stock_locate {0};
    OrderIntentPayload intent {};
};

struct ExecutionLogRecord {
    uint16_t stock_locate {0};
    OrderIntentPayload intent {};
};

enum class AsyncLogKind : uint8_t {
    Latency,
    Snapshot,
    Execution
};

struct AsyncLogRecord {
    AsyncLogKind kind {AsyncLogKind::Latency};
    LatencyLogRecord latency {};
    FpgaSyncSnapshot snapshot {};
    ExecutionLogRecord execution {};
};


struct FirstEventMask {
    std::size_t count {0};
    uint32_t first_event_mask {0}; // bit i => out[i] is first-in-frame
};
