#include "dma_memory_allocator.h"
#include "log.h"
#include <cerrno>
#include <cstring>
#include <linux/mman.h>
#include <linux/vfio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>

namespace {

constexpr uint64_t kIOVAEnd = UINT64_MAX;

bool unmapIOVA(int container_fd, uint64_t iova, size_t size) {
    if (container_fd < 0 || iova == 0 || size == 0) {
        return false;
    }

    struct vfio_iommu_type1_dma_unmap dma_unmap = {};
    dma_unmap.argsz = sizeof(dma_unmap);
    dma_unmap.iova = iova;
    dma_unmap.size = size;

    if (ioctl(container_fd, VFIO_IOMMU_UNMAP_DMA, &dma_unmap) == -1) {
        warn("Failed to unmap DMA IOVA 0x%llx: %s",
             static_cast<unsigned long long>(iova),
             strerror(errno));
        return false;
    }
    return true;
}

} // namespace

DMABuffer::DMABuffer(int container_fd, void* virt_addr, uint64_t iova_addr, size_t mapped_size)
    : m_container_fd(container_fd),
      m_virt(virt_addr),
      m_iova(iova_addr),
      m_size(mapped_size) {
}

DMABuffer::DMABuffer(DMABuffer&& other) noexcept {
    *this = std::move(other);
}

DMABuffer& DMABuffer::operator=(DMABuffer&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    reset();
    m_container_fd = other.m_container_fd;
    m_virt = other.m_virt;
    m_iova = other.m_iova;
    m_size = other.m_size;

    other.m_container_fd = -1;
    other.m_virt = nullptr;
    other.m_iova = 0;
    other.m_size = 0;
    return *this;
}

DMABuffer::~DMABuffer() {
    reset();
}

void DMABuffer::reset() noexcept {
    if (!valid()) {
        return;
    }

    (void)unmapIOVA(m_container_fd, m_iova, m_size);
    if (munmap(m_virt, m_size) == -1) {
        warn("Failed to munmap DMA memory %p: %s", m_virt, strerror(errno));
    }

    m_container_fd = -1;
    m_virt = nullptr;
    m_iova = 0;
    m_size = 0;
}

DMAMemoryAllocator::DMAMemoryAllocator(int container_fd, uint64_t page_size)
    : m_container_fd(container_fd),
      m_page_size(page_size) {
}

DMABuffer DMAMemoryAllocator::allocate(size_t size) {
    if (m_container_fd < 0) {
        warn("Cannot allocate DMA memory without a valid VFIO container fd");
        return {};
    }
    if (size == 0) {
        warn("Cannot allocate zero-sized DMA memory");
        return {};
    }

    size = _alignUpU64(size, m_page_size);
    const uint64_t iova = _alignUpU64(m_next_iova, m_page_size);
    if (iova > kIOVAEnd || iova + size - 1 > kIOVAEnd) {
        warn("IOMMU aperture exhausted: need 0x%llx bytes", static_cast<unsigned long long>(size));
        return {};
    }

    void* virt_addr = _allocDMAVirtualAddr(size);
    if (virt_addr == nullptr) {
        return {};
    }
    if (!_bindIOVAWithVirtAddr(virt_addr, iova, size, m_container_fd)) {
        if (munmap(virt_addr, size) == -1) {
            warn("Failed to release DMA mapping after VFIO map failure: %s", strerror(errno));
        }
        return {};
    }

    m_next_iova = iova + size;
    return DMABuffer(m_container_fd, virt_addr, iova, size);
}

void* DMAMemoryAllocator::_allocDMAVirtualAddr(size_t size) {
    // using mmap() because it can assign huge page within which the physical memory is continuous.
    void* virtual_address = mmap(NULL,
                                 size,
                                 PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_2MB,
                                 -1,
                                 0);
    if (virtual_address == MAP_FAILED) {
        warn("Failed to mmap DMA memory using huge page: %s", strerror(errno));
        return nullptr;
    }
    return virtual_address;
}

// this function makes the physical address in DRAM shared both by virtual address space and IOVA. one is for CPU access, the other is for device DMA access.
bool DMAMemoryAllocator::_bindIOVAWithVirtAddr(void* virt_addr, uint64_t iova, size_t size, int container_fd) {
    struct vfio_iommu_type1_dma_map dma_map = {};
    dma_map.vaddr = reinterpret_cast<uint64_t>(virt_addr);
    dma_map.iova = iova;
    dma_map.size = size;
    dma_map.argsz = sizeof(dma_map);
    dma_map.flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE;
    if (ioctl(container_fd, VFIO_IOMMU_MAP_DMA, &dma_map) == -1) {
        warn("IOMMU map DMA memory failed: %s", strerror(errno));
        return false;
    }
    return true;
}

uint64_t DMAMemoryAllocator::_alignUpU64(uint64_t value, uint64_t alignment) {
    if (!alignment) {
        return value;
    }
    return (value + alignment - 1) & ~(alignment - 1);
}
