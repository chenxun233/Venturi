#include "fpga_dev.h"
#include "../../common/log.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ctime>
#include <unistd.h>

FPGADev::FPGADev(std::string pci_addr)
    : m_basic_para {
        .pci_addr = std::move(pci_addr),
        .rx_que_num = 0,
        .tx_que_num = 0,
        .bar0_addr = nullptr,
    } {
}

FPGADev::~FPGADev() = default;

bool FPGADev::initHardware() {
    if (m_hw_ready && m_basic_para.bar0_addr != nullptr) {
        return true;
    }

    info("Initializing FPGA RX hardware...");

    if (!_getFD()) {
        warn("Failed to get VFIO device file descriptor");
        return false;
    }
    if (!_initDMAMemoryAllocator()) {
        warn("Failed to initialize DMA memory allocator");
        return false;
    }
    if (!_getBARAddr()) {
        warn("Failed to map BAR0");
        return false;
    }
    if (m_basic_para.bar0_addr == nullptr) {
        warn("BAR0 not mapped");
        return false;
    }
    _writeReg32(BAR0_GLOBAL_REG_RESET_OFFSET, 1);
    _readSymbolNum();
    m_hw_ready = true;
    return true;
}

bool FPGADev::setRxRingBuffers( uint32_t slot_num, uint32_t slot_size) {
    if (!m_hw_ready && !initHardware()) {
        error("FPGA hardware is not initialized");
        return false;
    }

    if (slot_num == 0 || (slot_num & (slot_num - 1)) != 0) {
        error("RX queue slot count %u is not a power of 2, which may cause issues with the current FPGA design", slot_num);
        return false;
    }

    if (slot_size == 0) {
        error("RX queue slot size must be non-zero");
        return false;
    }


    m_rx_queues.resize(m_basic_para.rx_que_num);

    auto& allocator = _getDMAAllocator();
    for (uint16_t que_idx = 0; que_idx < m_basic_para.rx_que_num; ++que_idx) {
        QueueConfig& queue = m_rx_queues[que_idx];
        queue.slot_num = slot_num;
        queue.slot_size_bytes = slot_size;

        const std::size_t queue_bytes = static_cast<std::size_t>(queue.slot_num) * queue.slot_size_bytes; // slot*size
        queue.dma_memory = allocator.allocate(queue_bytes);
        if (!queue.dma_memory.valid()) {
            warn("Failed to allocate DMA memory for queue %u", que_idx);
            return false;
        }
        std::memset(queue.dma_memory.virt(), 0, queue_bytes);
        _writeReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_IOVA_OFFSET), queue.dma_memory.iova());
        _writeReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_SLOT_COUNT_OFFSET), queue.slot_num);
        _writeReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_CONS_PTR_OFFSET), 0);

        info("Configured RX queue %u: IOVA=0x%016llx slots_num=%u slot_bytes=%u",
             que_idx,
             static_cast<unsigned long long>(queue.dma_memory.iova()),
             queue.slot_num,
             queue.slot_size_bytes);
    }
    m_is_valid = true;
    return true;
}



bool  FPGADev::setSymbolLocate(uint16_t que_idx, uint16_t stock_locate) {
    if (!m_hw_ready) {
        warn("FPGA hardware is not initialized");
        return false;
    }
    if (que_idx < m_rx_queues.size()) {
        m_rx_queues[que_idx].stock_locate = stock_locate;
        _writeReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_SYMBOL_LOC_OFFSET), stock_locate);
        return true;
    }
    warn("Queue index %u more than available %u in setSymbolLocate", que_idx, static_cast<uint16_t>(m_rx_queues.size()));
    return false;
}

bool FPGADev::setPriceBase(uint16_t que_idx, uint64_t price_base) {
    if (!m_hw_ready) {
        warn("FPGA hardware is not initialized");
        return false;
    }
    if (que_idx < m_rx_queues.size()) {
        m_rx_queues[que_idx].price_base = price_base;
        _writeReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_PRICE_BASE_OFFSET), price_base);
        return true;
    }
    warn("Queue index %u more than available %u in setPriceBase", que_idx, static_cast<uint16_t>(m_rx_queues.size()));
    return false;
}




