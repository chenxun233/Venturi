#pragma once

#include "../common/basic_dev.h"
#include "../common/dma_memory_allocator.h"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class FPGADev : public BasicDev {
public:
    FPGADev(std::string pci_addr);
    ~FPGADev() override;

    bool initHardware() override;
    bool setRxRingBuffers(uint16_t num_rx_queues, uint32_t num_buf, uint32_t buf_size) override;
    bool setTxRingBuffers(uint16_t num_tx_queues, uint32_t num_buf, uint32_t buf_size) override;

    void write_reg64(uint32_t offset, uint64_t value);
    uint64_t read_reg64(uint32_t offset);
    void write_reg32(uint32_t offset, uint32_t value);
    uint32_t read_reg32(uint32_t offset);

    bool test_register();
    bool trigger_interrupt();
    bool test_dma_write();
    bool test_dma_roundtrip();

private:

    static constexpr uint32_t REG_RX_IOVA_OFFSET = 0x00;
    static constexpr uint16_t RX_QUEUE_COUNT = 2;
    static constexpr uint32_t RX_RECORD_BYTES = 32;
    static constexpr uint32_t REG_RESET = 0x00;
    static constexpr uint32_t REG_ID = 0x04;
    static constexpr uint32_t REG_STATUS = 0x0C;
    static constexpr uint32_t REG_RX_QUE_BASE0 = 0x40;
    static constexpr uint32_t REG_RX_QUE_STRIDE = 0x40;
    static constexpr uint32_t REG_RX_QUE_SLOT_NUM_OFFSET = 0x08;
    static constexpr uint32_t REG_RX_QUE_ENABLE_OFFSET = 0x10;
    static constexpr uint32_t REG_RX_QUE_CONS_PTR_OFFSET = 0x18;
    static constexpr uint32_t REG_RX_QUE_PROD_OFFSET = 0x20;
    static constexpr uint32_t REG_RX_QUE_DROP_OFFSET = 0x28;
    static constexpr uint32_t REG_RX_QUE_STAT_OFFSET = 0x30;
    static constexpr uint64_t RX_DMA_CFG_ID = 0x4d5f52585f434647ULL;

    struct ExpectedEvent {
        bool ask_valid {false};
        uint32_t ask_price {0};
        uint32_t ask_shares {0};
        bool bid_valid {false};
        uint32_t bid_price {0};
        uint32_t bid_shares {0};
        uint16_t stock_locate {0};
    };

    struct DecodedEvent {
        bool ask_valid {false};
        uint32_t ask_price {0};
        uint32_t ask_shares {0};
        bool bid_valid {false};
        uint32_t bid_price {0};
        uint32_t bid_shares {0};
        uint64_t event_timestamp {0};
        uint16_t stock_locate {0};
    };

    struct QueueRuntime {
        std::string symbol_name;
        uint16_t stock_locate {0};
        uint32_t slot_num {0};
        uint32_t slot_size_bytes {RX_RECORD_BYTES};
        uint64_t host_cons_ptr {0};
        DMAMemoryPair dma_memory {nullptr, 0, 0};
        std::vector<ExpectedEvent> expected_events;
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

    bool _enableDMA() override;
    void _initStatus(DevStatus* stats) override;

    uint32_t _queueRegOffset(uint16_t que_idx, uint32_t reg_offset) const;
    bool _loadExpectedPayloads();
    bool _loadFixtureFrame(const std::string& file_name, std::vector<uint8_t>& frame_bytes);
    bool _parseExpectedEvents(const std::vector<uint8_t>& frame_bytes, QueueRuntime& queue);
    bool _parseMessage(const std::vector<uint8_t>& frame_bytes, std::size_t msg_offset, uint16_t msg_len, SymbolModel& model);
    void _applyBookUpdate(SymbolModel& model, uint8_t msg_type, uint64_t order_ref_num, uint64_t new_order_ref_num, char side, uint32_t shares, uint32_t price);
    void _accumulateLevel(std::map<uint32_t, uint64_t>& side_book, uint32_t price, int64_t delta_shares);
    void _emitEventIfChanged(SymbolModel& model);
    bool _runReplayEnvironmentChecks();
    bool _pollQueueAndValidate(uint16_t que_idx);
    DecodedEvent _decodeRecord(const uint8_t* slot_bytes) const;
    bool _compareEvent(uint16_t que_idx, uint64_t event_idx, const DecodedEvent& actual, uint64_t& last_timestamp);
    const QueueRuntime* _queueForIndex(uint16_t que_idx) const;

private:
    std::array<QueueRuntime, RX_QUEUE_COUNT> m_rx_queues {};
    bool m_hw_ready {false};
};
