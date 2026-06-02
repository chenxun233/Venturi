#include "basic_ring_buffer.h"
#include "dma_memory_allocator.h"
#include "log.h"
#include <cstring>
// Allocate descriptor DMA memory from the shared allocator used by the active device.
bool RingBuffer::_allocDescMemory(DMAMemoryAllocator& allocator, uint32_t num_desc, uint32_t size_desc){
    if (p_mem_pool == nullptr) {
        error("No memory pool linked yet");
        return false;
    }
	uint32_t total_size = num_desc * size_desc;
	m_desc_dma = allocator.allocate(total_size);
    if (!m_desc_dma.valid()) {
        error("Failed to allocate descriptor DMA memory");
        return false;
    }
	memset(m_desc_dma.virt(), -1, total_size);
	return true;

}



bool RingBuffer::createDescriptorRing(DMAMemoryAllocator& allocator, uint8_t* BAR_addr, uint32_t num_desc, uint32_t size_desc, uint8_t ring_index){
	m_num_desc = num_desc;
	m_size_desc = size_desc;
	if (!this->_allocDescMemory(allocator, num_desc, size_desc)) {
        return false;
    }
	if (!this->_bindDescMemIOVA(BAR_addr, ring_index)) {
        return false;
    }
	if (!this->_bindDescMemVirt()) {
        return false;
    }
	if (!a_linked_buf_addr) {
		a_linked_buf_addr = new pkt_buf*[m_num_desc]();
	}
	return true;
}
