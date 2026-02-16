# UltraScale GT Wizard IP Core — Port Reference

Incremental notes on the GT Wizard UltraScale (e.g. v1.7) IP core ports used in this project.  
Add more ports below as needed.

---

## Relationship: `rxresetdone_out`, `rxpmaresetdone_out`, `rxprgdivresetdone_out`

These three outputs are **receiver reset-done status** from the GT primitive. They assert in order as the RX path comes out of reset:

| Signal | What it indicates | Order |
|--------|-------------------|--------|
| **rxpmaresetdone_out** | PMA (Physical Medium Attachment) has completed reset: analog front-end, CDR, etc. | 1st |
| **rxprgdivresetdone_out** | RX programmable clock divider has completed reset (divides recovered clock to produce RXUSRCLK). | 2nd |
| **rxresetdone_out** | Full receiver path (PMA + PCS/digital) has completed reset. | 3rd |

**How the IP uses them:**

- **Releasing RX user clock:** `gtwiz_userclk_rx_reset` is de-asserted when **rxpmaresetdone** and **rxprgdivresetdone** are both high. So the RX user clock is considered valid once the PMA and the divider are ready; **rxresetdone** is not used here.
- **Declaring “RX init complete”:** The reset helper block asserts **gtwiz_reset_rx_done_out** only after **rxresetdone** (and its timer) is satisfied. So “RX reset done” to the user is gated by the full **rxresetdone** signal.

In short: **rxpmaresetdone** → **rxprgdivresetdone** → **rxresetdone**. Use PMA + prgdiv for RX user-clock stability; use **rxresetdone** for “receiver fully ready.”

**How they work with `gtwiz_reset_rx_done_in`:**

- **gtwiz_reset_rx_done_in** is an **input** to the GT Wizard IP. It is the “RX init complete” flag that the *user* (or the example design) drives into the IP.
- In the example design, the **reset helper block** drives **gtwiz_reset_rx_done_out** based on **rxresetdone** (and its FSM/timer). That output is then wired to the IP’s **gtwiz_reset_rx_done_in**. So the chain is: **rxresetdone_out** → reset FSM → **gtwiz_reset_rx_done_out** → **gtwiz_reset_rx_done_in**.
- **rxpmaresetdone** and **rxprgdivresetdone** do **not** drive `gtwiz_reset_rx_done_in`; they are used only for **gtwiz_userclk_rx_reset** (releasing the RX user clock). Only **rxresetdone** is used by the reset block to assert the “RX done” signal that becomes **gtwiz_reset_rx_done_in**.
- Inside the IP, **gtwiz_reset_rx_done_in** is used as the “reset done” input to the **RX buffer bypass** logic (`gtwiz_buffbypass_rx_resetdone_in`), so the bypass FSM knows when RX reset is complete. When the IP is generated without an internal reset controller, **gtwiz_reset_rx_done_out** of the IP is a pass-through of **gtwiz_reset_rx_done_in**.

So: the three primitive outputs feed two separate uses — **rxpma + rxprgdiv** for user clock release, **rxresetdone** for the “RX done” flag that you feed back as **gtwiz_reset_rx_done_in**.

---

## Status / clock-related outputs

### `rxpmaresetdone_out`

| Property | Description |
|----------|-------------|
| **Direction** | Output |
| **Width** | `[0:0]` (1 bit per channel) |
| **Primitive pin** | `RXPMARESETDONE` |

**Meaning:** PMA (Physical Medium Attachment) block has completed reset — analog front-end, CDR, etc. First of the three RX reset-done signals to assert. Used together with `rxprgdivresetdone_out` to release `gtwiz_userclk_rx_reset`.

---

### `rxprgdivresetdone_out`

| Property | Description |
|----------|-------------|
| **Direction** | Output |
| **Width** | `[0:0]` (1 bit per channel in multi-channel configs) |
| **Primitive pin** | `RXPRGDIVRESETDONE` |

**Meaning:** Indicates that the **receiver programmable clock divider** in the GT has completed reset and is stable.  
(RX **prg**rammable **div**ider **reset done**.)

**Typical use:** The example design (and our `SFP_wrapper`) use it to hold the RX user clock in reset until both the PMA and the programmable divider are ready:

```verilog
assign gtwiz_userclk_rx_reset_int = ~(&rxprgdivresetdone_int && &rxpmaresetdone_int);
```

Only when **both** `rxpmaresetdone` and `rxprgdivresetdone` are high is `gtwiz_userclk_rx_reset` de-asserted, so the RX user clock and data path are safe to use.

**Reference:** PG182 (GT Wizard UltraScale LogiCORE IP Product Guide); transceiver User Guides (e.g. UG576 for GTH) for the primitive.

---

### `rxresetdone_out`

| Property | Description |
|----------|-------------|
| **Direction** | Output |
| **Width** | `[0:0]` (1 bit per channel) |
| **Primitive pin** | `RXRESETDONE` |

**Meaning:** Full receiver path (PMA + PCS/digital) has completed reset. Last of the three RX reset-done signals to assert. The GT Wizard reset helper uses it to assert **gtwiz_reset_rx_done_out** — i.e. “RX init complete” is gated by `rxresetdone`, not by PMA/prgdiv alone.

---

## Reset handshake (input)

### `gtwiz_reset_rx_done_in`

| Property | Description |
|----------|-------------|
| **Direction** | Input |
| **Width** | `[0:0]` |

**Meaning:** User-driven “RX initialization complete” flag. The IP does not derive this internally when the reset controller is in the example design; the example connects **gtwiz_reset_rx_done_out** (from the reset helper, which is driven by **rxresetdone_out**) to this port. So this input should be asserted only after **rxresetdone** (and any FSM/timer) indicates full RX path ready.

**Use inside the IP:** Drives the RX buffer bypass block’s reset-done input (`gtwiz_buffbypass_rx_resetdone_in`). When the IP has no internal reset controller, **gtwiz_reset_rx_done_out** from the IP is a pass-through of this input.

---

*Other ports to be added incrementally.*