void FPGADev::_readSymbolNum() {
    m_basic_para.rx_que_num = static_cast<uint8_t>(_readReg64(BAR0_GLOBAL_REG_RX_SYMBOL_COUNT_OFFSET));
    m_rx_queues.resize(m_basic_para.rx_que_num);
    info("Device reports %llu symbols", static_cast<unsigned long long>(m_basic_para.rx_que_num));
    return;
}

void FPGADev::readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const {
    prod_ptr = _readReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_PROD_PTR_OFFSET));
}

void FPGADev::writeConsPtr(uint16_t que_idx, uint64_t cons_ptr) {
    _writeReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_CONS_PTR_OFFSET), cons_ptr);
}

uint64_t FPGADev::readDropCount(uint16_t que_idx) const {
    return _readReg64(_getRegAddr(que_idx, BAR0_RX_QUEUE_REG_DROP_COUNT_OFFSET));
}


const uint8_t* FPGADev::pollDataRaw(uint16_t que_idx,
                                    uint64_t cons_ptr) const {
        const uint8_t* dma_base = static_cast<const uint8_t*>(m_rx_queues[que_idx].dma_memory.virt());
        const std::size_t slot_index = static_cast<std::size_t>(cons_ptr & (m_rx_queues[que_idx].slot_num - 1));
        return dma_base + slot_index * m_rx_queues[que_idx].slot_size_bytes;
}





uint32_t FPGADev::_getRegAddr(uint16_t que_idx, uint32_t reg_offset) const {
    return BAR0_RX_QUEUE_BLOCK_BASE_OFFSET +
           static_cast<uint32_t>(que_idx) * BAR0_RX_QUEUE_BLOCK_STRIDE +
           reg_offset;
}

bool FPGADev::_getFD() {
    return _getIOMMUGroupID() &&
           _getVFIOContainerFD() &&
           _getVFIOGroupFD() &&
           _addGroup2Container() &&
           _getDeviceFD();
}

bool FPGADev::_getIOMMUGroupID() {
    std::filesystem::path device_dir =
        std::filesystem::path("/sys/bus/pci/devices") / m_basic_para.pci_addr.c_str();
    struct stat st;
    if (stat(device_dir.c_str(), &st) < 0) {
        warn("PCI device %s not found in sysfs", m_basic_para.pci_addr.c_str());
        return false;
    }
    std::filesystem::path group_link = device_dir / "iommu_group";
    std::error_code ec;
    std::filesystem::path group_target = std::filesystem::read_symlink(group_link, ec);
    if (ec) {
        warn("find the iommu_group for the device: %s", ec.message().c_str());
        return false;
    }
    const std::string group_name = group_target.filename().string();
    m_fds.group_id = std::stoi(group_name);
    info("IOMMU Group ID: %d", m_fds.group_id);
    return true;
}

bool FPGADev::_getVFIOContainerFD() {
    int cfd = m_fds.container_fd;
    if (cfd == -1) {
        cfd = ::open("/dev/vfio/vfio", O_RDWR);
        if (cfd == -1) {
            warn("failed to open /dev/vfio/vfio");
            return false;
        }
        m_fds.container_fd = cfd;
    }
    return true;
}

bool FPGADev::_getVFIOGroupFD() {
    if (m_fds.group_id == -1) {
        warn("Group ID is invalid");
        return false;
    }
    const std::string group_path = "/dev/vfio/" + std::to_string(m_fds.group_id);
    const int gfd = ::open(group_path.c_str(), O_RDWR);
    if (gfd == -1) {
        warn("failed to open %s", group_path.c_str());
        return false;
    }
    m_fds.group_fd = gfd;
    return true;
}

bool FPGADev::_addGroup2Container() {
    if (m_fds.container_fd == -1 || m_fds.group_fd == -1) {
        warn("Container fd or group fd is invalid");
        return false;
    }

    if (ioctl(m_fds.container_fd, VFIO_GET_API_VERSION) != VFIO_API_VERSION) {
        warn("the API version of the container is not compatible");
        return false;
    }

    if (ioctl(m_fds.container_fd, VFIO_CHECK_EXTENSION, VFIO_TYPE1_IOMMU) != 1) {
        warn("the container does not support Type1 IOMMU");
        return false;
    }

    struct vfio_group_status group_status;
    group_status.argsz = sizeof(group_status);
    if (ioctl(m_fds.group_fd, VFIO_GROUP_GET_STATUS, &group_status) == -1) {
        warn("failed to get VFIO group status");
        return false;
    }

    if ((group_status.flags & VFIO_GROUP_FLAGS_VIABLE) == 0) {
        warn("VFIO group is not viable - are all devices in the group bound to the VFIO driver?");
        return false;
    }

    if (ioctl(m_fds.group_fd, VFIO_GROUP_SET_CONTAINER, &m_fds.container_fd) == -1) {
        warn("failed to set container for VFIO group");
        return false;
    }

    const int ret = ::ioctl(m_fds.container_fd, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU);
    if (ret == -1 && errno != EBUSY) {
        warn("set Type1 IOMMU for the container: %s", strerror(errno));
        return false;
    }
    return true;
}

