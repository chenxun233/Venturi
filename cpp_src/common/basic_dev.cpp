#include "basic_dev.h"
#include "log.h"
#include <filesystem>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

static double diff_mpps(uint64_t pkts_new, uint64_t pkts_old, uint64_t nanos) {
	return (double) (pkts_new - pkts_old) / 1000000.0 / ((double) nanos / 1000000000.0);
}


static uint32_t diff_mbit(uint64_t bytes_new, uint64_t bytes_old, uint64_t pkts_new, uint64_t pkts_old, uint64_t nanos) {
	// take stuff on the wire into account, i.e., the preamble, SFD and IFG (20 bytes)
	// otherwise it won't show up as 10000 mbit/s with small packets which is confusing
	return (uint32_t) (((bytes_new - bytes_old) / 1000000.0 / ((double) nanos / 1000000000.0)) * 8
		+ diff_mpps(pkts_new, pkts_old, nanos) * 20 * 8);
}

BasicDev::BasicDev(std::string pci_addr,uint8_t max_bar_index):
m_basic_para()
{
    // initialize struct members in the constructor body
    m_basic_para.pci_addr = pci_addr;
    m_basic_para.num_rx_queues = 0;
    m_basic_para.num_tx_queues = 0;
    m_basic_para.max_bar_index = max_bar_index;
    m_basic_para.interrupt_timeout_ms = 100; 
    for (auto& addr : m_basic_para.p_bar_addr) {
        addr = nullptr;
    }
}


uint64_t BasicDev::_monotonic_time(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000 + static_cast<uint64_t>(ts.tv_nsec);
}

basic_para_type BasicDev::get_basic_para(){
    return m_basic_para;
}


void BasicDev::_print_stats_diff(DevStatus* stats_new, DevStatus* stats_old, uint64_t nanos){
	printf("[%s] RX: %d Mbit/s %.2f Mpps\n", m_basic_para.pci_addr.c_str(),
		diff_mbit(stats_new->rx_bytes, stats_old->rx_bytes, stats_new->rx_pkts, stats_old->rx_pkts, nanos),
		diff_mpps(stats_new->rx_pkts, stats_old->rx_pkts, nanos)
	);
	printf("[%s] TX: %d Mbit/s %.2f Mpps\n", m_basic_para.pci_addr.c_str(),
		diff_mbit(stats_new->tx_bytes, stats_old->tx_bytes, stats_new->tx_pkts, stats_old->tx_pkts, nanos),
		diff_mpps(stats_new->tx_pkts, stats_old->tx_pkts, nanos)
	);

}

// ============================================================================
// Common VFIO Setup Functions (shared by all PCIe drivers)
// ============================================================================

bool BasicDev::_getFD() {
    return
        this->_getGroupID() &&
        this->_getContainerFD() &&
        this->_getGroupFD() &&
        this->_addGroup2Container() &&
        this->_getDeviceFD();
}

bool BasicDev::_getGroupID() {
    std::filesystem::path device_dir = std::filesystem::path("/sys/bus/pci/devices") / this->m_basic_para.pci_addr.c_str();
    struct stat st;
    int ret = stat(device_dir.c_str(), &st);
    if (ret < 0) {
        warn("PCI device %s not found in sysfs", this->m_basic_para.pci_addr.c_str());
        return false;
    }
    std::filesystem::path group_link = device_dir / "iommu_group";
    std::error_code ec;
    std::filesystem::path group_target = std::filesystem::read_symlink(group_link, ec);
    if (ec) {
        warn("find the iommu_group for the device: %s", ec.message().c_str());
        return false;
    }
    std::string group_name = group_target.filename().string();
    int group_id = std::stoi(group_name);
    this->m_fds.group_id = group_id;
    info("IOMMU Group ID: %d", group_id);
    return true;
}

