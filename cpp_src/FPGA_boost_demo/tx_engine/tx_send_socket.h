#pragma once

#include "../common/shared_types.h"
#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

class LogPrinter;
class LatencyTracker;

class TxSendSocket {
public:
    TxSendSocket() = default;
    ~TxSendSocket() {
        _closeLocalFd();
    }

    TxSendSocket(const TxSendSocket&) = delete;
    TxSendSocket& operator=(const TxSendSocket&) = delete;

    void attachLogPrinter(LogPrinter* log_printer) {
        m_log_printer = log_printer;
    }
    void attachLatenyTracker(LatencyTracker* latency_tracker) {
        m_latency_tracker = latency_tracker;
    }

    // Only Connected controls are accepted here; disconnect handling goes through retireGeneration().
    void install(const TxTransportControl& control) {
        if (control.kind != TxTransportControlKind::Connected) {
            return;
        }
        if (control.generation == m_generation && control.tx_fd == m_send_fd) {
            return;
        }
        _closeLocalFd();
        m_send_fd = control.tx_fd;
        m_generation = control.generation;
    }

    void retireGeneration(uint64_t generation) {
        if (generation != m_generation) {
            return;
        }
        _closeLocalFd();
    }

    bool hasActiveFd() const {
        return m_send_fd >= 0;
    }

    uint64_t activeGeneration() const {
        return m_generation;
    }

    bool sendPayload(const TxOutboundRecord& record) {
        if (record.payload_length == 0 ||
            record.payload_length > record.payload.size() ||
            m_send_fd < 0) {
            return false;
        }

        std::size_t offset = 0;
        while (offset < static_cast<std::size_t>(record.payload_length)) {
            const ssize_t written = ::send(
                m_send_fd,
                record.payload.data() + static_cast<std::ptrdiff_t>(offset),
                static_cast<std::size_t>(record.payload_length) - offset,
                MSG_NOSIGNAL);
            if (written > 0) {
                offset += static_cast<std::size_t>(written);
                continue;
            }
            if (written < 0 && errno == EINTR) {
                continue;
            }
            _closeLocalFd(false);
            return false;
        }

        if (record.event_tag != 0 && m_latency_tracker != nullptr) {
            try {
                m_latency_tracker->pushRecord(TimeRecord {
                    .que_idx = record.que_idx,
                    .event_tag = record.event_tag,
                    .event_stage = stage::TX_SEND,
                    .time_captured = readMonotonicRawNs(),
                });
            } catch (...) {
                // Latency tracking failure must not affect send success.
            }
        }

        _logOrderSent(record);
        return true;
    }

private:
    void _closeLocalFd(bool clear_generation = true) {
        if (m_send_fd >= 0) {
            ::close(m_send_fd);
            m_send_fd = -1;
        }
        if (clear_generation) {
            m_generation = 0;
        }
    }

    void _logOrderSent(const TxOutboundRecord& record) {
        if (m_log_printer == nullptr || record.user_ref_num == 0) {
            return;
        }
        (void)m_log_printer->pushTxEvent(TxLogRecord {
            .event = TxEventKind::OrderSent,
            .user_ref_num = record.user_ref_num,
            .stock_locate = record.stock_locate,
            .price = record.price,
            .shares = record.shares,
        });
    }

    LogPrinter* m_log_printer {nullptr};
    LatencyTracker* m_latency_tracker {nullptr};
    int m_send_fd {-1};
    uint64_t m_generation {0};
};
