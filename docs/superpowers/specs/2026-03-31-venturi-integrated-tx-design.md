# Venturi-Integrated OUCH-over-SoupBinTCP TX Design

Date: 2026-03-31

## Goal

Add a real TX path to the existing top-level host application `Venturi.cpp` so that:

- `Venturi.cpp` remains the only client-side application
- RX path, strategy, executor, latency tracker, and log printer stay in that process
- `TxEngine` replaces the current TX stub
- the dummy exchange remains a separate executable on the second NIC

The client transport is OUCH-over-SoupBinTCP over a direct physical link between `enp1s0f0` and `enp5s0f0`.

## Scope

### In Scope

- Integrate the TX client into `Venturi.cpp`
- Keep `dummy_exchange_server` as a separate executable
- Delete the previously added separate standalone client executable and its related client-only wiring
- Use fixed client-side network/session settings
- Keep TX on the same thread as `Executor`
- Keep retrying TX connection in the background of the executor loop, not by spawning a dedicated TX thread
- Queue pending intents while disconnected
- Drop oldest pending order on queue overflow
- Push TX events into the existing `LatencyLogPrinter`
- Print key TX events immediately to stdout

### Out of Scope

- A separate standalone client executable
- Runtime CLI flags for client NIC/IP/session config
- Disk persistence of outstanding orders
- Full OUCH coverage beyond the existing demo subset
- Multiple concurrent exchange sessions

## Existing Application Boundary

`Venturi.cpp` is already the top application. It owns:

- FPGA RX device setup
- RX engines
- strategies
- executor
- latency tracker
- async log printer

The missing piece is a true TX path behind `Executor -> TxEngine`.

The design must preserve that boundary. The executor remains the point where trading intents become outbound orders. `TxEngine` is upgraded from a stub into a transport client, but no second client app is introduced.

## Network Topology

The two ports are directly linked physically.

Recommended fixed addressing:

- `enp1s0f0`: `192.168.50.1/30`
- `enp5s0f0`: `192.168.50.2/30`

Rules:

- `Venturi.cpp` binds client-side TX sockets to `192.168.50.1`
- `dummy_exchange_server` binds to `192.168.50.2:<port>`
- no default route on either test interface
- both interfaces stay kernel-owned

## Architecture

### Client Side: `Venturi.cpp`

`Venturi.cpp` remains the only client application.

Responsibilities stay unchanged for:

- RX polling
- strategy execution
- executor scheduling
- latency tracking
- async log printing

New responsibility:

- create and attach a real `TxEngine` configured with fixed connection/session settings

There is no separate order gateway client binary. The client-side order gateway behavior is part of `Venturi.cpp`.

### Server Side: `dummy_exchange_server`

The dummy exchange remains a separate executable.

Responsibilities:

- accept one SoupBinTCP client from `Venturi.cpp`
- validate OUCH login/session state
- decode inbound `Enter Order`
- send deterministic outbound `Accepted`, `Executed`, and `Rejected`
- keep in-memory session and order state

### TX Execution Model

`TxEngine` must run on the same thread as `Executor`.

That means:

- no dedicated TX thread
- no separate TX event loop thread
- no independent background reconnect worker

Instead, the executor thread drives TX cooperatively.

The executor loop already runs continuously. The design extends that loop so the same thread also:

- attempts reconnect when disconnected
- sends newly staged orders
- replays pending orders after reconnect
- polls the socket for available exchange responses
- emits Soup client heartbeats when needed

This keeps the concurrency model simple and matches the existing application structure.

## `TxEngine` Responsibilities

`TxEngine` is upgraded from a print stub into a real integrated transport component.

Responsibilities:

- hold fixed client configuration
  - bind IP
  - server IP and port
  - username/password
  - requested session
- assign strictly increasing `UserRefNum`
- convert `OrderIntent` into OUCH `Enter Order`
- wrap OUCH payloads in SoupBinTCP client packets
- manage one persistent TCP connection
- retry connection when unavailable
- keep a bounded pending queue / tracked-order table
- resend pending unconfirmed orders after reconnect
- drop oldest pending order on queue overflow
- decode sequenced exchange responses
- update order state on `Accepted`, `Rejected`, and `Executed`
- report TX events into both immediate prints and async log printer

