# Venturi C++ Host Runtime

This directory contains the host-side C++ code for Venturi. It includes shared VFIO and DMA infrastructure and the FPGA demo runtime that receives FPGA DMA records, generates order intents, sends orders to a dummy exchange, and records host-side latency.

## Overview

The main host runtime flow is:

1. The host polls DMA records produced by the FPGA.
2. Raw records are decoded into `FPGAEventDesc` values.
3. Strategy logic converts events into order intents.
4. The executor converts intents into concrete order executions.
5. The TX client sends orders to `dummy_server`.
6. Exchange feedback is parsed and matched against pending orders.
7. Latency components record timing across the pipeline.

The main executables are:

- `venturi`: top-level FPGA demo host process
- `dummy_server`: standalone exchange simulator for the TX path
- `test_fpga_hello_v2`: FPGA device validation entry point

## Directory Layout

```text
cpp_src/
├── CMakeLists.txt
├── common/
├── FPGA_boost_demo/
│   ├── app/
│   ├── common/
│   ├── decoder/
│   ├── driver/
│   ├── exchange/
│   ├── latency/
│   ├── rx_engine/
│   ├── strategy/
│   ├── sync/
│   ├── tests/
│   └── tx_client/
└── third_party/
```

### `common`

Shared host infrastructure used by the runtime:

- `basic_dev.*`: abstract device base class
- `basic_ring_buffer.*`: generic ring buffer support
- `dma_memory_allocator.*`: DMA-capable memory allocation
- `memory_pool.*`: packet and buffer pool support
- `device.h`, `log.h`: low-level utilities and logging

### `FPGA_boost_demo`

The FPGA demo runtime is organized by runtime ownership rather than only by folder shape:

- `app/`: executable entry points such as `venturi` and `dummy_server`
- `driver/`: FPGA PCIe device access and RX source plumbing
- `decoder/`: FPGA record decoding support
- `rx_engine/`: DMA polling, decode orchestration, and batch emission
- `strategy/`: event-to-intent logic
- `tx_client/`: order execution, gateway connection, send, and feedback handling
- `exchange/`: standalone dummy exchange runtime and protocol transport
- `latency/`: timestamp capture, aggregation, and logging
- `common/`: FPGA-demo-specific shared utilities
- `sync/`: regression and synchronization helpers
- `tests/`: unit and component tests for the FPGA demo runtime

## Runtime Structure

### `venturi`

`venturi` is the top-level host process for the FPGA demo. Its main orchestration lives in `FPGA_boost_demo/app/venturi.cpp` and wires together:

- `FPGARxEngine` for DMA polling and decode
- `DummyStrategy` for event evaluation
- `Executor` for intent-to-execution conversion
- `TxClient` for outbound order transport and feedback processing
- `LatencyTracker`, `LatencyAnalyzer`, and `LogPrinter` for host-side timing

The current runtime creates two RX/strategy/execution/TX lanes, pins worker threads to fixed CPUs, and connects the TX side to the dummy exchange at:

- client bind IP: `192.168.51.1`
- server IP: `192.168.51.2`
- port: `9000`

### `dummy_server`

`dummy_server` is a standalone exchange simulator. It accepts client connections, validates login and session state, generates exchange responses, and can delay fills to make latency behavior visible.

The executable entry point is `FPGA_boost_demo/app/dummy_server.cpp`. The runtime implementation lives under `FPGA_boost_demo/exchange/`.

## Building

### Configure

```bash
cd /home/chenxun/Documents/Project/Venturi
cmake -S cpp_src -B cpp_src/build
```

### Build Everything

```bash
cmake --build cpp_src/build -j"$(nproc)"
```

### Build Selected Targets

```bash
cmake --build cpp_src/build --target venturi dummy_server
cmake --build cpp_src/build --target test_fpga_hello_v2
```

## Running

### FPGA Validation

`test_fpga_hello_v2` expects a PCI address and a test number:

```bash
sudo ./cpp_src/build/test_fpga_hello_v2 0000:05:00.0 1
```

### Dummy Exchange Demo

Build and run the host runtime together with the dummy exchange:

```bash
cmake --build cpp_src/build --target dummy_server venturi
```

Terminal 1:

```bash
./cpp_src/build/dummy_server \
  --listen-ip 192.168.51.2 \
  --port 9000 \
  --fill-delay-ms 20
```

Terminal 2:

```bash
sudo ./cpp_src/build/venturi
```

## Testing

Run the CTest-discovered suite from the build directory:

```bash
cd /home/chenxun/Documents/Project/Venturi/cpp_src/build
ctest --output-on-failure
```

## Host Environment Notes

The host-side code assumes a Linux environment with:

- CMake 3.16 or newer
- a C++20-capable compiler
- VFIO and IOMMU support enabled
- permission to access PCI devices from userspace

Helpful setup scripts under the repository `scripts/` directory include:

- `scripts/setup-vfio.sh`
- `scripts/setup-hugepages.sh`
- `scripts/reset_pcie.sh`
- `scripts/port_isolation.sh`
- `scripts/dummy_server_netns.sh`

## Related Documentation

- `../README.md`: repository-level project overview
- `common/README.md`: shared host infrastructure details
- `../hierarchy.md`: FPGA demo source tree and class relationships

## Status

This directory is actively evolving. In particular:

- the FPGA demo runtime is present and buildable
- the `venturi` TX path is integrated with `TxClient` and `dummy_server`
- some FPGA validation paths are still placeholders in `test_fpga_hello_v2`

## License

This code follows the Venturi project license.
