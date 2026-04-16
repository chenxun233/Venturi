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
  - `TxConnection` + `TxSender`
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
- keep only the connection lifecycle loop visible
- do not store TX lifecycle state in local `Venturi` variables
- do not close published TX fds in `Venturi`
- push connection issues to `LogPrinter`, not `printf` from the connection thread

### 2. `TxSender`

Build only:

- install sender-side fd
- outbound encoding
- send login immediately after fd install
- login / heartbeat send
- sender-side socket ownership
- consume connection controls from its own `SpscRingQueue<TxConnectionInfo>`

Observe:

- login is sent as soon as sender gets the updated fd
- login frame is sent
- heartbeat path works
- sender loop is live in `Venturi.cpp`

Do not require yet:

- full feedback-driven completion

Threading:

- this stage still uses 1 TX-related thread
- `TxConnection` and `TxSender` may share that thread temporarily
- `TxConnection -> TxSender` should use one dedicated `SpscRingQueue<TxConnectionInfo>`
- `TxSender` should own that queue
- `TxConnection` should publish through the sender-facing push API

If current `TxSender` does not clearly own send-side authority, rewrite it in place.

### 3. `TxReceiver`

Build only:

- install receiver-side fd
- `recv()`
- Soup frame assembly
- read-side disconnect detection
- consume connection controls from its own `SpscRingQueue<TxConnectionInfo>`
- receive the login/session info established by sender in Stage 2
- send login-accepted/session-established info back to `TxSender`

Observe:

- receiver becomes active after connect
- receiver has the login/session context needed for live receive-side parsing
- inbound frames are assembled
- read-side disconnect is detected
- receiver pushes login-accepted/session-established info back to sender
- sender does not release outbound order frames until that feedback arrives

Do not require yet:

- sender-side completion logic

If current `TxReceiver` is only a bridge/wrapper, this stage is where it stops being acceptable.

Threading:

- this is now the first stage with 2 TX-related threads
- receiver/control thread: `TxConnection` + `TxReceiver`
- sender thread: `TxSender`
- `TxConnection -> TxReceiver` should use one dedicated `SpscRingQueue<TxConnectionInfo>`
- `TxReceiver` should own that queue
- `TxConnection` should publish through the receiver-facing push API
- `TxReceiver -> TxConnection` should use one dedicated `SpscRingQueue<TxDisconnectNotice>`
- do not use one shared multi-consumer queue

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
