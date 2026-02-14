`timescale 1ns / 1ps

// =============================================================================
// SFP 10GBASE-R Wrapper
//
// Uses the GT Wizard example design infrastructure (proven on hardware) with
// Corundum's eth_phy_10g for 10GBASE-R encoding/decoding.
//
// Architecture:
//   eth_xcvr_gth_full_wrapper  (GT channel + QPLL + user clocking + reset ctrl)
//   eth_xcvr_gth_full_example_init     (retry logic)
//   eth_phy_10g                        (XGMII <-> 64b/66b SERDES)
// =============================================================================

module SFP_wrapper #(
    parameter DATA_WIDTH = 64,
    parameter CTRL_WIDTH = DATA_WIDTH/8
)(
    // Free-running clock and reset (100 MHz, active-high reset)
    input  wire                  i_ctrl_clk,
    input  wire                  i_ctrl_rst,

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

    // Status
    output wire                  o_rx_block_lock,
    output wire                  o_rx_high_ber,
    output wire                  o_rx_status,
    output wire                  o_gtpowergood,
    output wire                  o_init_done,
    output wire [3:0]            o_init_retry_ctr
);


  // ===========================================================================
  // Internal signals
  // ===========================================================================

  // User clocking
  wire [0:0] gtwiz_userclk_tx_reset_int;
  wire [0:0] gtwiz_userclk_tx_srcclk_int;
  wire [0:0] gtwiz_userclk_tx_usrclk_int;
  wire [0:0] gtwiz_userclk_tx_usrclk2_int;
  wire [0:0] gtwiz_userclk_tx_active_int;
  wire [0:0] gtwiz_userclk_rx_reset_int;
  wire [0:0] gtwiz_userclk_rx_srcclk_int;
  wire [0:0] gtwiz_userclk_rx_usrclk_int;
  wire [0:0] gtwiz_userclk_rx_usrclk2_int;
  wire [0:0] gtwiz_userclk_rx_active_int;

  // Reset controller
  wire [0:0] gtwiz_reset_rx_cdr_stable_int;
  wire [0:0] gtwiz_reset_tx_done_int;
  wire [0:0] gtwiz_reset_rx_done_int;

  // TX/RX data
  wire [63:0] gtwiz_userdata_tx_int;
  wire [63:0] gtwiz_userdata_rx_int;

  // TX/RX header (64b/66b gearbox)
  wire [5:0]  txheader_int;
  wire [6:0]  txsequence_int;
  wire [5:0]  rxheader_int;
  wire [1:0]  rxheadervalid_int;
  wire [1:0]  rxdatavalid_int;
  wire [0:0]  rxgearboxslip_int;

  // GT status
  wire [0:0]  gtpowergood_int;
  wire [16:0] dmonitorout_int;
  wire [0:0]  eyescandataerror_int;
  wire [0:0]  rxprbserr_int;
  wire [0:0]  rxprbslocked_int;
  wire [1:0]  rxstartofseq_int;
  wire [15:0] drpdo_int;
  wire [0:0]  drprdy_int;
  wire [15:0] drpdo_common_int;
  wire [0:0]  drprdy_common_int;
  wire [0:0]  qpll0outclk_int;
  wire [0:0]  qpll0outrefclk_int;
  wire [0:0]  qpll1lock_int;
  wire [0:0]  qpll1outclk_int;
  wire [0:0]  qpll1outrefclk_int;

  assign o_gtpowergood = gtpowergood_int[0];


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

  wire phy_tx_clk = gtwiz_userclk_tx_usrclk2_int[0];
  wire phy_rx_clk = gtwiz_userclk_rx_usrclk2_int[0];

  // TX reset: hold PHY in reset until GT TX is done
  wire phy_tx_rst_raw;
  sync_reset #(.N(4)) tx_reset_sync_inst (
    .clk (phy_tx_clk),
    .rst (~gtwiz_reset_tx_done_int[0]),
    .out (phy_tx_rst_raw)
  );
  (* shreg_extract = "no" *) reg phy_tx_rst_r1 = 1'b1;
  (* shreg_extract = "no" *) reg phy_tx_rst_r2 = 1'b1;
  always @(posedge phy_tx_clk) begin
    phy_tx_rst_r1 <= phy_tx_rst_raw;
    phy_tx_rst_r2 <= phy_tx_rst_r1;
  end
  wire phy_tx_rst = phy_tx_rst_r2;

  // RX reset: hold PHY in reset until GT RX is done
  wire phy_rx_rst_raw;
  sync_reset #(.N(4)) rx_reset_sync_inst (
    .clk (phy_rx_clk),
    .rst (~gtwiz_reset_rx_done_int[0]),
    .out (phy_rx_rst_raw)
  );
  (* shreg_extract = "no" *) reg phy_rx_rst_r1 = 1'b1;
  (* shreg_extract = "no" *) reg phy_rx_rst_r2 = 1'b1;
  always @(posedge phy_rx_clk) begin
    phy_rx_rst_r1 <= phy_rx_rst_raw;
    phy_rx_rst_r2 <= phy_rx_rst_r1;
  end
  wire phy_rx_rst = phy_rx_rst_r2;

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
    .rx_block_lock       (rx_block_lock_w),
    .rx_high_ber         (rx_high_ber_w),
    .rx_status           (rx_status_w),

    .cfg_tx_prbs31_enable(1'b0),
    .cfg_rx_prbs31_enable(1'b0)
  );

  // Output clocks and resets
  assign o_xgmii_tx_clk  = phy_tx_clk;
  assign o_xgmii_tx_rst  = phy_tx_rst;
  assign o_xgmii_rx_clk  = phy_rx_clk;
  assign o_xgmii_rx_rst  = phy_rx_rst;
  assign o_rx_block_lock  = rx_block_lock_w;
  assign o_rx_high_ber    = rx_high_ber_w;
  assign o_rx_status      = rx_status_w;


  // ===========================================================================
  // Initialization and retry logic
  // ===========================================================================

  wire hb_gtwiz_reset_all_int;
  wire hb_gtwiz_reset_all_init_int;
  wire hb_gtwiz_reset_rx_datapath_int;
  wire hb_gtwiz_reset_rx_datapath_init_int;

  assign hb_gtwiz_reset_all_int = i_ctrl_rst || hb_gtwiz_reset_all_init_int;

  // Synchronize PHY watchdog's rx_reset_req into freerun domain
  wire serdes_rx_reset_req_sync;
  (* DONT_TOUCH = "TRUE" *)
  bit_synchronizer bit_sync_rx_reset_req_inst (
    .clk_in (i_ctrl_clk),
    .i_in   (serdes_rx_reset_req),
    .o_out  (serdes_rx_reset_req_sync)
  );

  assign hb_gtwiz_reset_rx_datapath_int = hb_gtwiz_reset_rx_datapath_init_int || serdes_rx_reset_req_sync;

  // Synchronize rx_status into freerun clock domain
  wire rx_status_sync;
  (* DONT_TOUCH = "TRUE" *)
  bit_synchronizer bit_sync_rx_status_inst (
    .clk_in (i_ctrl_clk),
    .i_in   (rx_status_w),
    .o_out  (rx_status_sync)
  );

  wire       init_done_int;
  wire [3:0] init_retry_ctr_int;

  eth_xcvr_gth_full_example_init example_init_inst (
    .clk_freerun_in  (i_ctrl_clk),
    .reset_all_in    (hb_gtwiz_reset_all_int),
    .tx_init_done_in (gtwiz_reset_tx_done_int),
    .rx_init_done_in (gtwiz_reset_rx_done_int),
    .rx_data_good_in (rx_status_sync),
    .reset_all_out   (hb_gtwiz_reset_all_init_int),
    .reset_rx_out    (hb_gtwiz_reset_rx_datapath_init_int),
    .init_done_out   (init_done_int),
    .retry_ctr_out   (init_retry_ctr_int)
  );

  assign o_init_done      = init_done_int;
  assign o_init_retry_ctr = init_retry_ctr_int;


  // ===========================================================================
  // GT Wizard Example Wrapper
  // ===========================================================================

  eth_xcvr_gth_full_wrapper example_wrapper_inst (
    .gthrxn_in                               (i_gt_rx_n_0)
   ,.gthrxp_in                               (i_gt_rx_p_0)
   ,.gthtxn_out                              (o_gt_tx_n_0)
   ,.gthtxp_out                              (o_gt_tx_p_0)
   ,.gtwiz_userclk_tx_reset_in               (gtwiz_userclk_tx_reset_int)
   ,.gtwiz_userclk_tx_srcclk_out             (gtwiz_userclk_tx_srcclk_int)
   ,.gtwiz_userclk_tx_usrclk_out             (gtwiz_userclk_tx_usrclk_int)
   ,.gtwiz_userclk_tx_usrclk2_out            (gtwiz_userclk_tx_usrclk2_int)
   ,.gtwiz_userclk_tx_active_out             (gtwiz_userclk_tx_active_int)
   ,.gtwiz_userclk_rx_reset_in               (gtwiz_userclk_rx_reset_int)
   ,.gtwiz_userclk_rx_srcclk_out             (gtwiz_userclk_rx_srcclk_int)
   ,.gtwiz_userclk_rx_usrclk_out             (gtwiz_userclk_rx_usrclk_int)
   ,.gtwiz_userclk_rx_usrclk2_out            (gtwiz_userclk_rx_usrclk2_int)
   ,.gtwiz_userclk_rx_active_out             (gtwiz_userclk_rx_active_int)
   ,.drp_clk_100              (i_ctrl_clk)
   ,.gtwiz_reset_all_in                      (hb_gtwiz_reset_all_int)
   ,.gtwiz_reset_tx_pll_and_datapath_in      (1'b0)
   ,.gtwiz_reset_tx_datapath_in              (1'b0)
   ,.gtwiz_reset_rx_pll_and_datapath_in      (1'b0)
   ,.gtwiz_reset_rx_datapath_in              (hb_gtwiz_reset_rx_datapath_int)
   ,.gtwiz_reset_rx_cdr_stable_out           (gtwiz_reset_rx_cdr_stable_int)
   ,.gtwiz_reset_tx_done_out                 (gtwiz_reset_tx_done_int)
   ,.gtwiz_reset_rx_done_out                 (gtwiz_reset_rx_done_int)
   ,.gtwiz_userdata_tx_in                    (gtwiz_userdata_tx_int)
   ,.gtwiz_userdata_rx_out                   (gtwiz_userdata_rx_int)
   ,.drpaddr_common_in                       (9'd0)
   ,.drpclk_common_in                        (1'b0)
   ,.drpdi_common_in                         (16'd0)
   ,.drpen_common_in                         (1'b0)
   ,.drpwe_common_in                         (1'b0)
   ,.gtrefclk00_in                           (sfp_mgt_refclk)
   ,.gtrefclk01_in                           (1'b0)
   ,.qpll0pd_in                              (1'b0)
   ,.qpll1pd_in                              (1'b1)
   ,.qpllrsvd2_in                            (5'd0)
   ,.qpllrsvd3_in                            (5'd0)
   ,.drpdo_common_out                        (drpdo_common_int)
   ,.drprdy_common_out                       (drprdy_common_int)
   ,.qpll0outclk_out                         (qpll0outclk_int)
   ,.qpll0outrefclk_out                      (qpll0outrefclk_int)
   ,.qpll1lock_out                           (qpll1lock_int)
   ,.qpll1outclk_out                         (qpll1outclk_int)
   ,.qpll1outrefclk_out                      (qpll1outrefclk_int)
   ,.drpaddr_in                              (9'd0)
   ,.drpclk_in                               (1'b0)
   ,.drpdi_in                                (16'd0)
   ,.drpen_in                                (1'b0)
   ,.drpwe_in                                (1'b0)
   ,.eyescanreset_in                         (1'b0)
   ,.loopback_in                             (3'b000)
   ,.rxcdrhold_in                            (1'b0)
   ,.rxdfelpmreset_in                        (1'b0)
   ,.rxgearboxslip_in                        (rxgearboxslip_int)
   ,.rxlpmen_in                              (1'b1)      // LPM mode
   ,.rxpcsreset_in                           (1'b0)
   ,.rxpd_in                                 (2'b00)
   ,.rxpllclksel_in                          (2'b11)
   ,.rxpmareset_in                           (1'b0)
   ,.rxpolarity_in                           (1'b0)      // RX NOT inverted
   ,.rxprbscntreset_in                       (1'b0)
   ,.rxprbssel_in                            (4'd0)
   ,.rxsysclksel_in                          (2'b10)
   ,.txdiffctrl_in                           (4'b1100)
   ,.txelecidle_in                           (1'b0)
   ,.txheader_in                             (txheader_int)
   ,.txinhibit_in                            (1'b0)
   ,.txmaincursor_in                         (7'b1000000)
   ,.txpcsreset_in                           (1'b0)
   ,.txpd_in                                 (2'b00)
   ,.txpdelecidlemode_in                     (1'b0)
   ,.txpllclksel_in                          (2'b11)
   ,.txpmareset_in                           (1'b0)
   ,.txpolarity_in                           (1'b1)      // TX inverted (board)
   ,.txpostcursor_in                         (5'd0)
   ,.txprbsforceerr_in                       (1'b0)
   ,.txprbssel_in                            (4'd0)
   ,.txprecursor_in                          (5'd0)
   ,.txsequence_in                           (txsequence_int)
   ,.txsysclksel_in                          (2'b10)
   ,.dmonitorout_out                         (dmonitorout_int)
   ,.drpdo_out                               (drpdo_int)
   ,.drprdy_out                              (drprdy_int)
   ,.eyescandataerror_out                    (eyescandataerror_int)
   ,.gtpowergood_out                         (gtpowergood_int)
   ,.rxdatavalid_out                         (rxdatavalid_int)
   ,.rxheader_out                            (rxheader_int)
   ,.rxheadervalid_out                       (rxheadervalid_int)
   ,.rxprbserr_out                           (rxprbserr_int)
   ,.rxprbslocked_out                        (rxprbslocked_int)
   ,.rxstartofseq_out                        (rxstartofseq_int)
  );


endmodule
