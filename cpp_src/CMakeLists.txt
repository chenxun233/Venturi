cmake_minimum_required(VERSION 3.16)
project(Venturi LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

add_compile_options(-g -O2 -march=native -fomit-frame-pointer
    -D_XOPEN_SOURCE=700
    -D_DEFAULT_SOURCE
    -Wall
    -Wextra
    -Wpedantic
)

###############################################################################
# Third-Party Test Dependencies
###############################################################################
set(GOOGLETEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/google_test)

if(EXISTS ${GOOGLETEST_ROOT}/CMakeLists.txt)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(${GOOGLETEST_ROOT} EXCLUDE_FROM_ALL)
endif()

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
set(FPGA_ADAPTER_DIR ${FPGA_ROOT}/decoder)
set(FPGA_ENGINE_DIR ${FPGA_ROOT}/engine)
set(FPGA_LATENCY_DIR ${FPGA_ROOT}/latency)
set(FPGA_PROTOCOL_DIR ${FPGA_ROOT}/protocol)
set(FPGA_RUNTIME_DIR ${FPGA_ROOT}/runtime)
set(FPGA_SYNC_DIR ${FPGA_ROOT}/sync)
set(FPGA_STRATEGY_DIR ${FPGA_ROOT}/strategy)
set(FPGA_TUNER_DIR ${FPGA_ROOT}/tuner)
set(FPGA_TX_DIR ${FPGA_ROOT}/tx)
set(FPGA_VALIDATION_DIR ${FPGA_ROOT}/validation)
set(FPGA_EXCHANGE_DIR ${FPGA_ROOT}/exchange)
set(FPGA_APP_DIR ${FPGA_ROOT}/app)
set(FPGA_TEST_DIR ${FPGA_ROOT}/tests)


# FPGA Hello World test v2 (using common infrastructure)
set(FPGA_SOURCES
    ${FPGA_DRIVER_DIR}/basic_rx_source.cpp
    ${FPGA_DRIVER_DIR}/fpga_dev.cpp
    ${FPGA_ADAPTER_DIR}/fpga_rx_decoder.cpp
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
)

add_executable(Venturi
    ${COMMON_SOURCES}
    ${FPGA_DRIVER_DIR}/basic_rx_source.cpp
    ${FPGA_DRIVER_DIR}/fpga_dev.cpp
    ${FPGA_ADAPTER_DIR}/fpga_rx_decoder.cpp
    ${FPGA_ENGINE_DIR}/fpga_rx_engine.cpp
    ${FPGA_LATENCY_DIR}/latency_log_printer.cpp
    ${FPGA_LATENCY_DIR}/trace_buffer.cpp
    ${FPGA_LATENCY_DIR}/latency_tracker.cpp
    ${FPGA_SYNC_DIR}/regression.cpp
    ${FPGA_SYNC_DIR}/sync_handler.cpp
    ${FPGA_STRATEGY_DIR}/dummy_strategy.cpp
    ${FPGA_STRATEGY_DIR}/passive_strategy.cpp
    ${FPGA_TUNER_DIR}/tuner.cpp
    ${FPGA_TX_DIR}/tx_translator.cpp
    ${FPGA_TX_DIR}/executor.cpp
    ${FPGA_TX_DIR}/tx_engine.cpp
    ${FPGA_VALIDATION_DIR}/runtime_payload_validator.cpp
    ${FPGA_APP_DIR}/Venturi.cpp
)
target_include_directories(Venturi PRIVATE
    ${COMMON_INCLUDES}
    ${INTEL_DRIVER_DIR}
    ${FPGA_DRIVER_DIR}
    ${FPGA_ADAPTER_DIR}
    ${FPGA_ENGINE_DIR}
    ${FPGA_LATENCY_DIR}
    ${FPGA_PROTOCOL_DIR}
    ${FPGA_SYNC_DIR}
    ${FPGA_STRATEGY_DIR}
    ${FPGA_TUNER_DIR}
    ${FPGA_TX_DIR}
    ${FPGA_VALIDATION_DIR}
)

add_executable(dummy_server
    ${FPGA_EXCHANGE_DIR}/dummy_exchange_server.cpp
    ${FPGA_EXCHANGE_DIR}/exchange_protocol.cpp
    ${FPGA_APP_DIR}/dummy_exchange_server_main.cpp
)
target_include_directories(dummy_server PRIVATE
    ${FPGA_EXCHANGE_DIR}
    ${FPGA_ROOT}/common
)

if(TARGET GTest::gtest_main)
    add_executable(fpga_rx_decoder_test
        ${FPGA_DRIVER_DIR}/basic_rx_source.cpp
        ${FPGA_DRIVER_DIR}/fake_fpga_dev.cpp
        ${FPGA_ADAPTER_DIR}/fpga_rx_decoder.cpp
        ${FPGA_TEST_DIR}/fpga_rx_decoder_test.cpp
    )
    target_include_directories(fpga_rx_decoder_test PRIVATE
        ${FPGA_DRIVER_DIR}
        ${FPGA_ADAPTER_DIR}
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(fpga_rx_decoder_test PRIVATE
        GTest::gtest_main
    )

    add_executable(regression_test
        ${FPGA_SYNC_DIR}/regression.cpp
        ${FPGA_TEST_DIR}/regression_test.cpp
    )
    target_include_directories(regression_test PRIVATE
        ${FPGA_SYNC_DIR}
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(regression_test PRIVATE
        GTest::gtest_main
    )

    add_executable(tx_translator_test
        ${FPGA_TX_DIR}/tx_translator.cpp
        ${FPGA_LATENCY_DIR}/latency_log_printer.cpp
        ${FPGA_TEST_DIR}/tx_translator_test.cpp
    )
    target_include_directories(tx_translator_test PRIVATE
        ${FPGA_TX_DIR}
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(tx_translator_test PRIVATE
        GTest::gtest_main
    )

    add_executable(dummy_exchange_server_test
        ${FPGA_EXCHANGE_DIR}/dummy_exchange_server.cpp
        ${FPGA_EXCHANGE_DIR}/exchange_protocol.cpp
        ${FPGA_TEST_DIR}/dummy_exchange_server_test.cpp
    )
    target_include_directories(dummy_exchange_server_test PRIVATE
        ${FPGA_EXCHANGE_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(dummy_exchange_server_test PRIVATE
        GTest::gtest_main
    )

    add_executable(tx_log_record_test
        ${FPGA_TX_DIR}/tx_engine.cpp
        ${FPGA_LATENCY_DIR}/latency_log_printer.cpp
        ${FPGA_TEST_DIR}/tx_log_record_test.cpp
    )
    target_include_directories(tx_log_record_test PRIVATE
        ${FPGA_TX_DIR}
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(tx_log_record_test PRIVATE
        GTest::gtest_main
    )

    add_executable(trace_buffer_overwrite_test
        ${FPGA_TEST_DIR}/trace_buffer_overwrite_test.cpp
    )
    target_include_directories(trace_buffer_overwrite_test PRIVATE
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(trace_buffer_overwrite_test PRIVATE
        GTest::gtest_main
    )

    add_executable(fixed_ring_buffer_test
        ${FPGA_TEST_DIR}/fixed_ring_buffer_test.cpp
    )
    target_include_directories(fixed_ring_buffer_test PRIVATE
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(fixed_ring_buffer_test PRIVATE
        GTest::gtest_main
    )



    include(GoogleTest)
    gtest_discover_tests(fpga_rx_decoder_test)
    gtest_discover_tests(regression_test)
    gtest_discover_tests(tx_translator_test)
    gtest_discover_tests(dummy_exchange_server_test)
    gtest_discover_tests(tx_log_record_test)
    gtest_discover_tests(trace_buffer_overwrite_test)
    gtest_discover_tests(fixed_ring_buffer_test)
endif()

###############################################################################
# Build Summary
###############################################################################
message(STATUS "")
message(STATUS "=== Venturi Build Configuration ===")
message(STATUS "Common directory:      ${COMMON_DIR}")
message(STATUS "Intel demo directory:   ${INTEL_ROOT}")
message(STATUS "FPGA demo directory:    ${FPGA_ROOT}")
message(STATUS "GoogleTest directory:   ${GOOGLETEST_ROOT}")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  - test_app_loopsend    (Intel 82599 loop send test)")
message(STATUS "  - test_app_pcap        (Intel 82599 packet capture)")
message(STATUS "  - test_fpga_hello      (FPGA standalone test)")
message(STATUS "  - test_fpga_hello_v2   (FPGA infrastructure test)")
message(STATUS "  - test_fpga_rx_adapter (FPGA RX decoder event pollBatch demo)")
message(STATUS "")
