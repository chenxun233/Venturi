#include "exchange_transport.h"

#include <cerrno>
#include <stdexcept>

#include <sys/socket.h>
#include <unistd.h>

ExchangeTransport::ExchangeTransport(std::size_t slot_capacity)
    : m_slots(slot_capacity) {}

void ExchangeTransport::attachClientFd(int slot_idx, int fd) {
    if (fd < 0) {
        throw std::runtime_error("invalid socket fd");
    }

    TransportSlot& TransportSlot = _readSlot(slot_idx);
    if (TransportSlot.fd >= 0) {
        ::close(TransportSlot.fd);
    }
    TransportSlot.fd = fd;
}

void ExchangeTransport::closeConnection(int slot_idx) {
    TransportSlot& TransportSlot = _readSlot(slot_idx);
    if (TransportSlot.fd >= 0) {
        ::close(TransportSlot.fd);
        TransportSlot.fd = -1;
    }
}

bool ExchangeTransport::hasSocket(int slot_idx) const {
    return _readSlot(slot_idx).fd >= 0;
}

int ExchangeTransport::readFd(int slot_idx) const {
    const TransportSlot& TransportSlot = _readSlot(slot_idx);
    if (TransportSlot.fd < 0) {
        throw std::runtime_error("runtime TransportSlot has no attached socket");
    }
    return TransportSlot.fd;
}

bool ExchangeTransport::receiveBytes(int slot_idx,
                                     std::vector<uint8_t>& buffer,
                                     std::size_t& bytes_received) const {
    const int fd = readFd(slot_idx);
    std::size_t offset = 0;

    while (offset < buffer.size()) {
        const ssize_t count = ::recv(fd, buffer.data() + static_cast<std::ptrdiff_t>(offset), buffer.size() - offset, 0);
        if (count > 0) {
            offset += static_cast<std::size_t>(count);
            continue;
        }
        if (count == 0) {
            bytes_received = offset;
            return false;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            bytes_received = offset;
            return true;
        }
        bytes_received = offset;
        return false;
    }

    bytes_received = offset;
    return true;
}

bool ExchangeTransport::sendBytes(int slot_idx,
                                  const uint8_t* bytes,
                                  std::size_t size,
                                  std::size_t& bytes_sent) const {
    const int fd = readFd(slot_idx);
    std::size_t offset = 0;

    while (offset < size) {
        const ssize_t count = ::send(fd,
                                     bytes + static_cast<std::ptrdiff_t>(offset),
                                     size - offset,
                                     MSG_NOSIGNAL);
        if (count > 0) {
            offset += static_cast<std::size_t>(count);
            continue;
        }
        if (count == 0) {
            bytes_sent = offset;
            return false;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            bytes_sent = offset;
            return true;
        }
        bytes_sent = offset;
        return false;
    }

    bytes_sent = offset;
    return true;
}

void ExchangeTransport::reset() {
    for (TransportSlot& TransportSlot : m_slots) {
        if (TransportSlot.fd >= 0) {
            ::close(TransportSlot.fd);
            TransportSlot.fd = -1;
        }
    }
}

ExchangeTransport::TransportSlot& ExchangeTransport::_readSlot(int slot_idx) {
    if (slot_idx < 0 || static_cast<std::size_t>(slot_idx) >= m_slots.size()) {
        throw std::runtime_error("invalid runtime TransportSlot index");
    }
    return m_slots[static_cast<std::size_t>(slot_idx)];
}

const ExchangeTransport::TransportSlot& ExchangeTransport::_readSlot(int slot_idx) const {
    if (slot_idx < 0 || static_cast<std::size_t>(slot_idx) >= m_slots.size()) {
        throw std::runtime_error("invalid runtime TransportSlot index");
    }
    return m_slots[static_cast<std::size_t>(slot_idx)];
}