bool BasicDev::_getContainerFD() {
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

bool BasicDev::_getGroupFD() {
    if (this->m_fds.group_id == -1) {
        warn("Group ID is invalid");
        return false;
    }
    std::string group_path = "/dev/vfio/" + std::to_string(this->m_fds.group_id);
    int gfd = ::open(group_path.c_str(), O_RDWR);
    if (gfd == -1) {
        warn("failed to open %s", group_path.c_str());
        return false;
    }
    this->m_fds.group_fd = gfd;
    return true;
}

bool BasicDev::_addGroup2Container() {
    if (this->m_fds.container_fd == -1 || this->m_fds.group_fd == -1) {
        warn("Container fd or group fd is invalid");
        return false;
    }

    if (!(ioctl(this->m_fds.container_fd, VFIO_GET_API_VERSION) == VFIO_API_VERSION)) {
        warn("the API version of the container is not compatible");
        return false;
    }

    // check if type1 is supported
    if (!(ioctl(this->m_fds.container_fd, VFIO_CHECK_EXTENSION, VFIO_TYPE1_IOMMU) == 1)) {
        warn("the container does not support Type1 IOMMU");
        return false;
    }

    // check if group is viable
    struct vfio_group_status group_status;
    group_status.argsz = sizeof(group_status);
    if (ioctl(this->m_fds.group_fd, VFIO_GROUP_GET_STATUS, &group_status) == -1) {
        warn("failed to get VFIO group status");
        return false;
    }

    if (!((group_status.flags & VFIO_GROUP_FLAGS_VIABLE) > 0)) {
        warn("VFIO group is not viable - are all devices in the group bound to the VFIO driver?");
        return false;
    }

    if (ioctl(this->m_fds.group_fd, VFIO_GROUP_SET_CONTAINER, &this->m_fds.container_fd) == -1) {
        warn("failed to set container for VFIO group");
        return false;
    }

    int ret = ::ioctl(this->m_fds.container_fd, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU);
    if (ret == -1 && errno != EBUSY) {
        warn("set Type1 IOMMU for the container: %s", strerror(errno));
        return false;
    }
    return true;
}

bool BasicDev::_getDeviceFD() {
    if (this->m_fds.group_fd == -1) {
        warn("Group fd is invalid");
        return false;
    }
    int dfd = ioctl(this->m_fds.group_fd, VFIO_GROUP_GET_DEVICE_FD, this->m_basic_para.pci_addr.c_str());
    if (dfd == -1) {
        warn("failed to get device fd from group");
        return false;
    }
    this->m_fds.device_fd = dfd;
    return true;
}

bool BasicDev::_getBARAddr(uint8_t bar_index) {
    m_basic_para.max_bar_index = bar_index;
    if (m_basic_para.max_bar_index > VFIO_PCI_BAR5_REGION_INDEX) {
        warn("BAR index %d is out of range", m_basic_para.max_bar_index);
        return false;
    }
    if (this->m_fds.device_fd == -1) {
        warn("Device fd is invalid");
        return false;
    }
    for (int i = 0; i <= m_basic_para.max_bar_index; i++) {
        struct vfio_region_info region_info = {};
        region_info.argsz = sizeof(region_info);
        region_info.index = i;
        int ret = ioctl(this->m_fds.device_fd, VFIO_DEVICE_GET_REGION_INFO, &region_info);
        if (ret == -1) {
            warn("Failed to get region info for BAR %d: %s", i, strerror(errno));
            return false;
        }
        if (region_info.size == 0) {
            info("BAR%d size is 0, skipping", i);
            continue;
        }
        uint8_t* temp_addr = static_cast<uint8_t*>(mmap(NULL, region_info.size, PROT_READ | PROT_WRITE,
                                                         MAP_SHARED, this->m_fds.device_fd, region_info.offset));
        if (temp_addr == MAP_FAILED) {
            warn("Failed to mmap BAR %d: %s", i, strerror(errno));
            return false;
        }
        m_basic_para.p_bar_addr[i] = temp_addr;
        info("BAR%d mapped at %p (size: 0x%llx)", i, (void*)temp_addr,
             (unsigned long long)region_info.size);
    }
    return true;
}