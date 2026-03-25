#pragma once

#include "../common/shared_types.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class RuntimePayloadValidator {
public:
    RuntimePayloadValidator();

    bool loadExpectedPayloads();
    bool validateBatch(uint16_t que_idx, const FPGAEventDesc* events, std::size_t count);
    bool isReadyForMeasurement() const;
    bool hasFailed() const;

private:
    struct ExpectedEvent {
        bool ask_valid {false};
        uint32_t ask_price {0};
        uint32_t ask_shares {0};
        bool bid_valid {false};
        uint32_t bid_price {0};
        uint32_t bid_shares {0};
        uint16_t stock_locate {0};
    };

    struct QueueValidationState {
        std::string symbol_name;
        uint16_t stock_locate {0};
        std::vector<ExpectedEvent> expected_events;
        std::size_t next_expected_idx {0};
        bool validated {false};
    };

    struct OrderState {
        char side {'\0'};
        uint32_t shares {0};
        uint32_t price {0};
    };

    struct SymbolModel {
        std::string symbol_name;
        uint16_t stock_locate {0};
        std::unordered_map<uint64_t, OrderState> orders;
        std::map<uint32_t, uint64_t> bid_book;
        std::map<uint32_t, uint64_t> ask_book;
        ExpectedEvent last_event;
        bool has_last_event {false};
        std::vector<ExpectedEvent> expected_events;
    };

    bool loadFixtureFrame(const std::string& file_name, std::vector<uint8_t>& frame_bytes) const;
    bool parseExpectedEvents(const std::vector<uint8_t>& frame_bytes, QueueValidationState& queue) const;
    bool parseMessage(const std::vector<uint8_t>& frame_bytes,
                      std::size_t msg_offset,
                      uint16_t msg_len,
                      SymbolModel& model) const;
    void applyBookUpdate(SymbolModel& model,
                         uint8_t msg_type,
                         uint64_t order_ref_num,
                         uint64_t new_order_ref_num,
                         char side,
                         uint32_t shares,
                         uint32_t price) const;
    void accumulateLevel(std::map<uint32_t, uint64_t>& side_book,
                         uint32_t price,
                         int64_t delta_shares) const;
    void emitEventIfChanged(SymbolModel& model) const;
    bool compareEvent(uint16_t que_idx, const FPGAEventDesc& actual, const ExpectedEvent& expected) const;
    void setFailureLocked(const std::string& reason);

    mutable std::mutex m_mutex;
    std::vector<QueueValidationState> m_queues;
    bool m_failed {false};
    std::string m_failure_reason;
};
