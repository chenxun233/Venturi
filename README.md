# Venturi

Venturi is an FPGA-plus-host framework for building and measuring a low-latency market-data-to-order pipeline. Its main purpose is latency tracking rather than trading itself.

The system receives Nasdaq ITCH 5.0 market data on the FPGA, builds top-of-book state in hardware, exports event records to host memory through PCIe DMA, and runs a host-side demo pipeline that can send orders to a simulated exchange. Latency is tracked across the software and hardware stages of this flow, except for PCIe transmission latency.

When `venturi` is stopped with `Ctrl+C`, it prints summary statistics for each tracked stage, including `min`, `p50`, `p99`, and `max`.

This repository contains:

- FPGA RTL for market-data receive, parsing, order-book updates, timestamping, and DMA
- C++ host runtime for polling FPGA events, running demo strategy logic, sending orders, and measuring latency
- a `dummy_server` process for exercising the TX path

## Run

Build the host runtime:

```bash
cd Venturi
cmake -S cpp_src -B cpp_src/build
cmake --build cpp_src/build -j"$(nproc)"
```

Before running, the usual host preparation is:

```bash
sudo ./scripts/reset_pcie.sh <BDF-or-vendor:device>
sudo ./scripts/setup-hugepages.sh
sudo ./scripts/setup-vfio.sh <pci_addr1> [pci_addr2] [pci_addr3]
sudo ./scripts/port_isolation.sh <dev>
```

For the TX demo path, it is useful to keep `venturi` and `dummy_server` in different network namespaces so the traffic path is less affected by netfilter and other host networking activity. The helper script below sets up a peer namespace for `dummy_server`.

If you want to isolate hot CPUs at boot, set the kernel command line in `/etc/default/grub` and rebuild GRUB. The usual knobs here are `isolcpus=`, `nohz_full=`, `rcu_nocbs=`, and `irqaffinity=`.

Run the demo with three terminals.

Terminal 1: start the dummy exchange:

```bash
sudo ip netns exec <dummy_server_namespace> ./cpp_src/build/dummy_server
```

Terminal 2: run the host runtime:

```bash
sudo ip netns exec <venturi_namespace> ./cpp_src/build/venturi
```

Terminal 3: feed market data into the FPGA path, for example with `tcpreplay`:

```bash
sudo tcpreplay -i <replay-interface> market_data/ONE_MSG_ONE_FRAME.pcap
```

## Print

When tcpreplay finished, you can press `ctrl+c` in `venturi` terminal, below will be printed:

```bash
[INFO   ] Venturi/cpp_src/FPGA_boost_demo/fpga_dev/fpga_dev.cpp:30 initHardware(): Initializing FPGA RX hardware...
[INFO   ] Venturi/cpp_src/FPGA_boost_demo/fpga_dev/fpga_dev.cpp:195 _getGroupID(): IOMMU Group ID: 14
[INFO   ] Venturi/cpp_src/FPGA_boost_demo/fpga_dev/fpga_dev.cpp:351 _getBARAddr(): BAR0 mapped at 0x789603bcd000 (size: 0x4000)
[INFO   ] Venturi/cpp_src/FPGA_boost_demo/fpga_dev/fpga_dev.cpp:136 _readSymbolNum(): Device reports 2 symbols
[INFO   ] Venturi/cpp_src/FPGA_boost_demo/fpga_dev/fpga_dev.cpp:90 setRxRingBuffers(): Configured RX queue 0: IOVA=0x0000000000200000 slots_num=1024 slot_bytes=32
[INFO   ] Venturi/cpp_src/FPGA_boost_demo/fpga_dev/fpga_dev.cpp:90 setRxRingBuffers(): Configured RX queue 1: IOVA=0x0000000000400000 slots_num=1024 slot_bytes=32
TxEvent queue=1 event=ConnectionEstablished
TxEvent queue=0 event=ConnectionEstablished
^Cqueue=0 total_received=16000 traced=7961
queue=1 total_received=16000 traced=7960
Latency Summary queue=0
record_count=15921 traced records=7961 warmup=1000 analyzed=6961

stage                                          min_ns       p50_ns       p90_ns       p99_ns       max_ns
FRAME_START_to_DMA_EMIT                           160          185          185          185          204
BATCH_DURATION                                     59           93          118          130          161
BATCH_END_to_STRATEGY_START                        52           66           92          101          146
STRATEGY_START_to_TX_SEND_ACCEPTED                 21           29           53          102          269
TX_SEND_ACCEPTED_to_TX_SEND_SYSCALL_ENTER           29           35           64          128          549
TX_SEND_SYSCALL_ENTER_to_TX_SEND                 1092         1304         1421         1539         1940

Latency Summary queue=1
record_count=15921 traced records=7960 warmup=1000 analyzed=6960

stage                                          min_ns       p50_ns       p90_ns       p99_ns       max_ns
FRAME_START_to_DMA_EMIT                           160          185          185          185          204
BATCH_DURATION                                     68           94          118          132          169
BATCH_END_to_STRATEGY_START                        54           91          117          130          222
STRATEGY_START_to_TX_SEND_ACCEPTED                 21           33           57          113          271
TX_SEND_ACCEPTED_to_TX_SEND_SYSCALL_ENTER           30           35           62          133          285
TX_SEND_SYSCALL_ENTER_to_TX_SEND                 1105         1283         1394         1532         1951

TxReceiver queue=0 sent=8000 accepted=8000 filled=8000 rejected=0 pending=0 malformed=0 ref_drops=0 connected=1 disconnected=0
TxReceiver queue=1 sent=8000 accepted=8000 filled=8000 rejected=0 pending=0 malformed=0 ref_drops=0 connected=1 disconnected=0
```

The only missing latency is the PCIe transmission latency. Because we do not have a hardware synced clock source.

## Documentation

Detailed architecture, module documentation, and implementation notes are published at:

https://chenxun233.github.io/Venturi/
