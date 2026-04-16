#pragma once

#include <array>
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
    FRAME_START = 0,
    DMA_EMIT = 1,
    DECODE = 2,
    STRATEGY_START = 3,
    EXECUTOR = 4,
    EXECUTION_TAKEN = 5,
    TX_EXECUTION_ACCEPTED = 6,
    TX_EXECUTION_DEQUEUE = 7,
    TX_ORDER_FRAME_BUILT = 8,
    TX_PENDING_RECORDED = 9,
    TX_ENQUEUE = 10,
    TX_SEND_ENTER = 11,
    TX_SEND_SYSCALL_ENTER = 12,
    TX_SEND = 13,
    BATCH_START = 14,
    BATCH_END = 15
};

struct TimeRecord {
    uint16_t    que_idx {0};
    uint64_t    event_tag {0};
    stage       event_stage {stage::DECODE};
    uint64_t    time_captured {0};
    uint32_t    sender_backlog_depth {0};
    uint32_t    tx_send_call_count {0};
    uint32_t    tx_send_bytes_total {0};
    uint32_t    tx_send_eintr_retry_count {0};
    uint32_t    tx_send_had_partial_write {0};
};

struct StageLatency{

    uint16_t    que_idx {0};
    uint64_t    event_tag {0};
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
    uint64_t    event_tag {0};
    uint64_t    frame_start_to_dma_emit_ns {0};
    int64_t     batch_duration_ns {0};
    int64_t     batch_end_to_strategy_start_ns {0};
    int64_t     strategy_start_to_tx_execution_accepted_ns {0};
    int64_t     tx_execution_accepted_to_tx_enqueue_ns {0};
    int64_t     tx_enqueue_to_tx_send_enter_ns {0};
    int64_t     tx_send_enter_to_tx_send_syscall_enter_ns {0};
    int64_t     tx_send_syscall_enter_to_tx_send_ns {0};
    uint32_t    tx_enqueue_backlog_depth {0};
    uint32_t    tx_send_enter_backlog_depth {0};
    uint32_t    tx_send_call_count {0};
    uint32_t    tx_send_bytes_total {0};
    uint32_t    tx_send_eintr_retry_count {0};
    uint32_t    tx_send_had_partial_write {0};
};

struct RegressionStatusLogRecord {
    bool has_para {false};
    double a_ns_per_tick {0.0};
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
    uint16_t que_idx {0};
    uint64_t event_tag {0};
    OrderIntentPayload intent {};
};

struct OrderExecution {
    uint16_t stock_locate {0};
    uint16_t que_idx {0};
    uint64_t event_tag {0};
    OrderIntentPayload order {};
};

struct ExecutionLogRecord {
    uint16_t stock_locate {0};
    OrderIntentPayload intent {};
};

enum class TxEventKind : uint8_t {
    ConnectionEstablished,
    ConnectionIssue,
    ConnectionLost,
    OrderSent,
    OrderAccepted,
    OrderRejected,
    OrderFilled,
    OrderDropped
};

struct TxOutboundRecord {
    uint32_t                user_ref_num    {0};
    uint16_t                stock_locate    {0};
    uint16_t                que_idx         {0};
    uint64_t                event_tag       {0};
    uint32_t                price           {0};
    uint32_t                shares          {0};
    uint8_t                 payload_length  {0};
    std::array<uint8_t, 64> payload         {};

};

struct TxLogRecord {
    uint16_t queue_idx     {0};
    TxEventKind event       {TxEventKind::ConnectionEstablished};
    uint32_t user_ref_num   {0};
    uint16_t stock_locate   {0};
    uint32_t price          {0};
    uint32_t shares         {0};
    uint16_t reason         {0};
    uint64_t match_number   {0};
};

struct TxInboundFrame {
    std::array<uint8_t, 67> payload {};
    uint8_t payload_length {0};
};

enum class TxConnectionKind : uint8_t {
    Connected,
    Disconnected,
};

enum class TxTransportEvent : uint8_t {
    Connected,
    Disconnected,
};

struct TxConnectionInfo {
    TxConnectionKind kind {TxConnectionKind::Disconnected};
    uint64_t generation {0};
    int fd {-1};
};

struct TxDisconnectNotice {
    uint64_t generation {0};
    uint16_t reason {0};
};

enum class TxSenderInboundKind : uint8_t {
    Frame,
    TransportEvent,
};

struct TxSenderInboundRecord {
    TxSenderInboundKind kind {TxSenderInboundKind::Frame};
    TxInboundFrame frame {};
    TxConnectionInfo transport_event {};
};

enum class AsyncLogKind : uint8_t {
    Latency,
    Snapshot,
    RegressionStatus,
    Execution,
    Tx
};

struct AsyncLogRecord {
    AsyncLogKind kind {AsyncLogKind::Latency};
    LatencyLogRecord latency {};
    FpgaSyncSnapshot snapshot {};
    RegressionStatusLogRecord regression_status {};
    ExecutionLogRecord execution {};
    TxLogRecord tx {};
};
