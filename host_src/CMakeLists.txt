cmake_minimum_required(VERSION 3.16)
project(Venturi LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

option(VENTURI_STABLE_LINK "Relax TX/demo link teardown behavior for single-machine lab use" OFF)

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
    ${COMMON_DIR}/basic_ring_buffer.cpp
    ${COMMON_DIR}/dma_memory_allocator.cpp
    ${COMMON_DIR}/memory_pool.cpp
)

set(COMMON_INCLUDES ${COMMON_DIR})

###############################################################################
# FPGA Driver and Applications
###############################################################################
set(FPGA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FPGA_DRIVER_DIR ${FPGA_ROOT}/fpga_dev)
set(FPGA_ADAPTER_DIR ${FPGA_ROOT}/decoder)
set(FPGA_ENGINE_DIR ${FPGA_ROOT}/fpga_rx_engine)
set(FPGA_LATENCY_DIR ${FPGA_ROOT}/latency)
set(FPGA_STRATEGY_DIR ${FPGA_ROOT}/strategy)
set(FPGA_TX_DIR ${FPGA_ROOT}/tx_client)
set(FPGA_EXCHANGE_DIR ${FPGA_ROOT}/exchange)
set(FPGA_APP_DIR ${FPGA_ROOT}/app)
set(FPGA_TEST_DIR ${FPGA_ROOT}/tests)
set(FPGA_COMMON_DIR ${FPGA_ROOT}/common)


# FPGA Hello World test v2 (using common infrastructure)
set(FPGA_SOURCES
    ${FPGA_DRIVER_DIR}/fpga_dev.cpp
)

add_executable(test_fpga_hello_v2
    ${COMMON_SOURCES}
    ${FPGA_SOURCES}
    ${FPGA_APP_DIR}/test_fpga_hello_v2.cpp
)
target_include_directories(test_fpga_hello_v2 PRIVATE
    ${COMMON_INCLUDES}
    ${FPGA_DRIVER_DIR}
    ${FPGA_ADAPTER_DIR}
)

add_executable(bar_latency_probe
    ${COMMON_SOURCES}
    ${FPGA_SOURCES}
    ${FPGA_COMMON_DIR}/thread_affinity.cpp
    ${FPGA_APP_DIR}/bar_latency_probe.cpp
)
target_include_directories(bar_latency_probe PRIVATE
    ${COMMON_INCLUDES}
    ${FPGA_DRIVER_DIR}
    ${FPGA_ADAPTER_DIR}
)

add_executable(venturi
    ${COMMON_SOURCES}
    ${FPGA_DRIVER_DIR}/fpga_dev.cpp
    ${FPGA_ENGINE_DIR}/fpga_rx_engine.cpp
    ${FPGA_COMMON_DIR}/thread_affinity.cpp
    ${FPGA_LATENCY_DIR}/log_printer.cpp
    ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
    ${FPGA_COMMON_DIR}/spsc_ring_queue.cpp
    ${FPGA_LATENCY_DIR}/latency_tracker.cpp
    ${FPGA_STRATEGY_DIR}/dummy_strategy.cpp
    ${FPGA_TX_DIR}/executor.cpp
    ${FPGA_TX_DIR}/tx_client.cpp
    ${FPGA_TX_DIR}/tx_connector.cpp
    ${FPGA_TX_DIR}/tx_receiver.cpp
    ${FPGA_TX_DIR}/tx_sender.cpp
    ${FPGA_APP_DIR}/venturi.cpp
)
# target_compile_definitions(venturi PRIVATE ISO=1)
if(VENTURI_STABLE_LINK)
    target_compile_definitions(venturi PRIVATE VENTURI_STABLE_LINK=1)
endif()
target_include_directories(venturi PRIVATE
    ${COMMON_INCLUDES}
    ${FPGA_DRIVER_DIR}
    ${FPGA_ADAPTER_DIR}
    ${FPGA_ENGINE_DIR}
    ${FPGA_LATENCY_DIR}
    ${FPGA_STRATEGY_DIR}
    ${FPGA_TX_DIR}
)

add_executable(dummy_server
    ${FPGA_COMMON_DIR}/thread_affinity.cpp
    ${FPGA_EXCHANGE_DIR}/dummy_server.cpp
    ${FPGA_EXCHANGE_DIR}/exchange_protocol.cpp
    ${FPGA_EXCHANGE_DIR}/exchange_transport.cpp
    ${FPGA_APP_DIR}/dummy_server.cpp
)
if(VENTURI_STABLE_LINK)
    target_compile_definitions(dummy_server PRIVATE VENTURI_STABLE_LINK=1)
endif()
target_include_directories(dummy_server PRIVATE
    ${FPGA_EXCHANGE_DIR}
    ${FPGA_ROOT}/common
)

