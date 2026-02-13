# Exanic X10 Evaluation Board Pin Constraints

This is a reference of the FPGA pin/IO constraints for the Exanic X10 evaluation board, extracted from `xdc_files/fpga_k35.xdc`.
It is intended to let you build a top-level design without opening the board schematic.

Part: `xcku035-fbva676-2-e` (from the XDC header comment).

## Quick Start (Vivado)

- Add `xdc_files/fpga_k35.xdc` to your project (Project Settings -> Constraints -> Add Sources).
- Match your top-level HDL port names to the `Port` names below; Vivado applies LOCs by exact name.
- `_n` signals are active-low; `_p`/`_n` pairs are differential and must be kept together.
- Use the listed `IOSTANDARD`, `SLEW`, and `DRIVE` values as shown; most GPIO are LVCMOS18.
- MGT pins (SFP/PCIe lanes and refclks) do not use IOSTANDARD; they are transceiver pins.

## Interface Pin Tables

### System Clock

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `clk_100mhz_p` | `D18` | IOSTANDARD=LVDS | 100 MHz system clock (LVDS) positive leg |
| `clk_100mhz_n` | `C18` | IOSTANDARD=LVDS | 100 MHz system clock (LVDS) negative leg |

### SFP+ MGT Lanes

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `sfp_1_rx_p` | `D2` | — | SFP1 MGT receive lane (input from module) positive leg |
| `sfp_1_rx_n` | `D1` | — | SFP1 MGT receive lane (input from module) negative leg |
| `sfp_1_tx_p` | `E4` | — | SFP1 MGT transmit lane (output to module) positive leg |
| `sfp_1_tx_n` | `E3` | — | SFP1 MGT transmit lane (output to module) negative leg |
| `sfp_2_rx_p` | `C4` | — | SFP2 MGT receive lane (input from module) positive leg |
| `sfp_2_rx_n` | `C3` | — | SFP2 MGT receive lane (input from module) negative leg |
| `sfp_2_tx_p` | `D6` | — | SFP2 MGT transmit lane (output to module) positive leg |
| `sfp_2_tx_n` | `D5` | — | SFP2 MGT transmit lane (output to module) negative leg |
| `sfp_mgt_refclk_p` | `H6` | — | SFP MGT reference clock 161.1328125 MHz positive leg |
| `sfp_mgt_refclk_n` | `H5` | — | SFP MGT reference clock 161.1328125 MHz negative leg |

### SFP Control and I2C

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `sfp_1_tx_disable` | `AA12` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP1 transmitter disable control (output to module) |
| `sfp_2_tx_disable` | `W14` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP2 transmitter disable control (output to module) |
| `sfp_1_npres` | `C24` | IOSTANDARD=LVCMOS18, PULLUP=true | SFP1 module not-present status (input, active low) |
| `sfp_2_npres` | `D24` | IOSTANDARD=LVCMOS18, PULLUP=true | SFP2 module not-present status (input, active low) |
| `sfp_1_los` | `W13` | IOSTANDARD=LVCMOS18, PULLUP=true | SFP1 loss-of-signal status (input) |
| `sfp_2_los` | `AB12` | IOSTANDARD=LVCMOS18, PULLUP=true | SFP2 loss-of-signal status (input) |
| `sfp_1_rs` | `B25` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP1 rate-select/control (output; module-dependent) |
| `sfp_2_rs` | `D25` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP2 rate-select/control (output; module-dependent) |
| `sfp_i2c_scl` | `W11` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12, PULLUP=true | SFP management I2C clock (shared) |
| `sfp_1_i2c_sda` | `Y11` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12, PULLUP=true | SFP1 management I2C data |
| `sfp_2_i2c_sda` | `Y13` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12, PULLUP=true | SFP2 management I2C data |

