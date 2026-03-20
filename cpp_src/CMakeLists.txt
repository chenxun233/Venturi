cmake_minimum_required(VERSION 3.16)
project(Venturi LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-g -O2 -march=native -fomit-frame-pointer
    -D_XOPEN_SOURCE=700
    -D_DEFAULT_SOURCE
    -Wall
    -Wextra
    -Wpedantic
)

###############################################################################
# Common Infrastructure (shared by all drivers)
###############################################################################
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)

set(COMMON_SOURCES
    ${COMMON_DIR}/basic_dev.cpp
    ${COMMON_DIR}/basic_ring_buffer.cpp
    ${COMMON_DIR}/dma_memory_allocator.cpp
    ${COMMON_DIR}/memory_pool.cpp
)

set(COMMON_INCLUDES ${COMMON_DIR})

###############################################################################
# Intel 82599 NIC Driver
###############################################################################
set(INTEL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Intel_demo)
set(INTEL_DRIVER_DIR ${INTEL_ROOT}/driver)
set(INTEL_APP_DIR ${INTEL_ROOT}/app)

set(INTEL_SOURCES
    ${INTEL_DRIVER_DIR}/vfio_dev.cpp
    ${INTEL_DRIVER_DIR}/ixgbe_ring_buffer.cpp
    ${INTEL_DRIVER_DIR}/factory.cpp
)

# Intel test applications
add_executable(test_app_loopsend
    ${COMMON_SOURCES}
    ${INTEL_SOURCES}
    ${INTEL_APP_DIR}/test_app_loopsend.cpp
)
target_include_directories(test_app_loopsend PRIVATE
    ${COMMON_INCLUDES}
    ${INTEL_DRIVER_DIR}
)

add_executable(test_app_pcap
    ${COMMON_SOURCES}
    ${INTEL_SOURCES}
    ${INTEL_APP_DIR}/test_app_pcap.cpp
)
target_include_directories(test_app_pcap PRIVATE
    ${COMMON_INCLUDES}
    ${INTEL_DRIVER_DIR}
)

###############################################################################
# FPGA Driver and Applications
###############################################################################
set(FPGA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/FPGA_boost_demo)
set(FPGA_DRIVER_DIR ${FPGA_ROOT}/driver)
set(FPGA_ADAPTER_DIR ${FPGA_ROOT}/adapter)
set(FPGA_VALIDATION_DIR ${FPGA_ROOT}/validation)
set(FPGA_APP_DIR ${FPGA_ROOT}/app)


# FPGA Hello World test v2 (using common infrastructure)
set(FPGA_SOURCES
    ${FPGA_DRIVER_DIR}/fpga_dev.cpp
    ${FPGA_ADAPTER_DIR}/fpga_rx_adapter.cpp
    ${FPGA_VALIDATION_DIR}/fpga_replay_validator.cpp
)

add_executable(test_fpga_hello_v2
    ${COMMON_SOURCES}
    ${FPGA_SOURCES}
    ${FPGA_APP_DIR}/test_fpga_hello_v2.cpp
)
target_include_directories(test_fpga_hello_v2 PRIVATE
    ${COMMON_INCLUDES}
    ${INTEL_DRIVER_DIR}  # Need Intel headers for ixgbe_type.h (used by common/basic_ring_buffer.cpp)
    ${FPGA_DRIVER_DIR}
    ${FPGA_ADAPTER_DIR}
    ${FPGA_VALIDATION_DIR}
)

add_executable(test_fpga_rx_adapter
    ${COMMON_SOURCES}
    ${FPGA_SOURCES}
    ${FPGA_APP_DIR}/test_fpga_rx_adapter.cpp
)
target_include_directories(test_fpga_rx_adapter PRIVATE
    ${COMMON_INCLUDES}
    ${INTEL_DRIVER_DIR}
    ${FPGA_DRIVER_DIR}
    ${FPGA_ADAPTER_DIR}
    ${FPGA_VALIDATION_DIR}
)

###############################################################################
# Build Summary
###############################################################################
message(STATUS "")
message(STATUS "=== Venturi Build Configuration ===")
message(STATUS "Common directory:      ${COMMON_DIR}")
message(STATUS "Intel demo directory:   ${INTEL_ROOT}")
message(STATUS "FPGA demo directory:    ${FPGA_ROOT}")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  - test_app_loopsend    (Intel 82599 loop send test)")
message(STATUS "  - test_app_pcap        (Intel 82599 packet capture)")
message(STATUS "  - test_fpga_hello      (FPGA standalone test)")
message(STATUS "  - test_fpga_hello_v2   (FPGA infrastructure test)")
message(STATUS "  - test_fpga_rx_adapter (FPGA RX adapter event poll demo)")
message(STATUS "")
