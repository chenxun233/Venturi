# FPGA_boost_demo Hierarchy

## Source Tree

```text
cpp_src/FPGA_boost_demo/
├── app
├── common
├── driver
├── exchange
├── latency
├── rx_engine
├── strategy
├── sync
├── tests
└── tx_client
```

## UML

```mermaid
classDiagram
    class FPGADev
    class FPGARxEngine
    class BasicRxDev
    class DummyStrategy
    class BasicStrategy
    class Executor
    class TxClient
    class TxSender
    class TxReceiver
    class TxConnector
    class LatencyTracker
    class LatencyAnalyzer
    class LogPrinter

    BasicStrategy <|-- DummyStrategy
    BasicRxDev <|-- FPGADev
    FPGARxEngine --> FPGADev

    Executor --> LatencyTracker
    TxClient --> TxConnector
    TxClient --> TxSender
    TxClient --> TxReceiver
    TxSender --> TxConnector
    TxSender --> TxReceiver
    TxSender --> LatencyTracker
    TxConnector --> LogPrinter
```

## Dummy Server

```mermaid
classDiagram
    class DummyServer
    class ExchangeProtocol
    class ExchangeTransport

    DummyServer --> ExchangeProtocol
    DummyServer --> ExchangeTransport
    ExchangeProtocol --> ExchangeTransport
```

## Notes

- `app/venturi.cpp` is the top-level orchestrator.
- `driver` and `rx_engine` form the FPGA RX path.
- `TxClient` wraps `TxSender`, `TxReceiver`, and `TxConnector` as one TX communication subsystem.
- `exchange` contains the dummy exchange simulator and protocol stack.
