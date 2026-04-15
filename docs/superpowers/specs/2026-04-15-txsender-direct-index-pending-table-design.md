# TxSender Direct-Index Pending Table Design

## Summary

Replace `TxSender` pending-order lookup from `std::unordered_map<uint32_t, TxOutboundRecord>` plus `ordered_tags` with a preallocated direct-index table.

The slot index is computed by:

```cpp
slot = user_ref_num & (slot_count - 1);
```

`slot_count` is mandatory power-of-two. This design intentionally chooses the simplest and fastest lookup path, and accepts collision-based rejection as the overflow behavior for slot aliasing.

## Goals

- Remove hash-table overhead from the `TxSender` hot path.
- Remove allocator traffic from pending-order insertion and erase.
- Preserve O(1) response lookup by `user_ref_num`.
- Keep `user_ref_num` as the authoritative pending key.
- Keep producer-side latency instrumentation valid for pending-state bookkeeping.

## Non-Goals

- No multi-probe or open-addressing logic.
- No background thread for pending-order ownership.
- No attempt to preserve acceptance of every new order under slot collisions.
- No redesign of transport send queues in this change.

## Current Problem

Current pending insertion is dominated by:

```cpp
m_pending_orders.order_records[record.user_ref_num] = record;
```

This is expensive because it combines:

- hash computation
- bucket lookup
- pointer chasing
- possible node allocation
- copy of a full `TxOutboundRecord`

This showed up directly in latency tracking as the largest pending-order substage.

## Selected Approach

### Data Structure

Replace the pending map with a fixed slot table:

- `slot_count` is a power of two
- each slot stores one pending entry
- slot lookup uses `user_ref_num & (slot_count - 1)`

Each slot stores:

- `bool occupied`
- `uint32_t user_ref_num`
- pending metadata required for:
  - resend/rebuild after reconnect
  - accepted/rejected/executed handling
  - tx logging

The stored payload remains whatever `TxSender` needs for resend and response handling. This design does not require slimming the record in the same change, although that can remain a later optimization.

### Capacity Rules

Two limits exist:

- `pending_capacity`: maximum number of live pending orders allowed by policy
- `slot_count`: storage size for direct-index lookup

Rules:

- `pending_capacity` must be non-zero
- `slot_count` must be power-of-two
- `slot_count` should be much larger than `pending_capacity`
- insertion is rejected if live pending count has reached `pending_capacity`
- insertion is also rejected if the computed slot is already occupied by a different live `user_ref_num`

This means collision handling is explicit rejection, not probing.

### Overflow / Collision Policy

When a new order cannot be inserted:

- do not evict an older pending order
- do not overwrite an occupied slot
- reject the new order locally

The rationale:

- preserves correctness of already-pending orders
- avoids expensive hot-path eviction work
- keeps semantics predictable

## Behavioral Design

### Insert Path

When recording a pending order:

1. If live pending count is already `>= pending_capacity`, reject new order.
2. Compute `slot = user_ref_num & (slot_count - 1)`.
3. If the slot is occupied by another `user_ref_num`, reject new order as a collision.
4. Store the pending record in the slot.
5. Increment live pending count.

### Lookup Path

For accepted/rejected/executed responses:

1. Compute the same slot from `user_ref_num`.
2. Check `occupied`.
3. Verify stored `user_ref_num` matches.
4. If it matches, use the stored pending record.
5. If not, treat as missing pending record.

### Erase Path

On accepted/rejected/executed completion:

1. Compute slot from `user_ref_num`.
2. Verify the slot is occupied and tag matches.
3. Clear the slot.
4. Decrement live pending count.

### Reconnect / Rebuild

`_rebuildBlockedRecords()` can no longer rely on `ordered_tags`.

New approach:

1. Scan the whole slot table.
2. Collect occupied entries into a temporary array/vector.
3. Sort collected entries by `user_ref_num`.
4. Rebuild `m_blocked_outbound` in ascending `user_ref_num` order.

This keeps resend ordering stable while moving the heavier work to a cold path.

## Interface Changes

### TxSenderConfig

Add a new configuration field:

- `pending_slot_count`

Requirements:

- mandatory power-of-two
- validated during `TxSender` construction

Recommended runtime value for current app:

- `pending_capacity = 1024`
- `pending_slot_count = 65536`

### TxSender Internal State

Replace:

- `std::unordered_map<uint32_t, TxOutboundRecord> order_records`
- `std::vector<uint32_t> ordered_tags`

With something like:

- `std::vector<PendingSlot> slots`
- `std::size_t live_count`
- `std::size_t slot_mask`

`slot_mask` is `slot_count - 1`.

## Latency Tracking Impact

Pending latency stage meaning remains:

- `PENDING_CAPACITY_HANDLED`
- `PENDING_TAG_RECORDED`
- `PENDING_RECORDED`

But their implementation intent changes:

- `PENDING_CAPACITY_HANDLED`: after capacity and collision checks complete
- `PENDING_TAG_RECORDED`: after whatever lightweight bookkeeping remains before final slot write
- `PENDING_RECORDED`: after slot record write completes

If the new structure removes one of these meaningful boundaries, the stages should be collapsed rather than kept artificially.

## Error Handling

### Power-of-Two Validation

Construction must fail fast if:

- `pending_slot_count == 0`
- `pending_slot_count` is not power-of-two

### Collision Handling

If a different live tag already occupies the masked slot:

- reject the new order
- do not disturb the old slot
- emit a tx-side rejection/drop log so the event is visible

### Missing Pending Response

If an exchange response arrives and the slot is empty or tag-mismatched:

- treat it as missing pending state
- do not crash
- preserve current defensive behavior

## Testing

### Unit Tests

- constructor rejects non-power-of-two `pending_slot_count`
- insert succeeds with empty slot
- lookup succeeds for matching `user_ref_num`
- erase clears matching slot
- insert rejects when live pending count reaches `pending_capacity`
- insert rejects when masked slot collides with another live tag
- accepted/rejected/executed still resolve correct pending entry
- rebuild after reconnect preserves ascending `user_ref_num` resend order

### Regression Tests

- existing tx sender tests that rely on pending handling remain green
- latency tracker tests still pass for pending-order stages
- `Venturi` still builds with explicit `pending_slot_count`

## Tradeoffs

### Advantages

- fastest possible lookup path in this direction
- no hashing
- no node allocation
- contiguous memory
- simple masking logic

### Costs

- correctness depends on collision rejection policy instead of collision resolution
- larger memory footprint because `slot_count` is intentionally oversized
- reconnect rebuild requires scanning the entire slot table

## Open Decisions Already Resolved

- Use direct indexing instead of hash lookup: yes
- Use `& (slot_count - 1)` instead of `%`: yes
- Require power-of-two slot count: yes
- Reject new order instead of evicting old pending order: yes
- Handle collisions by rejecting new order instead of probing: yes

## Implementation Notes

- Prefer a dedicated `PendingSlot` / `PendingRecord` type instead of reusing map-oriented structures.
- Keep slot validation branchless where reasonable, but do not sacrifice clarity in the first version.
- The rebuild path is cold; optimize for correctness first there.
