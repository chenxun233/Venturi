#include "tx_client.h"

#include "../latency/log_printer.h"

TxClient::TxClient(GatewayClientConfig connection_config,
                   TxSenderConfig sender_config,
                   std::size_t receiver_pending_capacity)
    : m_connector(std::move(connection_config)),
      m_sender(std::move(sender_config)),
      m_receiver(receiver_pending_capacity) {
    m_connector.attachSender(&m_sender);
    m_connector.attachReceiver(&m_receiver);
    m_sender.attachConnection(&m_connector);
    m_sender.attachReceiver(&m_receiver);
}

void TxClient::attachQueueIdx(uint16_t queue_idx) {
    m_connector.attachQueueIdx(queue_idx);
    m_receiver.attachQueueIdx(queue_idx);
}

void TxClient::attachLogPrinter(LogPrinter* log_printer) {
    m_connector.attachLogPrinter(log_printer);
}

void TxClient::attachLatencyTracker(LatencyTracker* latency_tracker) {
    m_sender.attachLatencyTracker(latency_tracker);
}

bool TxClient::acceptExecution(const OrderExecution& execution) noexcept {
    return m_sender.acceptExecution(execution);
}

bool TxClient::runOnce() {
    const bool connector_work = m_connector.pollConnect();
    const bool sender_work = m_sender.runOnce();
    const bool receiver_work = m_receiver.pollOnce(m_sender.readSendFd());
    return connector_work || sender_work || receiver_work;
}

TxReceiverStats TxClient::readReceiverStats() const {
    return m_receiver.readStats();
}

void TxClient::printReceiverSummary() const {
    m_receiver.printSummary();
}