### PCIe Gen3 x8 (MGT Lanes)

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `pcie_rx_p[0]` | `P2` | — | PCIe lane 0 receive (from host to FPGA) positive leg |
| `pcie_rx_p[1]` | `T2` | — | PCIe lane 1 receive (from host to FPGA) positive leg |
| `pcie_rx_p[2]` | `V2` | — | PCIe lane 2 receive (from host to FPGA) positive leg |
| `pcie_rx_p[3]` | `Y2` | — | PCIe lane 3 receive (from host to FPGA) positive leg |
| `pcie_rx_p[4]` | `AB2` | — | PCIe lane 4 receive (from host to FPGA) positive leg |
| `pcie_rx_p[5]` | `AD2` | — | PCIe lane 5 receive (from host to FPGA) positive leg |
| `pcie_rx_p[6]` | `AE4` | — | PCIe lane 6 receive (from host to FPGA) positive leg |
| `pcie_rx_p[7]` | `AF2` | — | PCIe lane 7 receive (from host to FPGA) positive leg |
| `pcie_rx_n[0]` | `P1` | — | PCIe lane 0 receive (from host to FPGA) negative leg |
| `pcie_rx_n[1]` | `T1` | — | PCIe lane 1 receive (from host to FPGA) negative leg |
| `pcie_rx_n[2]` | `V1` | — | PCIe lane 2 receive (from host to FPGA) negative leg |
| `pcie_rx_n[3]` | `Y1` | — | PCIe lane 3 receive (from host to FPGA) negative leg |
| `pcie_rx_n[4]` | `AB1` | — | PCIe lane 4 receive (from host to FPGA) negative leg |
| `pcie_rx_n[5]` | `AD1` | — | PCIe lane 5 receive (from host to FPGA) negative leg |
| `pcie_rx_n[6]` | `AE3` | — | PCIe lane 6 receive (from host to FPGA) negative leg |
| `pcie_rx_n[7]` | `AF1` | — | PCIe lane 7 receive (from host to FPGA) negative leg |
| `pcie_tx_p[0]` | `R4` | — | PCIe lane 0 transmit (from FPGA to host) positive leg |
| `pcie_tx_p[1]` | `U4` | — | PCIe lane 1 transmit (from FPGA to host) positive leg |
| `pcie_tx_p[2]` | `W4` | — | PCIe lane 2 transmit (from FPGA to host) positive leg |
| `pcie_tx_p[3]` | `AA4` | — | PCIe lane 3 transmit (from FPGA to host) positive leg |
| `pcie_tx_p[4]` | `AB6` | — | PCIe lane 4 transmit (from FPGA to host) positive leg |
| `pcie_tx_p[5]` | `AC4` | — | PCIe lane 5 transmit (from FPGA to host) positive leg |
| `pcie_tx_p[6]` | `AD6` | — | PCIe lane 6 transmit (from FPGA to host) positive leg |
| `pcie_tx_p[7]` | `AF6` | — | PCIe lane 7 transmit (from FPGA to host) positive leg |
| `pcie_tx_n[0]` | `R3` | — | PCIe lane 0 transmit (from FPGA to host) negative leg |
| `pcie_tx_n[1]` | `U3` | — | PCIe lane 1 transmit (from FPGA to host) negative leg |
| `pcie_tx_n[2]` | `W3` | — | PCIe lane 2 transmit (from FPGA to host) negative leg |
| `pcie_tx_n[3]` | `AA3` | — | PCIe lane 3 transmit (from FPGA to host) negative leg |
| `pcie_tx_n[4]` | `AB5` | — | PCIe lane 4 transmit (from FPGA to host) negative leg |
| `pcie_tx_n[5]` | `AC3` | — | PCIe lane 5 transmit (from FPGA to host) negative leg |
| `pcie_tx_n[6]` | `AD5` | — | PCIe lane 6 transmit (from FPGA to host) negative leg |
| `pcie_tx_n[7]` | `AF5` | — | PCIe lane 7 transmit (from FPGA to host) negative leg |
| `pcie_mgt_refclk_p` | `T6` | — | PCIe reference clock 100 MHz positive leg |
| `pcie_mgt_refclk_n` | `T5` | — | PCIe reference clock 100 MHz negative leg |
| `pcie_reset_n` | `AC22` | IOSTANDARD=LVCMOS18, PULLUP=true | PCIe PERST# reset (input, active low, pull-up enabled) |

### SMA

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `sma_in` | `B17` | IOSTANDARD=LVCMOS18 | SMA connector input to FPGA |
| `sma_out` | `B16` | IOSTANDARD=LVCMOS18, SLEW=FAST, DRIVE=12 | SMA connector output from FPGA |
| `sma_out_en` | `B19` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SMA output enable control |
| `sma_term_en` | `C16` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SMA termination enable control |

### LEDs

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `sfp_1_led[0]` | `A25` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP1 LED0 indicator output |
| `sfp_1_led[1]` | `A24` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP1 LED1 indicator output |
| `sfp_2_led[0]` | `E23` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP2 LED0 indicator output |
| `sfp_2_led[1]` | `D26` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SFP2 LED1 indicator output |
| `sma_led[0]` | `C23` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SMA LED0 indicator output |
| `sma_led[1]` | `D23` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12 | SMA LED1 indicator output |

