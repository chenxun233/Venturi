# Memory Barriers in MMIO Programming

## Background

Modern CPUs and compilers perform optimizations that can reorder memory operations:
- **Compiler reordering**: Optimizer rearranges instructions for efficiency
- **CPU reordering**: Out-of-order execution, write buffers, store coalescing

For regular memory, this is transparent. For MMIO (Memory-Mapped I/O) to hardware devices, order matters.

## Types of Barriers

| Barrier | Scope | Effect |
|---------|-------|--------|
| `__asm__ volatile ("" ::: "memory")` | Compiler only | Prevents compiler reordering; no CPU instruction emitted |
| `__asm__ volatile ("mfence" ::: "memory")` | Compiler + CPU | Full fence - orders all loads and stores |
| `__asm__ volatile ("sfence" ::: "memory")` | Compiler + CPU | Store fence - orders stores only |
| `__asm__ volatile ("lfence" ::: "memory")` | Compiler + CPU | Load fence - orders loads only |

**"Compiler only"** means:
- Forces assembly instruction order in generated code
- Does NOT emit any CPU instruction (the `""` is empty)
- CPU can still reorder at runtime via out-of-order execution
- Use when you only need to prevent compiler optimizations, not hardware reordering

## Use Cases for PCIe/FPGA

### 1. Sequential Register Writes
```c
// Without barrier: CPU may reorder or combine these writes
write_reg32(REG_DMA_ADDR_LO, addr_lo);
write_reg32(REG_DMA_ADDR_HI, addr_hi);
write_reg32(REG_DMA_CTRL, 0x1);  // Must happen LAST
```

With `mfence` between writes, ordering is guaranteed.

### 2. Write Then Read
```c
write_reg32(REG_COMMAND, cmd);
mfence();
status = read_reg32(REG_STATUS);  // Must see effect of write
```

### 3. Polling Loops
```c
while (!(read_reg32(REG_STATUS) & DONE_BIT)) {
    // Without volatile + barrier, compiler may hoist read out of loop
}
```

## Implementation in This Project

```c
void FPGAHelloDev::write_reg32(uint32_t offset, uint32_t value) {
    __asm__ volatile ("mfence" ::: "memory");
    volatile uint32_t* reg = (volatile uint32_t*)(m_basic_para.p_bar_addr[0] + offset);
    *reg = value;
}
```

- `volatile`: Prevents compiler from caching/optimizing away the access
- `mfence`: Ensures all prior memory operations complete before this write

## When Barriers Are NOT Needed

- Single isolated register access
- Reads that don't depend on prior writes
- When using kernel APIs like `writel()`/`readl()` (include barriers internally)

## Common Pitfalls

1. **Compiler barrier only** (`"" ::: "memory"`) doesn't prevent CPU reordering
2. **volatile alone** doesn't order multiple accesses - just prevents optimization of individual access
3. **Over-fencing** hurts performance - use only where ordering matters

## Performance Side Effects

Memory barriers have significant performance costs:

### 1. Pipeline Stalls
`mfence` forces the CPU to wait until all pending memory operations complete before continuing. This can stall the pipeline for dozens to hundreds of cycles.

### 2. Disables Write Combining
The CPU normally batches multiple small writes into larger transactions. A barrier flushes the write buffer, preventing this optimization.

### 3. Serialization Point
All out-of-order execution benefits are lost at the barrier. Instructions after the barrier cannot start until prior memory ops finish.

### Typical Latency

| Barrier | Approximate Cycles (Intel) |
|---------|---------------------------|
| Compiler only (`""`) | 0 (compile-time only) |
| `lfence` | ~5-10 cycles |
| `sfence` | ~10-20 cycles |
| `mfence` | ~30-100+ cycles |

### When to Avoid

```c
// BAD: mfence before EVERY register access - unnecessary overhead
void write_reg32(uint32_t offset, uint32_t value) {
    __asm__ volatile ("mfence" ::: "memory");
    volatile uint32_t* reg = ...;
    *reg = value;
}
```

For PCIe MMIO specifically, `volatile` combined with the fact that **PCIe write ordering is preserved within the same address space** often makes `mfence` unnecessary between consecutive writes to the same device.

### Better Approach

Add barrier only where ordering across operations matters:

```c
write_reg32(REG_DMA_ADDR_LO, addr_lo);
write_reg32(REG_DMA_ADDR_HI, addr_hi);
__asm__ volatile ("mfence" ::: "memory");  // Only here, before trigger
write_reg32(REG_DMA_CTRL, 0x1);
```

## References

- Intel SDM Vol. 3, Chapter 8 (Memory Ordering)
- Linux kernel `Documentation/memory-barriers.txt`
- PCIe Base Specification - ordering rules for posted vs non-posted writes
