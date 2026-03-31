# OUCH-over-SoupBinTCP Dual-NIC Demo Design

Date: 2026-03-31

## Goal

Build two host-side TCP modules on the same machine, connected through two physical NIC ports:

- `enp1s0f0`: order gateway side
- `enp5s0f0`: dummy exchange side

The gateway receives intents from the existing executor path, converts them into OUCH-like order-entry messages, and sends them to the dummy exchange over OUCH-over-SoupBinTCP. The dummy exchange returns deterministic OUCH-like responses with a small configurable delay so the demo shows realistic asynchronous exchange behavior.

This design is for a demo and intentionally implements a strict subset of full OUCH/SoupBinTCP behavior.

## Scope

### In Scope

- One direct physical link between `enp1s0f0` and `enp5s0f0`
- Kernel-owned NICs on both ports
- One persistent TCP connection
- SoupBinTCP session layer
- Simplified OUCH 5.0 subset
- Inbound order message:
  - `Type O` Enter Order
- Outbound order responses:
  - `Type A` Order Accepted
  - `Type E` Order Executed
  - `Type J` Rejected
- Gateway reconnect logic
- Benign resend of unconfirmed inbound orders after reconnect
- In-memory exchange session and order state
- Deterministic validation and delayed fills

### Out of Scope

- VFIO / kernel-bypass transport for these two modules
- SoupBinTCP multi-host redundancy
- Disk persistence across process restart
- Full OUCH coverage such as replace, cancel, modify, mass cancel
- SoupBinTCP recovery across exchange process restart
- Multiple simultaneous clients
- Production-grade matching engine behavior

## Why Kernel TCP

The project already contains a userspace Intel NIC driver, but that path is a raw frame path and not a TCP stack. Implementing standard TCP on top of VFIO would require connection management, retransmission, acknowledgment tracking, timers, flow control, and interoperability with a peer TCP stack. That is unnecessary for this demo.

Using kernel TCP is the most practical choice because:

- the requirement is explicitly "two modules that speak TCP"
- the Linux kernel already provides a correct TCP implementation
- the demo remains realistic for HFT order-entry style traffic
- the application can focus on OUCH and SoupBinTCP semantics instead of basic transport reliability

## Network Topology

The two NICs are connected by a real physical cable or DAC.

Recommended addressing:

- `enp1s0f0`: `192.168.50.1/30`
- `enp5s0f0`: `192.168.50.2/30`

Rules:

- both interfaces must be administratively up
- no default route should be attached to either test interface
- no external gateway is required
- the gateway client socket binds to `192.168.50.1`
- the dummy exchange server socket binds to `192.168.50.2:<port>`
- host network managers should not interfere with either interface during the demo

The lack of a default route on these test ports prevents them from being used as general outbound network paths by mistake. Traffic between `192.168.50.1` and `192.168.50.2` still works because both endpoints are on the same directly connected subnet.

## Architecture

This demo uses two separate executables:

- one server executable for the dummy exchange
- one client executable for the order gateway

The intended operator flow is:

- start the dummy exchange server in terminal 1
- start the gateway client in terminal 2

### Order Gateway

The order gateway runs on `enp1s0f0` and acts as the SoupBinTCP client.

Responsibilities:

- receive `OrderIntent` objects from the existing executor
- assign a strictly increasing `UserRefNum`
- map each intent into a simplified OUCH `Enter Order`
- wrap the OUCH payload inside a SoupBinTCP `Unsequenced Data` packet
- maintain the TCP connection and Soup login state
- receive Soup sequenced packets from the exchange
- decode embedded OUCH `Accepted`, `Executed`, and `Rejected` messages
- track local order state and timestamps
- reconnect and resend unconfirmed orders after disconnect

### Dummy Exchange

The dummy exchange runs on `enp5s0f0` and acts as the SoupBinTCP server.

Responsibilities:

- accept one TCP client session
- validate Soup login credentials
- emit Soup `Login Accepted` or `Login Rejected`
- accept Soup `Unsequenced Data` packets carrying OUCH `Enter Order`
- apply deterministic validation rules
- send OUCH responses inside Soup `Sequenced Data` packets
- generate Soup server heartbeats when idle
- maintain session state, outbound sequence state, and order outcomes in memory
- handle benign duplicate inbound orders by `UserRefNum`

### Existing Repo Integration

The current `Executor -> TxEngine` boundary is preserved conceptually:

