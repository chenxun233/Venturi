#include "tx_receiver.h"

#include "../common/shared_types.h"
#include "tx_connection.h"
#include "tx_sender.h"

TxReceiver::TxReceiver(TxConnection& connection, TxSender& sender)
    : m_connection(connection),
      m_sender(sender) {}

void TxReceiver::attachLogPrinter(LogPrinter* log_printer) {
    // Receiver is the inbound orchestrator in the split runtime. Forward the attachment so a user
    // attaching a logger to the receiver gets a logged TX stack by default.
    m_connection.attachLogPrinter(log_printer);
    m_sender.attachLogPrinter(log_printer);
}

bool TxReceiver::_tryEnqueueRecord(const TxSenderInboundRecord& record) {
    switch (record.kind) {
        case TxSenderInboundKind::Frame:
            return m_sender.acceptInboundFrame(record.frame);
        case TxSenderInboundKind::TransportEvent:
            return m_sender.acceptTransportControl(record.transport_event);
    }
    return false;
}

bool TxReceiver::_retryRetainedRecord() {
    if (!m_has_retained_record) {
        return true;
    }
    if (!_tryEnqueueRecord(m_retained_record)) {
        return false;
    }

    m_has_retained_record = false;
    return true;
}

bool TxReceiver::pollOnce() {
    const bool had_retained_record = m_has_retained_record;
    if (!_retryRetainedRecord()) {
        // Backpressure remains active; caller should keep polling instead of sleeping.
        return true;
    }

    bool did_work = had_retained_record;

    did_work = m_connection.pollConnectStep() || did_work;

    TxTransportControl transport_control {};
    if (m_connection.takeTransportControl(transport_control)) {
        if (!m_sender.acceptTransportControl(transport_control)) {
            m_retained_record = TxSenderInboundRecord {
                .kind = TxSenderInboundKind::TransportEvent,
                .transport_event = transport_control,
            };
            m_has_retained_record = true;
            return true;
        }
        did_work = true;
    }

    TxInboundFrame frame {};
    if (m_connection.readInboundFrame(frame)) {
        const TxSenderInboundRecord inbound_record {
            .kind = TxSenderInboundKind::Frame,
            .frame = frame,
        };
        if (!_tryEnqueueRecord(inbound_record)) {
            m_retained_record = inbound_record;
            m_has_retained_record = true;
            return true;
        }

        did_work = true;
    }

    if (m_connection.takeTransportControl(transport_control)) {
        if (!m_sender.acceptTransportControl(transport_control)) {
            m_retained_record = TxSenderInboundRecord {
                .kind = TxSenderInboundKind::TransportEvent,
                .transport_event = transport_control,
            };
            m_has_retained_record = true;
            return true;
        }
        did_work = true;
    }

    return did_work;
}
