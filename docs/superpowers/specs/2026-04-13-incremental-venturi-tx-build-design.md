# Incremental Venturi TX Build Design

## Goal

Build the TX path incrementally in `Venturi.cpp`, one module at a time, so each stage is:

- wired into the real runtime
- runnable even if incomplete
- easy to inspect and understand before moving on

The target architecture is still a split TX design, but the learning/build sequence is staged rather than delivered all at once.

## Core Principle

Do not build isolated modules off to the side and only integrate them at the end.

Instead:

1. build one module
2. wire it into `Venturi.cpp` immediately
3. observe the expected behavior
4. stop and verify understanding
5. only then move to the next module

This design is intentionally optimized for learning and observability, not just shortest implementation path.

## Desired End-State

The final TX path should converge toward these responsibilities:

- `TxConnection`
  - pure connection lifecycle module
  - connect/reconnect only
  - publishes fresh fds and disconnect events
  - does not own `send()`
  - does not own `recv()`
  - does not own protocol/session/pending state

- `TxSender`
  - owns sender-side fd
  - owns sending
  - owns protocol/session state
  - owns pending/replay state
  - marks requests done when feedback arrives

- `TxReceiver`
  - owns receiver-side fd
  - owns receiving
  - owns Soup frame assembly and feedback parsing
  - forwards parsed feedback to `TxSender`

- `Venturi.cpp`
  - wires these modules incrementally, stage by stage

## Non-Goals

- Do not require the whole final TX path to exist before anything is visible in `Venturi`.
- Do not hide incomplete wiring behind fake “finished” module boundaries.
- Do not optimize for maximum short-term code throughput at the expense of understandability.

## Incremental Build Strategy

### Stage 1: `TxConnection` Wired Into `Venturi`

#### Responsibility

At this stage, `TxConnection` should represent only transport lifecycle:

- create socket
- connect
- configure socket options
- reconnect after failure
- publish `Connected` / `Disconnected` controls
- publish fresh connection generation

It should not yet own:

- `send()`
- `recv()`
- inbound frame assembly
- login/session logic
- pending-order logic

#### Venturi Wiring

`Venturi.cpp` should:

- construct `TxConnection`
- run a small connection/control loop or thread
- observe connection controls in real runtime

No sender or receiver module is required yet.

#### Expected Observation

You should be able to observe:

- repeated connect attempts when server is unavailable
- one `Connected` publication when server becomes reachable
- generation increments only on successful new connection
- `Disconnected` publication on connection loss

#### Acceptable Incompleteness

At this stage it is acceptable that:

- no orders are sent
- no inbound application messages are processed
- latency tracking is not connected to TX path

#### Stop Condition

Do not move on until you can explain:

- who owns each newly published fd
- when a published fd must be closed
- how reconnect changes generation
- what happens if connection is published but not yet consumed

### Stage 2: `TxReceiver` Wired Into `Venturi`

#### Responsibility

Now introduce `TxReceiver` as the true receive-side owner:

- install receiver-side fd from connection control
- call `recv()`
- assemble Soup frames
- detect read-side disconnect
- produce parsed receive-side outputs

At this stage `TxReceiver` still should not own sender/session/pending state.

#### Venturi Wiring

`Venturi.cpp` should:

- create `TxReceiver`
- pass connection controls to it
- run a receiver loop or thread
- route received outputs to a temporary debug sink if `TxSender` is not wired yet

#### Expected Observation

You should be able to observe:

- receiver becomes active only after `Connected`
- inbound Soup frames are assembled correctly
- receiver-side disconnect is detected from read path
- `Venturi` visibly contains a real receiver loop, not just a stub

#### Acceptable Incompleteness

At this stage it is acceptable that:

- there is still no sender
- feedback is only logged or buffered temporarily
- no replay or pending-order state exists yet

#### Stop Condition

Do not move on until you can explain:

- where inbound partial-frame state lives
- why that state belongs to receiver, not connection
- how a read failure turns into a disconnect signal

