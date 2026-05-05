# Venturi

Venturi is an FPGA-plus-host trading systems project. It receives Nasdaq ITCH 5.0 market data on the FPGA, builds top-of-book state in hardware, exports event records to host memory through PCIe DMA, and runs a host-side demo trading pipeline that can send orders to a simulated exchange.

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
```

For the TX demo path, it is useful to keep `venturi` and `dummy_server` in different network namespaces so the traffic path is less affected by netfilter and other host networking activity. The helper script below sets up a peer namespace for `dummy_server`.

If you want to isolate hot CPUs at boot, set the kernel command line in `/etc/default/grub` and rebuild GRUB. The usual knobs here are `isolcpus=`, `nohz_full=`, `rcu_nocbs=`, and `irqaffinity=`.

Run the demo with three terminals.

Terminal 1: start the dummy exchange:

```bash
sudo ./scripts/dummy_server_netns.sh setup
sudo ./scripts/dummy_server_netns.sh start
```

Terminal 2: run the host runtime:

```bash
sudo ./cpp_src/build/venturi
```

Terminal 3: feed market data into the FPGA path, for example with `tcpreplay`:

```bash
sudo tcpreplay -i <replay-interface> market_data/ONE_MSG_ONE_FRAME.pcap
```

## Documentation

Detailed architecture, module documentation, and implementation notes are published at:

https://chenxun233.github.io/Venturi/