- `Executor` remains the source of intents
- `TxEngine` is replaced or extended into an order gateway transport adapter
- a new dummy exchange module is added alongside the existing host-side demo components

This keeps the new work aligned with the current host-side application structure instead of introducing an unrelated code path.

## Executable Layout

The first implementation should produce separate binaries instead of one combined process:

- `dummy_exchange_server`
  - binds to `192.168.50.2:<port>`
  - listens for one SoupBinTCP client
  - owns exchange-side session, sequencing, validation, and delayed fills

- `order_gateway_client`
  - binds locally to `192.168.50.1`
  - connects to `192.168.50.2:<port>`
  - owns Soup login, heartbeat, order submission, reconnect, and response correlation
  - can either ingest synthetic test orders directly or receive intents from the existing executor path

Operationally, the demo is run with two terminals:

1. terminal 1 starts `dummy_exchange_server`
2. terminal 2 starts `order_gateway_client`

This separation is preferable for the demo because:

- it mirrors a real client/exchange process boundary
- it makes startup ordering explicit
- it simplifies socket lifecycle debugging
- it makes reconnect tests easier because either side can be restarted independently

## Protocol Model

### SoupBinTCP Layer

All traffic is framed as SoupBinTCP packets:

- client to server:
  - `L` Login Request
  - `U` Unsequenced Data
  - `R` Client Heartbeat
  - `O` Logout Request
- server to client:
  - `A` Login Accepted
  - `J` Login Rejected
  - `S` Sequenced Data
  - `H` Server Heartbeat
  - `Z` End of Session

SoupBinTCP packet framing:

- 2-byte big-endian packet length
- 1-byte packet type
- variable-length payload

Sequencing rules:

- only server-to-client `S` packets are sequenced
- the first sequence number in a session is 1
- the server and client both compute the current sequence number by counting sequenced outbound packets
- the client reconnects with the next expected sequence number

Heartbeats:

- the server sends `H` if it has not sent data for more than 1 second
- the client sends `R` if it has not sent data for more than 1 second

### OUCH Subset

The application payload carried by Soup is a subset of OUCH 5.0:

- inbound:
  - `O` Enter Order
- outbound:
  - `A` Order Accepted
  - `E` Order Executed
  - `J` Rejected

Rules borrowed from OUCH:

- messages are fixed-length
- numeric fields are binary big-endian
- `UserRefNum` is the application transaction key
- the gateway treats inbound order submissions as retryable

Simplifications for the demo:

- no SoupBinTCP debug packets are required
- no optional OUCH appendages in v1
- no cancel/replace/modify flows
- no full Nasdaq session semantics beyond one active in-memory session

## Message Mapping

The gateway converts `OrderIntent` into a simplified OUCH `Enter Order`.

Required logical fields:

- message type
- `UserRefNum`
- side
- symbol or stock locate mapping
- shares
- price
- optional time-in-force or display flag fields only if needed by the chosen struct layout

The dummy exchange returns:

- `Accepted` when the order passes validation
- `Rejected` when the order fails validation
- `Executed` after a configured delay for accepted orders

For the demo, execution is a full fill. Partial fills are intentionally excluded.

## Order State Model

The gateway order state machine is:

- `PendingSend`
- `PendingAck`
- `Rejected`
- `Accepted`
- `Executed`

The exchange logical outcomes are:

- reject immediately
- accept immediately, then execute later

There is no accepted-but-never-filled path in v1 because the demo goal is to show complete request/response behavior with deterministic outcomes.

## Deterministic Exchange Rules

The dummy exchange uses predictable rules so the user can track and explain behavior.

Recommended initial rule set:

- reject unsupported symbols
- reject `shares == 0`
- reject shares above a configured maximum
- reject prices outside a configured min/max band
- otherwise accept
- after accept, issue one full `Executed` message after a configured delay

The fill delay is configurable and intentionally non-zero to demonstrate the difference between order acceptance and execution.

## Sequence And Resend Behavior

SoupBinTCP only guarantees sequenced recovery for server-to-client messages. Client-to-server data carried in Soup `U` packets may be lost if the socket fails. Therefore the gateway must preserve enough local state to resend orders that have not yet reached a terminal outcome.

Gateway resend rules:

- maintain an outstanding order table keyed by `UserRefNum`
- after disconnect, reconnect using the last known session ID and next expected outbound sequence number
- resend only orders for which no sequenced exchange response has yet been observed
- once an order has been observed as `Accepted` or `Rejected`, do not resend it

