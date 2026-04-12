#pragma once

#include "../common/shared_types.h"

class LogPrinter;
class TxConnection;
class TxSender;

class TxReceiver {
public:
    TxReceiver(TxConnection& connection, TxSender& sender);

    void attachLogPrinter(LogPrinter* log_printer);
    bool pollOnce();

private:
    bool _tryEnqueueRecord(const TxSenderInboundRecord& record);
    bool _retryRetainedRecord();

    TxConnection& m_connection;
    TxSender& m_sender;
    bool m_has_retained_record {false};
    TxSenderInboundRecord m_retained_record {};
};
