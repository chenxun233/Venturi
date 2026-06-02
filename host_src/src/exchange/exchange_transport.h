#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class ExchangeTransport {
public:
    explicit ExchangeTransport(std::size_t slot_capacity);

    void attachClientFd(int slot_idx, int fd);
    void closeConnection(int slot_idx);
    bool hasSocket(int slot_idx) const;
    int readFd(int slot_idx) const;
    bool receiveBytes(int slot_idx, std::vector<uint8_t>& buffer, std::size_t& bytes_received) const;
    bool sendBytes(int slot_idx, const uint8_t* bytes, std::size_t size, std::size_t& bytes_sent) const;
    void reset();

private:
    struct TransportSlot {
        int fd {-1};
    };

    TransportSlot& _readSlot(int slot_idx);
    const TransportSlot& _readSlot(int slot_idx) const;

private:
    std::vector<TransportSlot> m_slots {};
};
