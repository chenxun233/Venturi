# rx_dma_config
On one side, `rx_dma_config` connects to [cq_parser](pcie_wrapper/CQ_parser.md) and [cc_formatter](pcie_wrapper/CC_formatter.md) to connect to the host. On the other side, it connects to [rx_dma_stage](rx_dma_stage/overview.md), guiding the DMA process.
## Introduction
The implementation is in `verilog_src/DMA_related/rx_dma_config.v`.
## Design Logic
This module saves the offset address of different registers, they are:
```verilog
localparam [BAR0_SIZE-1:0] REG_RESET                        = 16'h00;
localparam [BAR0_SIZE-1:0] REG_ID                           = 16'h04;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_SLOT_NUM_OFFSET       = 16'h08;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_ENABLE_OFFSET         = 16'h10;
localparam [BAR0_SIZE-1:0] REG_STATUS                       = 16'h0C;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_CONS_PTR_OFFSET       = 16'h18;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_PROD_OFFSET           = 16'h20;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_DROP_OFFSET           = 16'h28;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_STAT_OFFSET           = 16'h30;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_BASE0                 = 16'h40;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_STRIDE                = 16'h40;
```

- `REG_RESET (0x00)`: Write any value to generate a one-cycle `o_reg_reset` pulse and clear the stored queue base address, slot count, and consumer pointer state.
- `REG_ID (0x04)`: Read-only module identifier register; returns `64'h4d5f52585f434647`.
- `REG_RX_QUE_SLOT_NUM_OFFSET (0x08)`: Per-queue offset for the slot-count register, which software writes to define the host ring depth; together with the consumer pointer, the NIC uses it to determine whether the queue is full and whether another DMA write can be issued.
- `REG_RX_QUE_ENABLE_OFFSET (0x10)`: Per-queue offset for the derived enable bit; it reads as `1` only when both the queue base address and slot count are non-zero.
- `REG_STATUS (0x0C)`: Global status register; currently it always reads back `0`. (Not used now)
- `REG_RX_QUE_CONS_PTR_OFFSET (0x18)`: Per-queue offset for the **consumer pointer** register written by software to indicate which descriptors have been consumed.
- `REG_RX_QUE_PROD_OFFSET (0x20)`: Per-queue offset for the **producer pointer** register reported by `rx_dma_stage` on reads.
- `REG_RX_QUE_DROP_OFFSET (0x28)`: Per-queue offset for the **drop counter** reported by `rx_dma_stage` on reads.
- `REG_RX_QUE_STAT_OFFSET (0x30)`: Per-queue offset for the queue status word reported by `rx_dma_stage` on reads. The current bit layout is bit 0 `enabled`, bit 1 `full`, and bit 2 `busy`.
- `REG_RX_QUE_BASE0 (0x40)`: Base address of queue 0's register window; writing here stores the descriptor ring base address for that queue.
- `REG_RX_QUE_STRIDE (0x40)`: Spacing between queue register windows, so queue `n` starts at `0x40 + n * 0x40`.

## How to coordinate with the host
This module is for MMIO operation, which means the host is always the initiator. As this module is for control purpose, host can do **write** and **read** to the registers in this module

Below gives two example on **write** and **read**, respectively.

![write and read examples](../figures/FPGA/top%20level/rx_dma_config%20timing.png)

One has to note that different from the general NIC operation, which takes two steps, there is only one step to start to DMA write in this design

In regular NIC, the host fills the descriptors, sends the IOVA of the descriptors to the NIC, from which the NIC takes out the starting address of the real packets and start to read/write.

In this NIC, the host directly write the starting address with MMIO to the NIC. This is due to the packet size is fixed to 256-bit.

## Connection with [rx_dma_stage](rx_dma_stage/overview.md)
Not all the registers acting with the host are in this module. Some of them are in [rx_dma_stage](rx_dma_stage/overview.md), as they are directly related to DMA operation. Below shows the main registers in the two modules. Among them,
`reg_que_base`, `reg_que_slot_num` and `reg_que_cons` are in **rx_dma_config**, as they are directly configured by the host through MMIO. Others are in [rx_dma_stage](rx_dma_stage/overview.md), since they vary in the process of DMA writing.

![The main registers in the two modules](../figures/FPGA/top%20level/connection%20with%20rx_dma_stage.png)

### Pointers
Everytime there is a packet having been sent to the host via DMA, `que_prod_ptr` will `+1`. Similarly, once the host has taken out the packet, it has to **write back** the current `reg_que_cons` through MMIO to notify the NIC (otherwise the NIC does not know the whether the memory pool is full or not, keep writing can overwrite the previous packets in the host). They are monotonic registers, with 64-bit width, widely enough for packets. Once they meet, it indicates the memory pool in the host is full, NIC starts to drop packets and increase `que_drop_count`.

### Address
`que_slot_index` is for calculating the DMA writing address, together with `i_que_base_addr` and `i_que_slot_num`. Because there is a boundary in the memory pool, `que_slot_index` will be wrapped to `0` once it hits `i_que_slot_num`. Below illustrates this behavior.

![slot_index_wrap](../figures/FPGA/top%20level/slot_index%20wrap%20up.png)


## Reset Register
The block also exports `o_reg_reset` (not shown in the figure), a software-triggered reset pulse used by `rx_dma_stage` to clear queue runtime state such as producer pointers and drop counters.

## Notes
- The enable register is currently derived status, not an independently stored control bit.
- Runtime counters live in `rx_dma_stage`; `rx_dma_config` only exposes them on MMIO reads.
