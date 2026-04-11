#pragma once

#include <cstddef>
#include <cstdint>

class BasicRxDev {
public:
    virtual ~BasicRxDev();

    virtual void _readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const = 0;
    virtual uint64_t _readDropCount(uint16_t que_idx) const = 0;
    virtual void _readProdPtrSnapshot(uint16_t que_idx,
                                    uint64_t& prod_ptr,
                                    uint64_t& fpga_tick,
                                    uint64_t& host_time_ns,
                                    uint64_t& interval,
                                    bool get_snapshot) = 0;
    virtual const uint8_t* _pollDataRaw(uint16_t que_idx, uint64_t cons_ptr) const = 0;
    virtual void _writeConsPtr(uint16_t que_idx, uint64_t cons_ptr) = 0;
    virtual bool isValid() const = 0;
};
