# XGMII and Ethernet Frame Endianness

Notes on byte order for the MAC layer: XGMII bus, Ethernet frame semantics, and why a byte reversal may be used when presenting frame data (e.g. for an order-book parser).

---

## 1. Ethernet frame: big-endian (network byte order)

The **Ethernet frame itself** uses **big-endian** for all multi-byte fields:

- **Multi-byte integers** (EtherType, MAC addresses, IP addresses, port numbers, length fields) are sent with the **most significant byte first** (earlier in the byte stream).
- Example: EtherType `0x0800` is sent as byte `0x08` first, then `0x00` → MSB first = big-endian.

So the **frame content** is big-endian; the first byte of a 16/32-bit value is its MSB.

---

## 2. XGMII bus: “little-endian” layout only

**XGMII is little-endian** only in this sense:

- On the **64-bit data bus**, the **first byte in time** (first byte of the frame in that beat) is placed in the **LSB** of the bus: **lane 0** = `xgmii_rxd[7:0]`.
- Lane 1 = `[15:8]`, lane 2 = `[23:16]`, … lane 7 = `[63:56]`.
- So: **first byte in time → LSB** = “little-endian” **mapping of the byte stream onto the 64-bit word**.

The **order and meaning of the bytes** (frame content) are **unchanged**. The frame is still big-endian; XGMII just carries that same byte stream with “first byte” at the LSB of the bus. There is **no conversion** of the frame to little-endian—only a **layout** choice (first in time → LSB).

---

## 3. XGMII lane order = Ethernet frame order

Within each 64-bit beat, **XGMII lane order is the same as the order of bytes in the Ethernet frame** (time order):

- Lane 0 = first byte of that beat in the frame  
- Lane 1 = second byte  
- …  
- Lane 7 = eighth byte  

If START is in lane 0, then lane 0 is the control character and **lane 1** is the first byte of the frame for that beat. So **XGMII lane order = order of bytes in the Ethernet frame**.

---

## 4. Why reverse when building a “frame” word (e.g. for parser)?

- **XGMII**: First frame byte (in time) sits at **LSB** of the 64-bit word (lane 0 or first data lane).
- **AXI / network convention**: Often we want “first byte of frame at **MSB**” of the 64-bit word (big-endian presentation) so that e.g. `frame_content[63:56]` = first byte, `[55:48]` = second, etc., for easier header parsing.

So a **byte reversal** in the parser (first byte → `[63:56]`, last of the beat → `[15:8]`) converts from “XGMII layout” (first at LSB) to “first byte at MSB” (big-endian-style word). That is a **presentation** choice for your logic, not a change to the Ethernet frame’s own byte order.

---

## 5. Why Corundum MAC does not reverse

In Corundum’s `axis_xgmii_rx_64`, the MAC **keeps XGMII lane order** (no byte reversal):

- Same 64-bit value is used for CRC and for `m_axis_tdata`.
- First data byte (e.g. when START in lane 0) stays at `[15:8]` (lane 1), not at `[63:56]`.
- Reasons: simpler datapath, no extra mux for swap, and the rest of the Corundum stack (mqnic, software) is written to expect that convention.

So: **AXI Stream does not mandate byte order**. Corundum chose “lane order”; a parser that wants “first byte at MSB” does a reverse in its own interface.

---

## 6. Summary table

| What | Endianness / convention |
|------|--------------------------|
| **Ethernet frame** (multi-byte fields) | **Big-endian** (network byte order) |
| **XGMII bus** (where first byte sits in the 64b word) | **Little-endian** (first byte in time at LSB) |
| **XGMII lane order** | Same as **Ethernet frame byte order** (time order) |
| **Parser “frame_content” (first byte at MSB)** | Byte reversal from XGMII layout → big-endian-style word |

---

## 7. References

- Order book parser: `verilog_src/orderbook_parser/orderbook_parser.v` (SOF/EOF detection, byte mapping for START in lane 0 / lane 4).
- Corundum MAC RX: `corundum/corundum_exanic_x10/src_verilog/axis_xgmii_rx_64.v` (keeps lane order, no reverse).
