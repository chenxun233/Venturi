# Venturi TCP Runtime Rename and Default Address Design

## Goal

Align the runtime app names and TCP defaults for the FPGA boost demo so the two-process demo matches the intended physical setup:

- Client host NIC: `enp5s0f0` with `192.168.50.1/24`
- Server host NIC: `enp6s0f0` with `192.168.50.2/24`
- Direct link between those NICs
- Runtime apps named `Venturi` and `dummy_server`

The change must preserve the existing client TX threading model: the client TCP path remains inside its dedicated TX thread and continues receiving outbound records from `Executor`.

## Scope

Allowed source changes:

- TCP-related client/server modules in `cpp_src/FPGA_boost_demo`
- `cpp_src/CMakeLists.txt` for target renames
- Build-directory cleanup in `cpp_src/build`

Out of scope:

- Changes to FPGA RX, strategy, executor semantics, or non-TCP data flow
- Changes to test targets beyond keeping them buildable
- New configuration systems, scripts, or unrelated refactors

## Current Problem

The server currently defaults to loopback because `DummyExchangeConfig.listen_ip` is initialized to `127.0.0.1`. If the server is launched without `--listen-ip`, it binds to loopback and prints:

`dummy_exchange_server listening on 127.0.0.1:9000`

That default does not match the intended two-NIC deployment.

## Proposed Design

### 1. Runtime target names

Rename the two runtime executables in CMake:

- `test_fpga_rx_adapter` -> `Venturi`
- `dummy_exchange_server` -> `dummy_server`

No test target names change.

### 2. Server TCP defaults

Update the server default listen address in `DummyExchangeConfig`:

- `listen_ip = "192.168.50.2"`
- `port = 9000`

CLI override support remains unchanged:

- `--listen-ip`
- `--port`
- existing auth and behavior flags

This gives the correct bind target even when the server is launched with no explicit address arguments.

### 3. Client TCP defaults

Keep the client TCP defaults aligned with the physical topology:

- bind IP: `192.168.50.1`
- server IP: `192.168.50.2`
- port: `9000`

The dedicated `TxEngine` thread remains unchanged. `Executor` still pushes records into the TX path exactly as it does now.

### 4. Build artifact cleanup

After renaming and rebuilding the runtime targets, remove stale unrelated application binaries from `cpp_src/build` so the directory is centered on the intended runtime apps plus any remaining tests.

Keep:

- `Venturi`
- `dummy_server`
- test executables

Remove stale runtime-app binaries produced by the old target names or unrelated app targets if they remain in `cpp_src/build`.

## Data Flow and Behavior

The TCP behavior remains structurally unchanged:

1. `Executor` emits outbound records.
2. `TxEngine` consumes those records on its separate thread.
3. `TxEngine` binds its local socket to the client IP.
4. `TxEngine` connects to the server IP and port.
5. `dummy_server` listens on the server IP and port.
6. Linux selects the NIC based on the bound local IP addresses:
   - `192.168.50.1` routes through `enp5s0f0`
   - `192.168.50.2` routes through `enp6s0f0`

No interface-name binding is introduced.

## Error Handling

Existing error handling remains in place:

- Invalid bind IP or listen IP still fails fast.
- Bind and connect failures still print socket-level errors.
- Server login and protocol handling remain unchanged.

The main expected improvement is that an argument-less server start now binds to `192.168.50.2:9000` instead of loopback.

## Verification Plan

Verification will cover:

1. Reconfigure and rebuild the renamed runtime targets.
2. Confirm the produced runtime binaries are `Venturi` and `dummy_server`.
3. Confirm stale unrelated runtime binaries are removed from `cpp_src/build` while test binaries remain.
4. Start `dummy_server` and verify it reports `192.168.50.2:9000`.
5. Start the client-side TCP path in `Venturi` or a targeted TCP connectivity check as appropriate for the available runtime environment.
6. Confirm the TCP link is established or, if full app startup is blocked by FPGA/VFIO prerequisites, confirm the server bind and client connect path as far as the environment allows.

## Risks and Constraints

- Full end-to-end runtime verification may depend on FPGA/VFIO prerequisites outside the TCP modules.
- Renaming CMake targets may require updating any local build commands that still reference old target names.
- Cleaning `cpp_src/build` must avoid deleting test executables the user wants to keep.

## Implementation Notes

Keep the implementation minimal:

- Prefer changing default constants and target names rather than introducing new abstractions.
- Do not alter the TX thread ownership or queueing model.
- Do not touch unrelated modules outside the approved scope.