if(TARGET GTest::gtest_main)
    add_executable(fpga_rx_engine_test
        ${COMMON_SOURCES}
        ${FPGA_DRIVER_DIR}/fpga_dev.cpp
        ${FPGA_ENGINE_DIR}/fpga_rx_engine.cpp
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_LATENCY_DIR}/log_printer.cpp
        ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
        ${FPGA_COMMON_DIR}/spsc_ring_queue.cpp
        ${FPGA_LATENCY_DIR}/latency_tracker.cpp
        ${FPGA_TEST_DIR}/fpga_rx_engine_test.cpp
    )
    target_include_directories(fpga_rx_engine_test PRIVATE
        ${COMMON_INCLUDES}
        ${FPGA_DRIVER_DIR}
        ${FPGA_ADAPTER_DIR}
        ${FPGA_ENGINE_DIR}
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(fpga_rx_engine_test PRIVATE
        GTest::gtest_main
    )
    add_executable(publisher_test
        ${FPGA_TEST_DIR}/publisher_test.cpp
    )
    target_include_directories(publisher_test PRIVATE
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(publisher_test PRIVATE
        GTest::gtest_main
    )

    add_executable(tx_translator_test
        ${FPGA_TX_DIR}/tx_client.cpp
        ${FPGA_TX_DIR}/tx_connector.cpp
        ${FPGA_TX_DIR}/tx_receiver.cpp
        ${FPGA_TX_DIR}/tx_sender.cpp
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_LATENCY_DIR}/log_printer.cpp
        ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
        ${FPGA_COMMON_DIR}/spsc_ring_queue.cpp
        ${FPGA_LATENCY_DIR}/latency_tracker.cpp
        ${FPGA_TEST_DIR}/tx_translator_test.cpp
    )
    target_include_directories(tx_translator_test PRIVATE
        ${FPGA_TX_DIR}
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    if(VENTURI_STABLE_LINK)
        target_compile_definitions(tx_translator_test PRIVATE VENTURI_STABLE_LINK=1)
    endif()
    target_link_libraries(tx_translator_test PRIVATE
        GTest::gtest_main
    )
    add_executable(dummy_strategy_test
        ${FPGA_STRATEGY_DIR}/dummy_strategy.cpp
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_LATENCY_DIR}/log_printer.cpp
        ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
        ${FPGA_COMMON_DIR}/spsc_ring_queue.cpp
        ${FPGA_LATENCY_DIR}/latency_tracker.cpp
        ${FPGA_TEST_DIR}/dummy_strategy_test.cpp
    )
    target_include_directories(dummy_strategy_test PRIVATE
        ${FPGA_STRATEGY_DIR}
        ${FPGA_TX_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(dummy_strategy_test PRIVATE
        GTest::gtest_main
    )

    add_executable(executor_test
        ${FPGA_TX_DIR}/executor.cpp
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_LATENCY_DIR}/log_printer.cpp
        ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
        ${FPGA_COMMON_DIR}/spsc_ring_queue.cpp
        ${FPGA_LATENCY_DIR}/latency_tracker.cpp
        ${FPGA_TEST_DIR}/executor_test.cpp
    )
    target_include_directories(executor_test PRIVATE
        ${FPGA_TX_DIR}
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(executor_test PRIVATE
        GTest::gtest_main
    )

    add_executable(dummy_exchange_server_test
        ${FPGA_EXCHANGE_DIR}/dummy_server.cpp
        ${FPGA_EXCHANGE_DIR}/exchange_protocol.cpp
        ${FPGA_EXCHANGE_DIR}/exchange_transport.cpp
        ${FPGA_TEST_DIR}/dummy_exchange_server_test.cpp
    )
    target_include_directories(dummy_exchange_server_test PRIVATE
        ${FPGA_EXCHANGE_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    if(VENTURI_STABLE_LINK)
        target_compile_definitions(dummy_exchange_server_test PRIVATE VENTURI_STABLE_LINK=1)
    endif()
    target_link_libraries(dummy_exchange_server_test PRIVATE
        GTest::gtest_main
    )

    add_executable(exchange_transport_test
        ${FPGA_EXCHANGE_DIR}/exchange_transport.cpp
        ${FPGA_TEST_DIR}/exchange_transport_test.cpp
    )
    target_include_directories(exchange_transport_test PRIVATE
        ${FPGA_EXCHANGE_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(exchange_transport_test PRIVATE
        GTest::gtest_main
    )

    add_executable(exchange_protocol_test
        ${FPGA_EXCHANGE_DIR}/exchange_protocol.cpp
        ${FPGA_TEST_DIR}/exchange_protocol_test.cpp
    )
    target_include_directories(exchange_protocol_test PRIVATE
        ${FPGA_EXCHANGE_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(exchange_protocol_test PRIVATE
        GTest::gtest_main
    )

    add_executable(tx_log_record_test
        ${FPGA_TX_DIR}/tx_client.cpp
        ${FPGA_TX_DIR}/tx_connector.cpp
        ${FPGA_TX_DIR}/tx_receiver.cpp
        ${FPGA_TX_DIR}/tx_sender.cpp
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_LATENCY_DIR}/log_printer.cpp
        ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
        ${FPGA_COMMON_DIR}/spsc_ring_queue.cpp
        ${FPGA_LATENCY_DIR}/latency_tracker.cpp
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

    add_executable(tx_receiver_feedback_test
        ${FPGA_TX_DIR}/tx_receiver.cpp
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_TEST_DIR}/tx_receiver_feedback_test.cpp
    )
    target_include_directories(tx_receiver_feedback_test PRIVATE
        ${FPGA_TX_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    if(VENTURI_STABLE_LINK)
        target_compile_definitions(tx_receiver_feedback_test PRIVATE VENTURI_STABLE_LINK=1)
    endif()
    target_link_libraries(tx_receiver_feedback_test PRIVATE
        GTest::gtest_main
    )

    add_executable(log_printer_test
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_LATENCY_DIR}/log_printer.cpp
        ${FPGA_TEST_DIR}/log_printer_test.cpp
    )
    target_include_directories(log_printer_test PRIVATE
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(log_printer_test PRIVATE
        GTest::gtest_main
    )

    add_executable(latency_tracker_test
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_LATENCY_DIR}/log_printer.cpp
        ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
        ${FPGA_LATENCY_DIR}/latency_tracker.cpp
        ${FPGA_COMMON_DIR}/spsc_ring_queue.cpp
        ${FPGA_TEST_DIR}/latency_tracker_test.cpp
    )
    target_include_directories(latency_tracker_test PRIVATE
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(latency_tracker_test PRIVATE
        GTest::gtest_main
    )

    add_executable(latency_analyzer_test
        ${FPGA_LATENCY_DIR}/latency_analyzer.cpp
        ${FPGA_TEST_DIR}/latency_analyzer_test.cpp
    )
    target_include_directories(latency_analyzer_test PRIVATE
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(latency_analyzer_test PRIVATE
        GTest::gtest_main
    )

    add_executable(spsc_ring_queue_overwrite_test
        ${FPGA_TEST_DIR}/spsc_ring_queue_overwrite_test.cpp
    )
    target_include_directories(spsc_ring_queue_overwrite_test PRIVATE
        ${FPGA_LATENCY_DIR}
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(spsc_ring_queue_overwrite_test PRIVATE
        GTest::gtest_main
    )

    add_executable(fixed_circular_buffer_test
        ${FPGA_TEST_DIR}/fixed_circular_buffer_test.cpp
    )
    target_include_directories(fixed_circular_buffer_test PRIVATE
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(fixed_circular_buffer_test PRIVATE
        GTest::gtest_main
    )

    add_executable(thread_affinity_test
        ${FPGA_COMMON_DIR}/thread_affinity.cpp
        ${FPGA_TEST_DIR}/thread_affinity_test.cpp
    )
    target_include_directories(thread_affinity_test PRIVATE
        ${FPGA_ROOT}/common
        ${FPGA_TEST_DIR}
    )
    target_link_libraries(thread_affinity_test PRIVATE
        GTest::gtest_main
    )



    include(GoogleTest)
    gtest_discover_tests(fpga_rx_engine_test)
    gtest_discover_tests(publisher_test)
    gtest_discover_tests(tx_translator_test)
    gtest_discover_tests(dummy_strategy_test)
    gtest_discover_tests(executor_test)
    gtest_discover_tests(dummy_exchange_server_test)
    gtest_discover_tests(tx_log_record_test)
    gtest_discover_tests(tx_receiver_feedback_test)
    gtest_discover_tests(log_printer_test)
    gtest_discover_tests(latency_tracker_test)
    gtest_discover_tests(latency_analyzer_test)
    gtest_discover_tests(spsc_ring_queue_overwrite_test)
    gtest_discover_tests(fixed_circular_buffer_test)
    gtest_discover_tests(thread_affinity_test)
endif()

###############################################################################
# Build Summary
###############################################################################
message(STATUS "")
message(STATUS "=== Venturi Build Configuration ===")
message(STATUS "Common directory:      ${COMMON_DIR}")
message(STATUS "FPGA demo directory:    ${FPGA_ROOT}")
message(STATUS "GoogleTest directory:   ${GOOGLETEST_ROOT}")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  - test_fpga_hello_v2   (FPGA infrastructure test)")
message(STATUS "  - venturi              (FPGA host runtime)")
message(STATUS "  - dummy_server         (dummy exchange simulator)")
message(STATUS "  - VENTURI_STABLE_LINK=${VENTURI_STABLE_LINK}")
message(STATUS "")
