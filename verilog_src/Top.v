`timescale 1ns / 1ps

// =============================================================================
// Top.v - Venturi FPGA NIC Top Level Module
// =============================================================================
// Top-level wrapper using the modular PCIe interface architecture:
//   pcie_interface.v → CQ_parser, CC_formatter, RQ_formatter, RC_parser
//   user_logic.v     → Register handling and DMA control
// =============================================================================

module top #(
parameter DATA_WIDTH = 256,
parameter KEEP_WIDTH = DATA_WIDTH / 32,
parameter BAR0_SIZE  = 16
)(
// =========================================================================
// PCIe Interface
// =========================================================================
output wire [7:0]   pci_exp_txn,
output wire [7:0]   pci_exp_txp,
input  wire [7:0]   pci_exp_rxn,
input  wire [7:0]   pci_exp_rxp,

// =========================================================================
// Reference Clock (100MHz from PCIe slot)
// =========================================================================
input  wire         sys_clk_p,
input  wire         sys_clk_n,

// =========================================================================
// PCIe Reset (active-low from slot PERST#)
// =========================================================================
input  wire         sys_rst_n
);

// =========================================================================
// Clock Buffer for Reference Clock
// =========================================================================
wire sys_clk;
wire sys_clk_gt;

IBUFDS_GTE3 #(
    .REFCLK_EN_TX_PATH  (1'b0),
    .REFCLK_HROW_CK_SEL (2'b00),
    .REFCLK_ICNTL_RX    (2'b00)
) refclk_ibuf (
    .O     (sys_clk_gt),
    .ODIV2 (sys_clk),
    .CEB   (1'b0),
    .I     (sys_clk_p),
    .IB    (sys_clk_n)
);

// =========================================================================
// Internal Wires - User Clock and Reset
// =========================================================================
wire user_clk;
wire user_reset_p;

// =========================================================================
// Internal Wires - CQ Parser Outputs
// =========================================================================
wire                    cq_valid;
wire [3:0]              cq_type;
wire [BAR0_SIZE-1:0]    cq_reg_addr;
wire [63:0]             cq_payload;
wire [2:0]              cq_bar_id;
wire [15:0]             cq_requester_id;
wire [7:0]              cq_tag;
wire [2:0]              cq_tc;
wire [6:0]              cq_lower_addr;
wire [10:0]             cq_payload_dw_count;
wire                    cq_last;

// =========================================================================
// Internal Wires - CC Formatter Inputs
// =========================================================================
wire                    cc_ready;
wire                    cc_valid;
wire [15:0]             cc_requester_id;
wire [7:0]              cc_tag;
wire [2:0]              cc_tc;
wire [6:0]              cc_lower_addr;
wire [10:0]             cc_dword_count;
wire [2:0]              cc_status;
wire [DATA_WIDTH/2-1:0] cc_payload;
wire                    cc_last;

// =========================================================================
// Internal Wires - RQ Formatter Inputs
// =========================================================================
wire                    rq_ready;
wire                    rq_valid;
wire [3:0]              rq_type;
wire                    rq_payload_sop;
wire                    rq_payload_last;
wire [63:0]             rq_addr;
wire [10:0]             rq_payload_dw_count;
wire [7:0]              rq_tag;
wire [2:0]              rq_tc;
wire [DATA_WIDTH-1:0]   rq_payload;

// =========================================================================
// Internal Wires - RC Parser Outputs (realigned by gearbox)
// =========================================================================
wire                    rc_valid;
wire [11:0]             rc_lower_addr;
wire [3:0]              rc_err_code;
wire [7:0]              rc_tag;
wire [12:0]             rc_payload_byte_count;
wire                    rc_request_completed;
wire [15:0]             rc_requester_id;
wire                    rc_payload_last;
wire [DATA_WIDTH-1:0]   rc_payload;
wire [KEEP_WIDTH-1:0]   rc_payload_dw_keep;
wire                    rc_posioned;
wire [12:0]             rc_payload_dw_count;

// =========================================================================
// Internal Wires - Configuration
// =========================================================================
wire [2:0]              cfg_max_payload;
wire [2:0]              cfg_max_read_req;
wire [15:0]             pcie_requester_id;

// =========================================================================
// Internal Wires - Status
// =========================================================================


// =========================================================================
// PCIe Interface Instantiation
// =========================================================================
pcie_interface #(
    .DATA_WIDTH             (DATA_WIDTH),
    .BAR0_SIZE              (BAR0_SIZE)
) pcie_if_inst (
    // PCIe Physical Pins
    .pci_exp_txn            (pci_exp_txn),
    .pci_exp_txp            (pci_exp_txp),
    .pci_exp_rxn            (pci_exp_rxn),
    .pci_exp_rxp            (pci_exp_rxp),

    // System Interface
    .sys_clk                (sys_clk),
    .sys_clk_gt             (sys_clk_gt),
    .sys_reset              (sys_rst_n),        // Active-low (IP configured for ACTIVE LOW)
    .user_clk               (user_clk),
    .user_reset_p           (user_reset_p),      // Active-high

    // CQ Parser Outputs
    .cq_valid             (cq_valid),
    .cq_type              (cq_type),
    .cq_reg_addr           (cq_reg_addr),
    .cq_payload           (cq_payload),
    .cq_bar_id            (cq_bar_id),
    .cq_requester_id      (cq_requester_id),
    .cq_tag               (cq_tag),
    .cq_tc                (cq_tc),
    .cq_lower_addr        (cq_lower_addr),
    .cq_payload_dw_count   (cq_payload_dw_count),
    .cq_last               (cq_last),

    // CC Formatter Inputs
    .cc_ready               (cc_ready),
    .cc_valid               (cc_valid),
    .cc_requester_id        (cc_requester_id),
    .cc_tag                 (cc_tag),
    .cc_tc                  (cc_tc),
    .cc_lower_addr          (cc_lower_addr),
    .cc_dword_count         (cc_dword_count),
    .cc_status              (cc_status),
    .cc_payload             (cc_payload),
    .cc_last                (cc_last),

    // RQ Formatter Inputs
    .rq_ready               (rq_ready),
    .rq_valid               (rq_valid),
    .rq_type                (rq_type),
    .rq_payload_sop         (rq_payload_sop),
    .rq_payload_last        (rq_payload_last),
    .rq_addr                (rq_addr),
    .rq_payload_dw_count    (rq_payload_dw_count),
    .rq_tag                 (rq_tag),
    .rq_tc                  (rq_tc),
    .rq_payload             (rq_payload),
    // RC Parser Outputs (realigned by gearbox)
    .rc_lower_addr          (rc_lower_addr),
    .rc_err_code            (rc_err_code),
    .rc_valid               (rc_valid),
    .rc_tag                 (rc_tag),
    .rc_payload_byte_count  (rc_payload_byte_count),
    .rc_request_completed   (rc_request_completed),
    .rc_requester_id        (rc_requester_id),
    .rc_payload_last        (rc_payload_last),
    .rc_payload             (rc_payload),
    .rc_payload_dw_keep     (rc_payload_dw_keep),
    .rc_posioned            (rc_posioned),
    .rc_payload_dw_count    (rc_payload_dw_count),

    // Configuration Outputs
    .cfg_max_payload        (cfg_max_payload),
    .cfg_max_read_req       (cfg_max_read_req),
    .pcie_requester_id      (pcie_requester_id)
);

// =========================================================================
// User Logic Instantiation
// =========================================================================
user_logic #(
    .DATA_WIDTH             (DATA_WIDTH),
    .BAR0_SIZE              (BAR0_SIZE)
) user_logic_inst (
    .user_clk                (user_clk),
    .user_reset_p           (user_reset_p),

    // CQ Parser Interface
    .cq_valid               (cq_valid),
    .cq_type                (cq_type),
    .cq_reg_addr            (cq_reg_addr),
    .cq_payload             (cq_payload),
    .cq_bar_id              (cq_bar_id),
    .cq_requester_id        (cq_requester_id),
    .cq_tag                 (cq_tag),
    .cq_tc                  (cq_tc),
    .cq_lower_addr          (cq_lower_addr),
    .cq_payload_dw_count    (cq_payload_dw_count),
    .cq_last                (cq_last),

    // CC Formatter Interface
    .cc_ready               (cc_ready),
    .cc_valid               (cc_valid),
    .cc_requester_id        (cc_requester_id),
    .cc_tag                 (cc_tag),
    .cc_tc                  (cc_tc),
    .cc_lower_addr          (cc_lower_addr),
    .cc_dword_count         (cc_dword_count),
    .cc_status              (cc_status),
    .cc_payload                (cc_payload),
    .cc_last                (cc_last),

    // RQ Formatter Interface
    .rq_ready               (rq_ready),
    .rq_valid               (rq_valid),
    .rq_type                (rq_type),
    .rq_payload_sop                 (rq_payload_sop),
    .rq_payload_last                (rq_payload_last),
    .rq_addr                (rq_addr),
    .rq_payload_dw_count         (rq_payload_dw_count),
    .rq_tag                 (rq_tag),
    .rq_tc                  (rq_tc),
    .rq_payload             (rq_payload),

    // RC Parser Interface (realigned by gearbox)
    .rc_lower_addr          (rc_lower_addr),
    .rc_err_code            (rc_err_code),
    .rc_payload_byte_count  (rc_payload_byte_count),
    .rc_request_completed   (rc_request_completed),
    .rc_requester_id        (rc_requester_id),
    .rc_tag                 (rc_tag),
    .rc_valid               (rc_valid),
    .rc_payload_last        (rc_payload_last),
    .rc_payload             (rc_payload),
    .rc_payload_dw_keep     (rc_payload_dw_keep),
    .rc_posioned            (rc_posioned),
    .rc_payload_dw_count    (rc_payload_dw_count),

    // Configuration
    .cfg_max_payload        (cfg_max_payload),
    .cfg_max_read_req       (cfg_max_read_req),
    .pcie_requester_id      (pcie_requester_id)
);

endmodule