### Stage 3: `TxSender` Wired Into `Venturi`

#### Responsibility

Now introduce `TxSender` as the send-side owner:

- install sender-side fd from connection control
- encode outbound Soup/OUCH messages
- allocate identifiers
- send test/login/heartbeat traffic
- own sender-side send helper

At this stage it can still be incomplete on feedback-driven completion.

#### Venturi Wiring

`Venturi.cpp` should:

- create `TxSender`
- route sender-side connection controls into it
- run sender loop or sender-owned processing in the actual runtime

#### Expected Observation

You should be able to observe:

- login request sent after connect
- heartbeats sent on schedule
- sender-side `send()` path active

#### Acceptable Incompleteness

At this stage it is acceptable that:

- feedback processing is incomplete or stubbed
- pending/replay may still be partial

#### Stop Condition

Do not move on until you can explain:

- why sender owns `send()`
- why sender owns `UserRefNum` allocation
- why sender should be the authority on pending state

### Stage 4: `TxReceiver -> TxSender` Feedback Wiring

#### Responsibility

Now connect receive-side parsed feedback back into sender-side state.

`TxReceiver` should emit parsed feedback events.

`TxSender` should consume them and:

- mark pending orders done
- handle accepts/rejects/executions/cancels
- rebuild replay state on matching disconnect

#### Venturi Wiring

`Venturi.cpp` should now wire:

- receiver outputs -> sender feedback ingress

This is the first stage where end-to-end order lifecycle begins to exist.

#### Expected Observation

You should be able to observe:

- pending entries disappear on terminal feedback
- replay candidates rebuild on disconnect
- reconnect leads to resumption/recovery rather than silent loss

#### Stop Condition

Do not move on until you can explain:

- why sender, not receiver, owns completion state
- how disconnect generation protects against stale feedback/control

### Stage 5: Full Intent Path In `Venturi`

#### Responsibility

Now wire the actual strategy/executor intent flow into `TxSender`.

At this point:

- upstream intents become outbound records
- sender sends them
- receiver parses feedback
- sender closes the loop on pending/replay/state

#### Venturi Wiring

`Venturi.cpp` should contain the real staged TX runtime:

- intent source
- sender loop
- receiver loop
- connection/control integration

#### Expected Observation

You should be able to observe:

- end-to-end order lifecycle
- reconnect plus replay behavior
- login/heartbeat/order flow in one runtime

### Stage 6: Final Failure And Recovery Validation

#### Responsibility

This stage is about proving behavior, not adding new architecture.

#### Expected Observation

You should validate:

- connect failure
- send failure
- receive failure
- disconnect before feedback
- disconnect after feedback
- reconnect with replay
- stale disconnect/control ignored correctly

## What Each Stage Should Record

Every stage should explicitly record:

- what module was added or changed
- what `Venturi.cpp` wiring changed
- what was intentionally still missing
- what you observed when running or testing it
- what design questions became clearer after the stage

## Recommended Learning Order

The recommended order is:

1. `TxConnection`
2. `TxReceiver`
3. `TxSender`
4. feedback wiring
5. full intent path
6. failure/recovery audit

This order is chosen because it teaches:

- connection lifecycle first
- receive-side state placement second
- send-side authority third
- sender-owned completion semantics fourth

## Why This Design Is Good

This incremental design is good for learning because:

- every stage is visible in `Venturi`
- incomplete behavior is explicit rather than hidden
- module ownership becomes understandable through observation
- you can stop after each stage and reason about the actual runtime, not a hypothetical final diagram

It is also good engineering practice because:

- each stage has a bounded purpose
- later stages build on already-observed behavior
- failure/recovery reasoning is deferred until the necessary ownership boundaries already exist

## Acceptance Criteria

This staged design is correct when:

1. each stage can be explained independently
2. each stage is visibly wired into `Venturi.cpp`
3. each stage has explicit expected observations
4. no stage silently depends on hidden final-stage assumptions
5. the final full TX path emerges by composition of understood modules rather than one opaque refactor
