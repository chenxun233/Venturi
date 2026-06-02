#pragma once
#include <cstdint>
#include "dma_memory_allocator.h"
#include "memory_pool.h"
#ifndef wrap_ring
#define wrap_ring(index, ring_size) (uint16_t) ((index + 1) & (ring_size - 1))
#endif


class RingBuffer{
    public:
        virtual         ~RingBuffer() = default;
        virtual bool    linkMemoryPool( DMAMemoryPool* const mem_pool) = 0;
        bool            createDescriptorRing(DMAMemoryAllocator& allocator, uint8_t* BAR_addr,uint32_t num_desc, uint32_t size_desc, uint8_t ring_index);
    protected:
        bool            _allocDescMemory(DMAMemoryAllocator& allocator, uint32_t num_desc, uint32_t size_desc);
        virtual bool    _bindDescMemIOVA(uint8_t* BAR_addr, uint8_t ring_index) = 0;
        virtual bool    _bindDescMemVirt() = 0;
    protected:
        uint32_t        m_size_desc{0}   ;
        uint32_t        m_num_buf{0};
        uint32_t        m_num_desc{0}    ;
        DMAMemoryPool*  p_mem_pool{nullptr};
        DMABuffer       m_desc_dma;
        pkt_buf**       a_linked_buf_addr{nullptr}; // one-on-one to descriptors
        uint16_t        m_desc_head{0}        ; // used descriptor start index
        uint16_t        m_desc_tail{0}        ; // used descriptor end index


};