### I2C (Board EEPROM)

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `eeprom_i2c_scl` | `B26` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12, PULLUP=true | Board EEPROM I2C clock |
| `eeprom_i2c_sda` | `C26` | IOSTANDARD=LVCMOS18, SLEW=SLOW, DRIVE=12, PULLUP=true | Board EEPROM I2C data |

### Boot Flash (BPI x16)

| Port | LOC | Attributes | Description |
|---|---|---|---|
| `flash_dq[0]` | `AE10` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 0 (BPI x16, bidirectional) |
| `flash_dq[1]` | `AC8` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 1 (BPI x16, bidirectional) |
| `flash_dq[2]` | `AD10` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 2 (BPI x16, bidirectional) |
| `flash_dq[3]` | `AD9` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 3 (BPI x16, bidirectional) |
| `flash_dq[4]` | `AC11` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 4 (BPI x16, bidirectional) |
| `flash_dq[5]` | `AF10` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 5 (BPI x16, bidirectional) |
| `flash_dq[6]` | `AF14` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 6 (BPI x16, bidirectional) |
| `flash_dq[7]` | `AE12` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 7 (BPI x16, bidirectional) |
| `flash_dq[8]` | `AD14` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 8 (BPI x16, bidirectional) |
| `flash_dq[9]` | `AF13` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 9 (BPI x16, bidirectional) |
| `flash_dq[10]` | `AE13` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 10 (BPI x16, bidirectional) |
| `flash_dq[11]` | `AD8` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 11 (BPI x16, bidirectional) |
| `flash_dq[12]` | `AC13` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 12 (BPI x16, bidirectional) |
| `flash_dq[13]` | `AD13` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 13 (BPI x16, bidirectional) |
| `flash_dq[14]` | `AA14` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 14 (BPI x16, bidirectional) |
| `flash_dq[15]` | `AB15` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash data bit 15 (BPI x16, bidirectional) |
| `flash_addr[0]` | `AD11` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 0 (BPI x16) |
| `flash_addr[1]` | `AE11` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 1 (BPI x16) |
| `flash_addr[2]` | `AF12` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 2 (BPI x16) |
| `flash_addr[3]` | `AB11` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 3 (BPI x16) |
| `flash_addr[4]` | `AB9` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 4 (BPI x16) |
| `flash_addr[5]` | `AB14` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 5 (BPI x16) |
| `flash_addr[6]` | `AA10` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 6 (BPI x16) |
| `flash_addr[7]` | `AA9` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 7 (BPI x16) |
| `flash_addr[8]` | `W10` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 8 (BPI x16) |
| `flash_addr[9]` | `AA13` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 9 (BPI x16) |
| `flash_addr[10]` | `Y15` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 10 (BPI x16) |
| `flash_addr[11]` | `AC12` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 11 (BPI x16) |
| `flash_addr[12]` | `V12` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 12 (BPI x16) |
| `flash_addr[13]` | `V11` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 13 (BPI x16) |
| `flash_addr[14]` | `Y12` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 14 (BPI x16) |
| `flash_addr[15]` | `W9` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 15 (BPI x16) |
| `flash_addr[16]` | `Y8` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 16 (BPI x16) |
| `flash_addr[17]` | `W8` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 17 (BPI x16) |
| `flash_addr[18]` | `W15` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 18 (BPI x16) |
| `flash_addr[19]` | `AA15` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 19 (BPI x16) |
| `flash_addr[20]` | `AE16` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 20 (BPI x16) |
| `flash_addr[21]` | `AF15` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 21 (BPI x16) |
| `flash_addr[22]` | `AE15` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address bit 22 (BPI x16) |
| `flash_region` | `AD15` | IOSTANDARD=LVCMOS18, PULLUP=true | Boot flash region select / control |
| `flash_ce_n` | `AC9` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash chip enable (active low) |
| `flash_oe_n` | `AC14` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash output enable (active low) |
| `flash_we_n` | `AB10` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash write enable (active low) |
| `flash_adv_n` | `Y10` | IOSTANDARD=LVCMOS18, DRIVE=16 | Boot flash address valid/advance (active low) |

## GT Transceiver Details

These high-speed interfaces use GTHE3 transceivers (Kintex UltraScale). The tables below map each MGT lane to its GT channel/site and quad common block.

### Per-lane GT Channel Mapping

