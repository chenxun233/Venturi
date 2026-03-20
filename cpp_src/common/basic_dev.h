// this file contains an abstract class named BasicDev which defines the basic interfaces for a network device driver
#ifndef BASIC_DEV_H
#define BASIC_DEV_H
#include <cstdint>
#include <memory>
#include <string>
#include <linux/vfio.h>
#include "dma_memory_allocator.h"

struct DevStatus {
    uint64_t    rx_pkts;
    uint64_t    tx_pkts;
    uint64_t    rx_bytes;
    uint64_t    tx_bytes;
};

struct basic_para_type{
	std::string             pci_addr; //the pci address you can find in lspci
	uint16_t                rx_que_num; // the number of rx queues
	uint16_t                tx_que_num;
    uint8_t*                bar0_addr; // BAR0 address
};

struct VfioFd{
    int        container_fd;
    int        group_id;
    int        group_fd;
    int        device_fd;
};

class BasicDev{
    public:
        BasicDev(std::string pci_addr )           ;
        virtual             ~BasicDev()   = default;
        virtual bool        initHardware()= 0 ;
        virtual bool        setRxRingBuffers(uint16_t rx_que_num,uint32_t num_buf, uint32_t buf_size) = 0 ;
        virtual bool        setTxRingBuffers(uint16_t tx_que_num,uint32_t num_buf, uint32_t buf_size) = 0 ;



    protected:
        // Common VFIO setup functions (shared by all PCIe drivers)
        bool                _getFD()                                        ;
        bool                _getBARAddr ()                                  ;
        bool                _initDMAMemoryAllocator()                       ;
    protected:
        // VFIO helper functions (hardware-agnostic)
        bool                _getGroupID()                                   ;
        bool                _getContainerFD()                               ;
        bool                _getGroupFD()                                   ;
        bool                _addGroup2Container()                           ;
        bool                _getDeviceFD()                                  ;
        void                _writeReg64(uint32_t offset, uint64_t value) ;
        uint64_t            _readReg64(uint32_t offset) const            ;
        void                _writeReg32(uint32_t offset, uint32_t value) ;
        uint32_t            _readReg32(uint32_t offset) const            ;
        bool                _readReg128(uint32_t offset,
                                           uint64_t& low_qword,
                                           uint64_t& high_qword) const      ;
        uint64_t            _monotonic_time()                               ;
        DMAMemoryAllocator& _getDMAAllocator()                              ;
        const DMAMemoryAllocator& _getDMAAllocator() const                  ;
        virtual void        _initStatus(DevStatus* stats)= 0 ;
        void                _print_stats_diff(DevStatus* stats_new, DevStatus* stats_old, uint64_t nanos);
    protected:
        basic_para_type     m_basic_para                                    ;
        DevStatus           m_dev_stats{0,0,0,0}                            ;
        VfioFd              m_fds{-1,-1,-1,-1}                              ;  
        std::unique_ptr<DMAMemoryAllocator> m_dma_allocator                 ;
};
#endif // BASIC_DEV_H