bool FPGADev::_getDeviceFD() {
    if (m_fds.group_fd == -1) {
        warn("Group fd is invalid");
        return false;
    }
    const int dfd = ioctl(m_fds.group_fd, VFIO_GROUP_GET_DEVICE_FD, m_basic_para.pci_addr.c_str());
    if (dfd == -1) {
        warn("failed to get device fd from group");
        return false;
    }
    m_fds.device_fd = dfd;
    return true;
}

void FPGADev::_writeReg64(uint32_t offset, uint64_t value) {
    if (m_basic_para.bar0_addr == nullptr) {
        warn("BAR0 not mapped");
        return;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint64_t* reg = reinterpret_cast<volatile uint64_t*>(m_basic_para.bar0_addr + offset);
    *reg = value;
}

uint64_t FPGADev::_readReg64(uint32_t offset) const {
    if (m_basic_para.bar0_addr == nullptr) {
        warn("BAR0 not mapped");
        return 0;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint64_t* reg = reinterpret_cast<volatile uint64_t*>(m_basic_para.bar0_addr + offset);
    return *reg;
}

void FPGADev::_writeReg32(uint32_t offset, uint32_t value) {
    if (m_basic_para.bar0_addr == nullptr) {
        warn("BAR0 not mapped");
        return;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint32_t* reg = reinterpret_cast<volatile uint32_t*>(m_basic_para.bar0_addr + offset);
    *reg = value;
}

uint32_t FPGADev::_readReg32(uint32_t offset) const {
    if (m_basic_para.bar0_addr == nullptr) {
        warn("BAR0 not mapped");
        return 0;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint32_t* reg = reinterpret_cast<volatile uint32_t*>(m_basic_para.bar0_addr + offset);
    return *reg;
}

bool FPGADev::_getBARAddr() {
    constexpr uint32_t kBarIndex = VFIO_PCI_BAR0_REGION_INDEX;
    if (m_fds.device_fd == -1) {
        warn("Device fd is invalid");
        return false;
    }
    struct vfio_region_info region_info = {};
    region_info.argsz = sizeof(region_info);
    region_info.index = kBarIndex;
    const int ret = ioctl(m_fds.device_fd, VFIO_DEVICE_GET_REGION_INFO, &region_info);
    if (ret == -1) {
        warn("Failed to get region info for BAR0: %s", strerror(errno));
        return false;
    }
    if (region_info.size == 0) {
        warn("BAR0 size is 0");
        return false;
    }
    uint8_t* temp_addr = static_cast<uint8_t*>(mmap(nullptr,
                                                    region_info.size,
                                                    PROT_READ | PROT_WRITE,
                                                    MAP_SHARED,
                                                    m_fds.device_fd,
                                                    region_info.offset));
    if (temp_addr == MAP_FAILED) {
        warn("Failed to mmap BAR0: %s", strerror(errno));
        return false;
    }
    m_basic_para.bar0_addr = temp_addr;
    info("BAR0 mapped at %p (size: 0x%llx)",
         static_cast<void*>(temp_addr),
         static_cast<unsigned long long>(region_info.size));
    return true;
}

bool FPGADev::_initDMAMemoryAllocator() {
    if (m_fds.container_fd < 0) {
        warn("Cannot initialize DMA allocator without a valid container fd");
        return false;
    }
    if (!m_dma_allocator) {
        m_dma_allocator = std::make_unique<DMAMemoryAllocator>(m_fds.container_fd);
    }
    return true;
}

DMAMemoryAllocator& FPGADev::_getDMAAllocator() {
    return *m_dma_allocator;
}

const DMAMemoryAllocator& FPGADev::_getDMAAllocator() const {
    return *m_dma_allocator;
}