| Port | LOC | GT Pin / Channel | Description |
|---|---|---|---|
| `sfp_1_rx_p` | `D2` | MGTHRXP0_227 (GTHE3_CHANNEL_X0Y12) | SFP 1 receive positive |
| `sfp_1_rx_n` | `D1` | MGTHRXN0_227 (GTHE3_CHANNEL_X0Y12) | SFP 1 receive negative |
| `sfp_1_tx_p` | `E4` | MGTHTXP0_227 (GTHE3_CHANNEL_X0Y12) | SFP 1 transmit positive |
| `sfp_1_tx_n` | `E3` | MGTHTXN0_227 (GTHE3_CHANNEL_X0Y12) | SFP 1 transmit negative |
| `sfp_2_rx_p` | `C4` | MGTHRXP1_227 (GTHE3_CHANNEL_X0Y13) | SFP 2 receive positive |
| `sfp_2_rx_n` | `C3` | MGTHRXN1_227 (GTHE3_CHANNEL_X0Y13) | SFP 2 receive negative |
| `sfp_2_tx_p` | `D6` | MGTHTXP1_227 (GTHE3_CHANNEL_X0Y13) | SFP 2 transmit positive |
| `sfp_2_tx_n` | `D5` | MGTHTXN1_227 (GTHE3_CHANNEL_X0Y13) | SFP 2 transmit negative |
| `pcie_rx_p[0]` | `P2` | MGTHRXP3_225 (GTHE3_CHANNEL_X0Y7) | PCIe RX lane 0 positive |
| `pcie_rx_n[0]` | `P1` | MGTHRXN3_225 (GTHE3_CHANNEL_X0Y7) | PCIe RX lane 0 negative |
| `pcie_tx_p[0]` | `R4` | MGTHTXP3_225 (GTHE3_CHANNEL_X0Y7) | PCIe TX lane 0 positive |
| `pcie_tx_n[0]` | `R3` | MGTHTXN3_225 (GTHE3_CHANNEL_X0Y7) | PCIe TX lane 0 negative |
| `pcie_rx_p[1]` | `T2` | MGTHRXP2_225 (GTHE3_CHANNEL_X0Y6) | PCIe RX lane 1 positive |
| `pcie_rx_n[1]` | `T1` | MGTHRXN2_225 (GTHE3_CHANNEL_X0Y6) | PCIe RX lane 1 negative |
| `pcie_tx_p[1]` | `U4` | MGTHTXP2_225 (GTHE3_CHANNEL_X0Y6) | PCIe TX lane 1 positive |
| `pcie_tx_n[1]` | `U3` | MGTHTXN2_225 (GTHE3_CHANNEL_X0Y6) | PCIe TX lane 1 negative |
| `pcie_rx_p[2]` | `V2` | MGTHRXP1_225 (GTHE3_CHANNEL_X0Y5) | PCIe RX lane 2 positive |
| `pcie_rx_n[2]` | `V1` | MGTHRXN1_225 (GTHE3_CHANNEL_X0Y5) | PCIe RX lane 2 negative |
| `pcie_tx_p[2]` | `W4` | MGTHTXP1_225 (GTHE3_CHANNEL_X0Y5) | PCIe TX lane 2 positive |
| `pcie_tx_n[2]` | `W3` | MGTHTXN1_225 (GTHE3_CHANNEL_X0Y5) | PCIe TX lane 2 negative |
| `pcie_rx_p[3]` | `Y2` | MGTHRXP0_225 (GTHE3_CHANNEL_X0Y4) | PCIe RX lane 3 positive |
| `pcie_rx_n[3]` | `Y1` | MGTHRXN0_225 (GTHE3_CHANNEL_X0Y4) | PCIe RX lane 3 negative |
| `pcie_tx_p[3]` | `AA4` | MGTHTXP0_225 (GTHE3_CHANNEL_X0Y4) | PCIe TX lane 3 positive |
| `pcie_tx_n[3]` | `AA3` | MGTHTXN0_225 (GTHE3_CHANNEL_X0Y4) | PCIe TX lane 3 negative |
| `pcie_rx_p[4]` | `AB2` | MGTHRXP3_224 (GTHE3_CHANNEL_X0Y3) | PCIe RX lane 4 positive |
| `pcie_rx_n[4]` | `AB1` | MGTHRXN3_224 (GTHE3_CHANNEL_X0Y3) | PCIe RX lane 4 negative |
| `pcie_tx_p[4]` | `AB6` | MGTHTXP3_224 (GTHE3_CHANNEL_X0Y3) | PCIe TX lane 4 positive |
| `pcie_tx_n[4]` | `AB5` | MGTHTXN3_224 (GTHE3_CHANNEL_X0Y3) | PCIe TX lane 4 negative |
| `pcie_rx_p[5]` | `AD2` | MGTHRXP2_224 (GTHE3_CHANNEL_X0Y2) | PCIe RX lane 5 positive |
| `pcie_rx_n[5]` | `AD1` | MGTHRXN2_224 (GTHE3_CHANNEL_X0Y2) | PCIe RX lane 5 negative |
| `pcie_tx_p[5]` | `AC4` | MGTHTXP2_224 (GTHE3_CHANNEL_X0Y2) | PCIe TX lane 5 positive |
| `pcie_tx_n[5]` | `AC3` | MGTHTXN2_224 (GTHE3_CHANNEL_X0Y2) | PCIe TX lane 5 negative |
| `pcie_rx_p[6]` | `AE4` | MGTHRXP1_224 (GTHE3_CHANNEL_X0Y1) | PCIe RX lane 6 positive |
| `pcie_rx_n[6]` | `AE3` | MGTHRXN1_224 (GTHE3_CHANNEL_X0Y1) | PCIe RX lane 6 negative |
| `pcie_tx_p[6]` | `AD6` | MGTHTXP1_224 (GTHE3_CHANNEL_X0Y1) | PCIe TX lane 6 positive |
| `pcie_tx_n[6]` | `AD5` | MGTHTXN1_224 (GTHE3_CHANNEL_X0Y1) | PCIe TX lane 6 negative |
| `pcie_rx_p[7]` | `AF2` | MGTHRXP0_224 (GTHE3_CHANNEL_X0Y0) | PCIe RX lane 7 positive |
| `pcie_rx_n[7]` | `AF1` | MGTHRXN0_224 (GTHE3_CHANNEL_X0Y0) | PCIe RX lane 7 negative |
| `pcie_tx_p[7]` | `AF6` | MGTHTXP0_224 (GTHE3_CHANNEL_X0Y0) | PCIe TX lane 7 positive |
| `pcie_tx_n[7]` | `AF5` | MGTHTXN0_224 (GTHE3_CHANNEL_X0Y0) | PCIe TX lane 7 negative |

