# Incremental Venturi TX Build

This note is the quick reference version of the formal design in:

- [docs/superpowers/specs/2026-04-13-incremental-venturi-tx-build-design.md](/home/chenxun/Documents/Project/Venturi/docs/superpowers/specs/2026-04-13-incremental-venturi-tx-build-design.md)

Use this file while working through the TX path module by module.

## Rule

Build one module at a time.

After each module:

1. wire it into `Venturi.cpp`
2. observe what it does
3. explain the ownership
4. only then move on

Do not jump straight to the full final TX path.

Important:

- the current `TxConnection`, `TxReceiver`, and `TxSender` implementations are not assumed to be correct
- you may reuse those same file/class names
- but you should rewrite or delete their current logic whenever it violates the intended split

## Threading By Stage

- Stage 1: 1 TX-related thread
  - `TxConnection` only
- Stage 2: 1 TX-related thread
  - `TxConnection` + `TxReceiver`
- Stage 3 and later: 2 TX-related threads
  - receiver/control thread: `TxConnection` + `TxReceiver`
  - sender thread: `TxSender`

`TxConnection` should not become a permanent third hot-path thread.

## Stage Order

### 1. `TxConnection`

Build only:

- connect/reconnect
- socket option setup
- generation tracking
- `Connected` / `Disconnected` control publication

Observe:

- connect attempts
- successful connect publication
- disconnect publication
- generation changes

Do not require yet:

- sender
- receiver
- login
- order flow

If current `TxConnection` still owns `recv()` or protocol/session logic, that is something to remove in this stage.

Also for Stage 1:

- do not instantiate `TxSender` in `Venturi.cpp`
- do not instantiate `TxReceiver` in `Venturi.cpp`
- remove sender/receiver runtime loops from `Venturi.cpp`
- keep only the connection-control loop visible

### 2. `TxReceiver`

Build only:

- install receiver-side fd
- `recv()`
- Soup frame assembly
- read-side disconnect detection

Observe:

- receiver becomes active after connect
- inbound frames are assembled
- read-side disconnect is detected

Do not require yet:

- sender-side completion logic

Threading:

- `TxConnection` and `TxReceiver` should share the same receiver/control thread in this stage

If current `TxReceiver` is only a bridge/wrapper, this stage is where it stops being acceptable.

### 3. `TxSender`

Build only:

- install sender-side fd
- outbound encoding
- login / heartbeat send
- sender-side socket ownership

Observe:

- login frame is sent
- heartbeat path works
- sender loop is live in `Venturi.cpp`

Do not require yet:

- full feedback-driven completion

If current `TxSender` does not clearly own send-side authority, rewrite it in place.

Threading:

- this is the first stage with 2 TX-related threads
- receiver/control thread: `TxConnection` + `TxReceiver`
- sender thread: `TxSender`

### 4. Receiver To Sender Feedback

Build:

- receiver emits parsed feedback
- sender consumes it
- sender owns completion / pending cleanup / replay rebuild

Observe:

- accepts/rejects/executions change sender state
- disconnect leads to replay rebuild

### 5. Full Intent Path

Build:

- strategy/executor intents flow into sender
- sender sends
- receiver parses
- sender closes the loop

Observe:

- end-to-end order lifecycle

### 6. Failure And Recovery Validation

Validate:

- send failure
- receive failure
- disconnect before feedback
- reconnect and replay
- stale control ignored correctly

## Ownership Reminder

Desired end-state:

- `TxConnection`: pure connection lifecycle
- `TxReceiver`: true receive owner
- `TxSender`: true send owner and state owner

Important:

- `TxSender` should own pending/replay/completion state
- `TxReceiver` should not be the authority for “order is done”
- `TxReceiver` should feed feedback back into `TxSender`

## What To Write Down At Each Stage

For each stage, note:

- what module changed
- what changed in `Venturi.cpp`
- what is intentionally incomplete
- what you observed
- what ownership rule became clear
- what existing logic was removed because it violated the split pattern
