#pragma once

#include "tx_connector.h"
#include "tx_receiver.h"
#include "tx_sender.h"

#include <cstddef>
#include <cstdint>

class LatencyTracker;

class TxClient {
public:
    TxClient(GatewayClientConfig connection_config = {},
             TxSenderConfig sender_config = {},
             std::size_t receiver_pending_capacity = 1024);
    ~TxClient() = default;
    TxClient(const TxClient&) = delete;
    TxClient& operator=(const TxClient&) = delete;
    TxClient(TxClient&&) = delete;
    TxClient& operator=(TxClient&&) = delete;

    void attachQueueIdx(uint16_t queue_idx);
    void attachLogPrinter(LogPrinter* log_printer);
    void attachLatencyTracker(LatencyTracker* latency_tracker);
    bool acceptExecution(const OrderExecution& execution) noexcept;
    bool runOnce();
    TxReceiverStats readReceiverStats() const;
    void printReceiverSummary() const;

private:
    TxConnector m_connector;
    TxSender m_sender;
    TxReceiver m_receiver;
};