### GT Common/Quad Assignment

| Interface | Quad | Common | Channels |
|---|---|---|---|
| SFP+ 1 & 2 | 227 | GTHE3_COMMON_X0Y3 | X0Y12, X0Y13 |
| PCIe Lane 0-3 | 225 | GTHE3_COMMON_X0Y1 | X0Y4, X0Y5, X0Y6, X0Y7 |
| PCIe Lane 4-7 | 224 | GTHE3_COMMON_X0Y0 | X0Y0, X0Y1, X0Y2, X0Y3 |

### MGT Reference Clocks

| Refclk | LOC | MGT Refclk Site | Frequency |
|---|---|---|---|
| `sfp_mgt_refclk_p/n` | `H6/H5` | MGTREFCLK0P/N_227 | 161.1328125 MHz |
| `pcie_mgt_refclk_p/n` | `T6/T5` | MGTREFCLK0P/N_225 | 100 MHz |

## Design Notes

- System clock: `clk_100mhz_p/n` is a 100 MHz LVDS clock (create_clock in XDC uses 10 ns period).
- SFP+ refclk: `sfp_mgt_refclk_p/n` is 161.1328125 MHz (create_clock uses 6.206 ns period).
- PCIe refclk: `pcie_mgt_refclk_p/n` is 100 MHz (create_clock uses 10 ns period).
- `pcie_reset_n` is active-low and has a pull-up enabled in the XDC.
- I2C lines (`*_i2c_*`) are configured with `PULLUP true` in the XDC.
- Flash interface is BPI x16 with `flash_dq[15:0]` and `flash_addr[22:0]` plus control signals.

## How to Use This Reference

- Use this file as the authoritative pin map for your top-level entity/module.
- When adding new ports, extend `xdc_files/fpga_k35.xdc` and update this file so the reference stays accurate.
- If you plan to change voltage standards or drive strengths, confirm the board rail (most IO are 1.8 V).
- If your board revision differs, verify these LOCs against the actual schematic before taping out a design.

## Source Files

- Pin/IO constraints: `xdc_files/fpga_k35.xdc`
- Boot timing constraints only (no LOCs): `xdc_files/boot.xdc`

