# Stage 1 TxConnection Control Publisher Design

## Goal

Build only the first visible TX module step in `Venturi.cpp`:

- `TxConnection` becomes a pure connection lifecycle module
- `Venturi.cpp` wires it in directly
- the stage is observable only through internal control publication and tests

This stage is intentionally incomplete. It exists to establish ownership and lifecycle semantics before sender or receiver behavior is introduced.

## Why Stage 1 Exists

The current TX split has overlapping responsibilities. Stage 1 deliberately strips the first module down to one job:

- create and manage transport connection lifecycle
- publish fresh connection controls

The point is not to send or receive data yet.
The point is to make fd ownership, generation semantics, and reconnect behavior understandable in isolation, but still visible in the actual `Venturi` runtime.

## Chosen Stage 1 Approach

Chosen approach:

- keep `TxConnection` only
- wire it into `Venturi.cpp`
- add a tiny connection lifecycle loop/thread in `Venturi`
- do not instantiate `TxReceiver`
- do not instantiate `TxSender`
- do not add login or order flow

Rejected alternatives:

- tests only, no `Venturi` wiring
  - rejected because the user explicitly wants incremental `Venturi` construction
- add a separate observer/helper object in `Venturi`
  - rejected because that adds structure without helping Stage 1 understanding

## Stage 1 Responsibilities

### `TxConnection` Owns

- connect/reconnect timing
- socket creation
- socket option setup
- connection generation tracking
- publication of `TxConnectionInfo`
- cleanup of unpublished fds on teardown

### `TxConnection` Does Not Own

- `send()`
- `recv()`
- inbound frame assembly
- Soup session state
- OUCH protocol state
- pending/replay state
- sender/receiver queues

If current `TxConnection` still owns any of the forbidden items above, Stage 1 is expected to remove them.

## Stage 1 Public Contract

At the end of Stage 1, `TxConnection` should expose a connection-lifecycle-only API shape:

```cpp
class TxConnection {
public:
    TxConnection();
    explicit TxConnection(GatewayClientConfig config);
    ~TxConnection();

    void attachLogPrinter(LogPrinter* log_printer);
    bool pollConnect();
    bool takeConnectionInfo(TxConnectionInfo& control);
    bool isConnected() const;
};
```

Internal behavior:

- on successful connect, publish `Connected { generation, sender_fd, receiver_fd }`
  or, if Stage 1 intentionally uses an interim single published fd shape, document that explicitly and keep ownership semantics clear
- on disconnect, publish `Disconnected { generation, reason/fd-invalid }`
- close unpublished handed-off fds on teardown

Stage 1 should prefer the final ownership direction, but it does not need to fully implement later-stage sender/receiver consumption yet.

## Venturi Wiring

`Venturi.cpp` should be changed only enough to make Stage 1 visible:

- construct `TxConnection`
- start a small connection lifecycle thread or loop
- repeatedly call `pollConnect()`
- do not interpret published transport-control state in `Venturi`
- do not close published transport-control fds in `Venturi`
- keep TX lifecycle state internal to the TX module boundary

No other TX modules should be instantiated in this stage.
In particular, `Venturi.cpp` should remove or temporarily omit:

- `TxSender`
- `TxReceiver`
- sender thread
- receiver thread

Stage 1 `Venturi.cpp` should compile and run with only the connection-lifecycle path present.

That means Stage 1 `Venturi.cpp` is allowed to have a temporary shape like:

```cpp
TxConnection tx_connection(...);

std::thread tx_connection_thread([&]() {
    while (true) {
        bool did_work = false;
        did_work = tx_connection.pollConnect() || did_work;
        if (!did_work) {
            std::this_thread::sleep_for(kThreadSleepTime);
        }
    }
});
```

The exact local observation container can be simple. No separate observer object is required.
Published controls remain observable through tests in Stage 1, not through local `Venturi` variables or local connection-state mirrors.

## Logging

Stage 1 `TxConnection` should not print directly from the connection thread.

Connection lifecycle issues should be emitted through the TX log path into `LogPrinter`, not through `std::printf` inside the module.

## Expected Observation

When Stage 1 is working, you should be able to observe:

- repeated connect attempts when the gateway is absent
- a `Connected` control publication when the gateway becomes reachable
- generation increments only on successful new connections
- a `Disconnected` control publication when the connection is lost
- no sender-side activity
- no receiver-side frame handling

## Acceptable Incompleteness

At the end of Stage 1, all of the following are acceptable:

- no `TxReceiver` instantiated in `Venturi`
- no `TxSender` instantiated in `Venturi`
- no login request
- no heartbeat
- no order flow
- no inbound frame processing
- no latency tracker wiring for TX path

This stage is successful if connection lifecycle and control publication are clean and observable.

## Tests For Stage 1

Stage 1 should add or keep only lifecycle-focused tests:

- successful connect publishes control
- disconnect publishes control for active generation
- destructor cleans up unpublished connected-control fds
- reconnect timing/generation behavior

Stage 1 should not yet test:

- inbound frame reading
- sender-side send flow
- feedback-to-pending transitions

Those belong to later stages.

## Stop Condition

Do not move on to Stage 2 until you can clearly answer:

1. Who owns each fd published by `TxConnection`?
2. When must an unpublished fd be closed?
3. What changes the connection generation?
4. What happens if `Connected` is published but no later module consumes it yet?
5. Why is `TxConnection` not allowed to keep `recv()` in this stage?

## Risks

### Risk: Carrying Old Responsibilities Forward

If `TxConnection` still does `recv()` or frame assembly in Stage 1, then the stage has not actually clarified the module boundary.

Mitigation:

- explicitly remove any receive-path state from Stage 1 scope

### Risk: Stage 1 Quietly Depends On Later Modules

If `Venturi.cpp` needs sender or receiver logic just to compile Stage 1, the stage boundary is wrong.

Mitigation:

- keep the `Venturi` wiring minimal and local to connection controls only

## Acceptance Criteria

Stage 1 is complete when:

1. `TxConnection` is visibly wired into `Venturi.cpp`
2. `TxConnection` only owns connection lifecycle concerns
3. `TxConnection` publishes connection controls that can be observed locally in `Venturi`
4. no sender/receiver/protocol behavior is required for the stage to run
5. Stage 1 tests cover only connection lifecycle and control publication
6. `Venturi.cpp` does not manually own TX lifecycle state or published TX fds
