#pragma once
#include "../../common/basic_dev.h"
#include <cstdint>
#include <vector>
#include "../../common/memory_pool.h"
#include "ixgbe_ring_buffer.h"

#define PKT_SIZE 60
#define BATCH_SIZE 64 // the number of pkt to be sent per time
#define TX_CLEAN_BATCH 256 // the number of tx descriptors to clean in one batch
#define MOVING_AVERAGE_RANGE 5
#define IRQ_SET_BUF_LEN (sizeof(struct vfio_irq_set) + sizeof(int))
#define MAX_INTERRUPT_VECTORS 32
#define MSIX_IRQ_SET_BUF_LEN (sizeof(struct vfio_irq_set) + sizeof(int) * (MAX_INTERRUPT_VECTORS + 1))
#ifndef wrap_ring
#define wrap_ring(index, ring_size) (uint16_t) ((index + 1) & (ring_size - 1))
#endif

struct __attribute__((__packed__)) MacAddress {
    uint8_t addr[6];
};

struct interrupt_moving_avg {
    uint32_t index;
    uint32_t length;
    uint64_t sum;
    uint64_t measured_rates[MOVING_AVERAGE_RANGE];
};

struct InterruptQueue {
    int vfio_event_fd;
    int vfio_epoll_fd;
    bool interrupt_enabled {true};
    uint64_t last_time_checked;
    uint64_t instr_counter;
    uint64_t rx_pkts;
    uint64_t interval;
    uint32_t timeout_ms {100};
    struct interrupt_moving_avg moving_avg;
};

struct interruptPara {
    uint32_t itr_rate {0x028};
    std::vector<InterruptQueue> interrupt_queues;
    uint8_t interrupt_type {0};
};

struct QueuesPtr {
    void*                   rx;
    void*                   tx;
};
    


class Intel82599Dev : public BasicDev{
    public:
        Intel82599Dev(std::string pci_addr);
        ~Intel82599Dev();
        bool        initHardware();
        bool        initializeInterrupt(const int interrupt_interval, const uint32_t timeout_ms);
        bool        enableDevQueues();
        bool        enableDevInterrupt();
        bool        setRxRingBuffers(uint16_t tx_que_num,uint32_t num_buf, uint32_t buf_size)     override;
        bool        setTxRingBuffers(uint16_t tx_que_num,uint32_t num_buf, uint32_t buf_size)     override;
        bool        sendOnQueue(uint8_t* p_data, size_t size, uint16_t que_idx)                     ;
        void        loopSendTest(uint32_t num_buf);
        void        capturePackets(uint16_t batch_size,int64_t n_packets, std::string file_name);
        void        infoNIC_Tx(uint16_t tail_index);
        void        infoNIC_Rx(uint16_t tail_index);
        bool        setPromisc(bool enable)                             ;
        bool        wait4Link();
    private:
        // _getFD() and _getBARAddr() are now inherited from BasicDev
        bool        _enableDMA()                                                            ;
    private:    
        bool        _dev_disable_IRQ()                                                     ;
        bool        _dev_clear_interrupts()                                                ;
        bool        _dev_rst_hardware()                                                    ;
        bool        _get_mac_address()                                                     ;
        bool        _init_eeprom_n_dma()                                                   ;
        bool        _init_link_nego()                                                      ;
        DevStatus    _readStatus()                                                         ;
        void        _initStatus(DevStatus* stats)                       override      ;
    private:
        bool        _initRxDescRingRegs();
        bool        _initTxDescRingRegs();
        bool        _enableDevRxQueue();
        bool        _enableDevTxQueue();
        void        _enableDevMSIInterrupt(uint16_t que_idx)                              ;
        void        _enableDevMSIxInterrupt(uint16_t que_idx)                             ;
        uint32_t    _get_link_speed()                                                      ;
        bool        _getDevIRQType()                                                       ;
        bool        _setupIRQQueues(const int interrupt_interval, const uint32_t timeout_ms);
        int         _injectEventFdToVFIODev_msi()                                          ;
        int         _injectEventFdToVFIODev_msix(int index)                                ;
        int         _vfio_epoll_ctl(int event_fd)                                          ;
        uint16_t    _calc_ip_checksum  (uint8_t* data, uint32_t len)                       ;
    private:
        uint32_t                        m_num_rx_bufs{0}                                   ;   
        uint32_t                        m_buf_rx_size{0}                                   ;
        uint32_t                        m_num_tx_bufs{0}                                   ;
        uint32_t                        m_buf_tx_size{0}                                   ;
        MacAddress                      m_mac_address{}                                    ;
        interruptPara                   m_interrupt_para                                   ;
        // std::vector<DMAMemoryPool*>        p_mempool                                          ;
        DMAMemoryPool*                    p_tx_mempool{nullptr}                              ;
        std::vector<IXGBE_RxRingBuffer*>  p_rx_ring_buffers                                  ;
        std::vector<IXGBE_TxRingBuffer*>  p_tx_ring_buffers                                  ;

};
