`timescale 1ns / 1ps

// =============================================================================
// SFP 10GBASE-R Wrapper
//
// Uses the GT Wizard example design infrastructure (proven on hardware) with
// Corundum's eth_phy_10g for 10GBASE-R encoding/decoding.
//
// Architecture:
//   eth_xcvr_gth_full_wrapper  (GT channel + QPLL + user clocking + reset ctrl + init retry)
//   eth_phy_10g                (XGMII <-> 64b/66b SERDES)
// =============================================================================

module pcs_pma_wrapper #(
    parameter DATA_WIDTH = 64,
    parameter CTRL_WIDTH = DATA_WIDTH/8
)(
    // Free-running clock and reset (100 MHz, active-high reset)
    input  wire                  i_drp_clk,
    input  wire                  i_rst_p,

    // GT reference clock (161.1328125 MHz differential)
    input  wire                  i_gt_refclk_p,
    input  wire                  i_gt_refclk_n,

    // GT serial pins
    input  wire                  i_gt_rx_p_0,
    input  wire                  i_gt_rx_n_0,
    output wire                  o_gt_tx_p_0,
    output wire                  o_gt_tx_n_0,

    // XGMII interface
    input  wire [DATA_WIDTH-1:0] i_xgmii_txd,
    input  wire [CTRL_WIDTH-1:0] i_xgmii_txc,
    output wire [DATA_WIDTH-1:0] o_xgmii_rxd,
    output wire [CTRL_WIDTH-1:0] o_xgmii_rxc,

    // XGMII clocks and resets (active-high resets)
    output wire                  o_xgmii_tx_clk,
    output wire                  o_xgmii_tx_rst,
    output wire                  o_xgmii_rx_clk,
    output wire                  o_xgmii_rx_rst,
    output wire                  o_rx_status
);


  // ===========================================================================
  // Internal signals
  // ===========================================================================



  // Reset controller
  wire [0:0] gtwiz_reset_tx_done_int;
  wire [0:0] gtwiz_reset_rx_done_int;

  // TX/RX data
  wire [63:0] gtwiz_userdata_tx_int;
  wire [63:0] gtwiz_userdata_rx_int;

  // TX/RX header (64b/66b gearbox)
  wire [5:0]  txheader_int;
  wire [6:0]  txsequence_int;
  wire [5:0]  rxheader_int;
  wire [0:0]  rxgearboxslip_int;

  // ===========================================================================
  // Reference clock buffer
  // ===========================================================================

  wire sfp_mgt_refclk;

  IBUFDS_GTE3 #(
    .REFCLK_EN_TX_PATH  (1'b0),
    .REFCLK_HROW_CK_SEL (2'b00),
    .REFCLK_ICNTL_RX    (2'b00)
  ) ibufds_gte3_refclk_inst (
    .I     (i_gt_refclk_p),
    .IB    (i_gt_refclk_n),
    .CEB   (1'b0),
    .O     (sfp_mgt_refclk),
    .ODIV2 ()
  );





  // ===========================================================================
  // 10GBASE-R PHY (Corundum eth_phy_10g)
  // ===========================================================================

  wire phy_tx_clk ;
  wire phy_rx_clk ;

  // TX reset: hold PHY in reset until GT TX is done
  wire phy_tx_rst;
  rst_synchronizer #(
    .IN_ACTIVE_HIGH  (1),
    .OUT_ACTIVE_HIGH (1)
  ) tx_reset_sync_inst (
    .i_clk   (phy_tx_clk),
    .rst_in  (~gtwiz_reset_tx_done_int[0]),
    .rst_out (phy_tx_rst)
  );

  // RX reset: hold PHY in reset until GT RX is done
  wire phy_rx_rst;
  rst_synchronizer #(
    .IN_ACTIVE_HIGH  (1),
    .OUT_ACTIVE_HIGH (1)
  ) rx_reset_sync_inst (
    .i_clk   (phy_rx_clk),
    .rst_in  (~gtwiz_reset_rx_done_int[0]),
    .rst_out (phy_rx_rst)
  );

  // SERDES wires
  wire [63:0] serdes_tx_data;
  wire [1:0]  serdes_tx_hdr;
  wire [1:0]  serdes_rx_hdr;
  wire        serdes_rx_bitslip;
  wire        serdes_rx_reset_req;

  // Connect PHY SERDES TX to GT
  assign gtwiz_userdata_tx_int = serdes_tx_data;
  assign txheader_int          = {4'd0, serdes_tx_hdr};
  assign txsequence_int        = 7'd0;

  // Connect GT RX to PHY SERDES
  assign serdes_rx_hdr         = rxheader_int[1:0];
  assign rxgearboxslip_int     = serdes_rx_bitslip;

  wire rx_block_lock_w;
  wire rx_high_ber_w;
  wire rx_status_w;

  eth_phy_10g #(
    .DATA_WIDTH         (64),
    .CTRL_WIDTH         (8),
    .HDR_WIDTH          (2),
    .BIT_REVERSE        (1),
    .SCRAMBLER_DISABLE  (0),
    .PRBS31_ENABLE      (0),
    .TX_SERDES_PIPELINE (0),
    .RX_SERDES_PIPELINE (0),
    .BITSLIP_HIGH_CYCLES(1),
    .BITSLIP_LOW_CYCLES (8),
    .COUNT_125US        (125000/6.4)
  ) phy_inst (
    .tx_clk              (phy_tx_clk),
    .tx_rst              (phy_tx_rst),
    .rx_clk              (phy_rx_clk),
    .rx_rst              (phy_rx_rst),

    .xgmii_txd           (i_xgmii_txd),
    .xgmii_txc           (i_xgmii_txc),
    .xgmii_rxd           (o_xgmii_rxd),
    .xgmii_rxc           (o_xgmii_rxc),

    .serdes_tx_data      (serdes_tx_data),
    .serdes_tx_hdr       (serdes_tx_hdr),
    .serdes_rx_data      (gtwiz_userdata_rx_int),
    .serdes_rx_hdr       (serdes_rx_hdr),
    .serdes_rx_bitslip   (serdes_rx_bitslip),
    .serdes_rx_reset_req (serdes_rx_reset_req),

    .tx_bad_block        (),
    .rx_error_count      (),
    .rx_bad_block        (),
    .rx_sequence_error   (),
    .rx_block_lock       (),
    .rx_high_ber         (),
    .rx_status           (rx_status_w),

    .cfg_tx_prbs31_enable(1'b0),
    .cfg_rx_prbs31_enable(1'b0)
  );

  // Output clocks and resets
  assign o_xgmii_tx_clk   = phy_tx_clk;
  assign o_xgmii_tx_rst   = phy_tx_rst;
  assign o_xgmii_rx_clk   = phy_rx_clk;
  assign o_xgmii_rx_rst   = phy_rx_rst;
  assign o_rx_status      = rx_status_w;


  // ===========================================================================
  // Sync PHY status into freerun domain for wrapper init block
  // ===========================================================================

  wire serdes_rx_reset_req_sync;
  (* DONT_TOUCH = "TRUE" *)
  bit_synchronizer bit_sync_rx_reset_req_inst (
    .i_clk (i_drp_clk),
    .i_in   (serdes_rx_reset_req),
    .o_out  (serdes_rx_reset_req_sync)
  );

  wire rx_status_sync;
  (* DONT_TOUCH = "TRUE" *)
  bit_synchronizer bit_sync_rx_status_inst (
    .i_clk (i_drp_clk),
    .i_in   (rx_status_w),
    .o_out  (rx_status_sync)
  );

  // ===========================================================================
  // GT Wizard Wrapper (includes init/retry logic)
  // ===========================================================================

  eth_xcvr_gth_full_wrapper example_wrapper_inst (
    .i_gthrxn                    (i_gt_rx_n_0)
   ,.i_gthrxp                    (i_gt_rx_p_0)
   ,.o_gthtxn                    (o_gt_tx_n_0)
   ,.o_gthtxp                    (o_gt_tx_p_0)
   ,.o_gtwiz_userclk_tx_usrclk2  (phy_tx_clk)
   ,.o_gtwiz_userclk_rx_usrclk2  (phy_rx_clk)
   ,.i_drp_clk                   (i_drp_clk)
   ,.i_reset_all                 (i_rst_p)
   ,.i_rx_data_good              (rx_status_sync)
   ,.i_rx_reset_req              (serdes_rx_reset_req_sync)
   ,.o_gtwiz_reset_tx_done       (gtwiz_reset_tx_done_int)
   ,.o_gtwiz_reset_rx_done       (gtwiz_reset_rx_done_int)
   ,.i_gtwiz_userdata_tx         (gtwiz_userdata_tx_int)
   ,.o_gtwiz_userdata_rx         (gtwiz_userdata_rx_int)
   ,.i_gtrefclk00                (sfp_mgt_refclk)
   ,.i_rxgearboxslip             (rxgearboxslip_int)
   ,.i_txheader                  (txheader_int)
   ,.i_txsequence                (txsequence_int)
   ,.o_rxheader                  (rxheader_int)
  );


endmodule
