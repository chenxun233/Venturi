#pragma once
#include <cstddef>
#include <cstdint>

class DMABuffer {
public:
    DMABuffer() = default;
    DMABuffer(DMABuffer&& other) noexcept;
    DMABuffer& operator=(DMABuffer&& other) noexcept;
    ~DMABuffer();

    DMABuffer(const DMABuffer&) = delete;
    DMABuffer& operator=(const DMABuffer&) = delete;

    void* virt() const { return m_virt; }
    uint64_t iova() const { return m_iova; }
    size_t size() const { return m_size; }
    bool valid() const { return m_virt != nullptr && m_iova != 0 && m_size != 0; }
    explicit operator bool() const { return valid(); }

private:
    friend class DMAMemoryAllocator;

    DMABuffer(int container_fd, void* virt_addr, uint64_t iova_addr, size_t mapped_size);
    void reset() noexcept;
    int m_container_fd {-1};
    void* m_virt {nullptr};
    uint64_t m_iova {0};
    size_t m_size {0};
};

class DMAMemoryAllocator {
public:
    explicit DMAMemoryAllocator(int container_fd, uint64_t page_size = 2ULL * 1024ULL * 1024ULL);
    ~DMAMemoryAllocator() = default;

    DMAMemoryAllocator(const DMAMemoryAllocator&) = delete;
    DMAMemoryAllocator& operator=(const DMAMemoryAllocator&) = delete;

    /// Allocates huge-page-backed DMA memory and maps it into the VFIO IOMMU.
    /// Use virt() for CPU access and iova() as the device DMA address.
    DMABuffer allocate(size_t size);

private:
    static uint64_t alignUpU64(uint64_t value, uint64_t alignment);
    static void* allocDMAVirtualAddr(size_t size);
    static bool bindIOVAWithVirtAddr(void* virt_addr, uint64_t iova, size_t size, int container_fd);

    int m_container_fd {-1};
    uint64_t m_page_size {2ULL * 1024ULL * 1024ULL};
    uint64_t m_next_iova {0x10000};
};
