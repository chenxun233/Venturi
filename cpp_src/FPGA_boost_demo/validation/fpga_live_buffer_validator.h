#pragma once

#include "../decoder/fpga_rx_decoder.h"

#include <cstddef>
#include <map>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

class FpgaQueueLiveValidator {
public:
    FpgaQueueLiveValidator(uint16_t que_idx,
                           std::string symbol_name,
                           uint16_t stock_locate,
                           std::string fixture_path);

    bool loadExpectedEvents();
    bool validateBatch(std::span<const FPGAEventDesc> batch);
    bool isComplete() const;

private:
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
        FPGAEventDesc last_event {};
        bool has_last_event {false};
        std::vector<FPGAEventDesc> expected_events;
    };

    bool _loadFixtureFrame(std::vector<uint8_t>& frame_bytes) const;
    bool _parseExpectedEvents(const std::vector<uint8_t>& frame_bytes);
    bool _parseMessage(const std::vector<uint8_t>& frame_bytes,
                       std::size_t msg_offset,
                       uint16_t msg_len,
                       SymbolModel& model);
    bool _compareRecord(std::size_t event_idx, const FPGAEventDesc& actual) const;
    void _applyBookUpdate(SymbolModel& model,
                          uint8_t msg_type,
                          uint64_t order_ref_num,
                          uint64_t new_order_ref_num,
                          char side,
                          uint32_t shares,
                          uint32_t price);
    void _accumulateLevel(std::map<uint32_t, uint64_t>& side_book,
                          uint32_t price,
                          int64_t delta_shares);
    void _emitEventIfChanged(SymbolModel& model);
    void _printBatch(std::span<const FPGAEventDesc> batch) const;

    uint16_t m_que_idx;
    std::string m_symbol_name;
    uint16_t m_stock_locate;
    std::string m_fixture_path;
    std::vector<FPGAEventDesc> m_expected_events;
    std::size_t m_next_event_idx {0};
    std::size_t m_batch_limit {0};
};