An `Accepted` order is no longer considered resend-eligible. After reconnect, the client should recover any later `Executed` message from the exchange's sequenced Soup stream instead of re-entering the order.

Dummy exchange idempotency rules:

- if a `UserRefNum` is new, process normally
- if the same `UserRefNum` arrives again after disconnect, do not create a second logical order
- preserve the original logical outcome for that `UserRefNum`

This is sufficient for a demo of benign resend without implementing full exchange redundancy.

## Data Flow

### Normal Path

1. Gateway establishes TCP to `192.168.50.2:<port>` from local IP `192.168.50.1`.
2. Gateway sends Soup `Login Request`.
3. Dummy exchange validates credentials and returns Soup `Login Accepted`.
4. Executor emits an `OrderIntent`.
5. Gateway assigns `UserRefNum`, converts to OUCH `Enter Order`, and wraps it in Soup `Unsequenced Data`.
6. Dummy exchange validates the order.
7. Dummy exchange emits OUCH `Accepted` or `Rejected` inside Soup `Sequenced Data`.
8. If accepted, dummy exchange emits OUCH `Executed` inside a later Soup `Sequenced Data` packet after the configured delay.
9. Gateway correlates responses by `UserRefNum` and updates local timestamps and state.

### Reconnect Path

1. TCP connection drops after one or more orders have been submitted.
2. Gateway records session-down state.
3. Gateway reconnects and sends Soup `Login Request` with requested session and next expected sequence number.
4. Dummy exchange resumes outbound sequenced delivery from the requested point if still available in memory.
5. Gateway resends any still-unconfirmed orders.
6. Dummy exchange treats duplicate `UserRefNum` submissions idempotently.

## Error Handling

Socket handling:

- partial reads must continue until the complete Soup packet header and payload are assembled
- partial writes must continue until the full packet is sent
- `recv() == 0` is treated as orderly disconnect
- transient connection errors trigger reconnect with backoff

Protocol handling:

- invalid Soup packet type closes the session with an error log
- malformed OUCH payload length or unsupported OUCH message type causes the exchange to log and close the connection
- unknown `UserRefNum` on a gateway response is logged and dropped
- invalid login credentials produce Soup `Login Rejected` and socket close

Operational handling:

- if link is down on either interface, the gateway continues retrying connection
- if the server process restarts, in-memory recovery is lost; the client must start a fresh session
- if the client process restarts, its local outstanding-order table is lost; that is acceptable for this demo

## Timing And Observability

The demo should record at least the following timestamps:

- gateway order created
- gateway order serialized and sent
- gateway `Accepted` or `Rejected` received
- gateway `Executed` received

Derived metrics:

- intent-to-send latency
- send-to-ack latency
- send-to-fill latency

Logging should include:

- session ID
- Soup sequence number on outbound exchange events
- `UserRefNum`
- order state transitions
- rejection reason when applicable

Use a monotonic clock for local latency measurements.

## Testing Strategy

### Unit-Level

- Soup packet encoder/decoder
- fixed-length OUCH subset encoder/decoder
- big-endian numeric conversion
- duplicate `UserRefNum` handling
- reconnect bookkeeping and outstanding-order selection

### Integration-Level

- login success path
- login reject path
- valid order -> `Accepted` -> delayed `Executed`
- invalid order -> `Rejected`
- idle heartbeat behavior in both directions
- disconnect after send but before response observed
- reconnect with next expected sequence number
- benign resend of unconfirmed order after reconnect

### Environment-Level

- verify both NICs report link up
- verify `ss` or equivalent shows the TCP connection on the intended IP pair
- verify packet capture on each interface shows only the expected point-to-point flow

## Implementation Boundaries

The first implementation plan should produce:

- a SoupBinTCP codec layer
- a simplified OUCH subset codec layer
- a standalone gateway client executable bound to `enp1s0f0`
- a standalone dummy exchange server executable bound to `enp5s0f0`
- integration of the gateway send path with the existing executor path
- enough tests to validate framing, login, order response flow, and reconnect behavior

The plan should not attempt:

- full OUCH 5.0 support
- cross-process persistence
- VFIO-based TCP
- multiple sessions or accounts

## References

- OUCH 5.0 order-entry specification in repo: `documents/Ouch5.0.pdf`
- SoupBinTCP 3.0 specification: `https://nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/soupbintcp.pdf`