### Cooperative Progress API

Because `TxEngine` must stay on the executor thread, it needs a non-blocking progress API.

The intended shape is:

- `sendIntent(intent)`
  - stage the order
  - try immediate send if connected
- `runCycle()`
  - called regularly by the executor loop
  - advances connect/reconnect, replay, reads responses, and emits heartbeats

`Executor::run()` should be extended so it continues to drain intent buffers as before, but also calls `TxEngine::runCycle()` each loop iteration.

## Queueing And Replay

When TX is disconnected:

- new intents are still accepted from strategies through executor
- `TxEngine` stages them into a bounded pending queue
- reconnect attempts continue from the executor thread

When the queue is full:

- drop the oldest pending order
- print the drop immediately
- also record the drop into the async TX log stream

When reconnect succeeds:

- replay pending orders in original remaining queue order
- keep accepted/rejected orders out of the resend set once sequenced exchange confirmation has been observed

This behavior is practical for the demo because it preserves recent strategy decisions while keeping memory bounded.

## Protocol Model

The transport remains OUCH-over-SoupBinTCP.

### Client to Exchange

- Soup `Login Request`
- Soup `Unsequenced Data` carrying OUCH `Enter Order`
- Soup `Client Heartbeat`

### Exchange to Client

- Soup `Login Accepted` or `Login Rejected`
- Soup `Sequenced Data` carrying OUCH:
  - `Accepted`
  - `Executed`
  - `Rejected`
- Soup `Server Heartbeat`

The OUCH subset remains:

- inbound:
  - `Type O` Enter Order
- outbound:
  - `Type A` Order Accepted
  - `Type E` Order Executed
  - `Type J` Rejected

## Logging Design

TX events must use both immediate printing and the existing async log printer.

### Immediate prints

Print immediately with flush when:

- connection established
- connection lost
- order sent
- order rejected
- order filled
- queue overflow drop

These are operator-facing demo messages and should not wait behind async logging.

### Async log printer integration

`TxEngine` should also push structured TX events into `LatencyLogPrinter`.

This requires extending the shared async log record model with TX event entries such as:

- `ConnectionEstablished`
- `ConnectionLost`
- `OrderSent`
- `OrderRejected`
- `OrderFilled`
- `OrderDropped`

`LatencyLogPrinter` should print those TX events in a consistent format with the rest of the app’s async logs.

### Attachment model

`TxEngine` should expose `attachLogPrinter(LatencyLogPrinter*)`.

`Venturi.cpp` should attach the same printer already used by the latency tracker and executor.

## Failure Handling

### Connection unavailable at startup

- `Venturi.cpp` should not fail fast
- RX and strategy processing continue
- `TxEngine` keeps retrying and prints connection errors

### Connection loss during runtime

- print loss immediately
- keep staging outbound orders up to bounded capacity
- keep retrying reconnect from executor thread
- replay pending unconfirmed orders after reconnect

### Exchange responses

- `Rejected` marks the order terminal and non-replayable
- `Accepted` marks the order confirmed and non-replayable
- later `Executed` finalizes fill state

## Testing Strategy

### Unit-Level

- Soup codec encode/decode
- OUCH subset encode/decode
- pending queue overflow with drop-oldest policy
- replay set excludes sequenced-confirmed orders
- TX event record creation for logger integration

### Integration-Level

- `Venturi.cpp` build still succeeds with integrated TX
- `dummy_exchange_server` accepts the integrated client
- valid intent produces send -> accept -> fill
- invalid intent produces send -> reject
- disconnected startup keeps retrying without killing `Venturi.cpp`
- reconnect path replays pending orders

### Operator-Level Demo

Two processes only:

1. `dummy_exchange_server`
2. `Venturi.cpp`-based top app

There is no separate client executable in the final design.

## Implementation Boundary

The first implementation plan should:

- remove the separate standalone client executable from the target design
- delete the previously added separate client app target and its related client-only docs/build wiring
- keep only the exchange as a separate app
- integrate the TX session/protocol into the existing `Venturi.cpp` path
- keep TX driven by executor thread only
- extend the async log printer for TX events

The plan should not introduce:

- a new client app
- a TX worker thread
- runtime-configurable client network/session CLI
