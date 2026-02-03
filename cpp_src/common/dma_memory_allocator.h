#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

struct DMAMemoryPair {
    // start of the virtual address
    void*   virt;
    // start of the physical/IO virtual address
    uint64_t iova;
    size_t  size;
};

class DMAMemoryAllocator {
    
    public:
        static DMAMemoryAllocator& getInstance               ()
        {
            static DMAMemoryAllocator instance; 
            return instance;
        }
                                    ~DMAMemoryAllocator         ();
    
        /// Allocates huge-page-backed DMA memory and maps it into the VFIO IOMMU.
        /// Use \p virt for CPU access; use \p iova as the device address (e.g. RQ/CC buffers).
        /// \param size Requested (total) size in bytes (rounded up to huge-page alignment).
        /// \param container_fd VFIO container fd for VFIO_IOMMU_MAP_DMA.
        /// \return DMAMemoryPair with .virt, .iova, and .size.
        DMAMemoryPair               allocDMAMemory              (size_t size, int container_fd);

    private:                    
                                    DMAMemoryAllocator          ()                                                      ;
        uint64_t                    _alignUpU64                 (uint64_t value, uint64_t alignment)                    ;
        void*                       _allocDMAVirtualAddr        (size_t ring_size)                                      ;
        bool                        _bindIOVAWithVirtAddr       (void* virt_addr, uint64_t iova, size_t ring_size, int container_fd)   ;
        bool                        _unmapVirtualAddr           ()                                                      ;
        bool                        _unmapIOVirtualAddr         ()                                                      ;
    private:                                                        
        uint64_t                    m_page_size                 {2*1024*1024};// 2MB huge page size                                 ;        ;
        uint64_t                    m_next_iova                 {0x10000}                                        ;
        std::vector<DMAMemoryPair>  m_allocated_memories                                                                ;

};