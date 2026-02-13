//------------------------------------------------------------------------------
//  (c) Copyright 2013-2018 Xilinx, Inc. All rights reserved.
//  Modified: Replace PRBS with Corundum 10GBASE-R PHY (eth_phy_10g) + XGMII
//------------------------------------------------------------------------------

`timescale 1ps/1ps

module eth_xcvr_gth_full_example_top (

  // Differential reference clock inputs
  input  wire mgtrefclk0_x0y0_p,
  input  wire mgtrefclk0_x0y0_n,

  // Serial data ports for transceiver channel 0
  input  wire ch0_gthrxn_in,
  input  wire ch0_gthrxp_in,
  output wire ch0_gthtxn_out,
  output wire ch0_gthtxp_out,

  // Free-running clock (100 MHz LVDS from board)
  input  wire hb_gtwiz_reset_clk_freerun_p,
  input  wire hb_gtwiz_reset_clk_freerun_n,
  input  wire hb_gtwiz_reset_all_in,

  // SFP control
  output wire sfp_0_tx_disable,
  output wire sfp_0_rs,
  output wire sfp_1_tx_disable

);

  // SFP control: enable TX on channel 0, disable channel 1
  assign sfp_0_tx_disable = 1'b0;
  assign sfp_0_rs         = 1'b0;
  assign sfp_1_tx_disable = 1'b1;

  // XGMII and status — internal (no board pins), TX tied to XGMII idle
  wire [63:0] xgmii_txd = 64'h0707070707070707;
  wire [7:0]  xgmii_txc = 8'hFF;
  wire [63:0] xgmii_rxd;
  wire [7:0]  xgmii_rxc;
  wire        rx_block_lock;
  wire        rx_status;
  wire        rx_high_ber;


  // ===================================================================================================================
  // PER-CHANNEL SIGNAL ASSIGNMENTS
  // ===================================================================================================================

  wire [0:0] gthrxn_int;
  assign gthrxn_int[0:0] = ch0_gthrxn_in;

  wire [0:0] gthrxp_int;
  assign gthrxp_int[0:0] = ch0_gthrxp_in;

  wire [0:0] gthtxn_int;
  assign ch0_gthtxn_out = gthtxn_int[0:0];

  wire [0:0] gthtxp_int;
  assign ch0_gthtxp_out = gthtxp_int[0:0];

  // User clocking
  wire [0:0] gtwiz_userclk_tx_reset_int;
  wire [0:0] gtwiz_userclk_tx_usrclk2_int;
  wire [0:0] gtwiz_userclk_tx_active_int;
  wire [0:0] gtwiz_userclk_rx_reset_int;
  wire [0:0] gtwiz_userclk_rx_usrclk2_int;
  wire [0:0] gtwiz_userclk_rx_active_int;
  wire [0:0] gtwiz_userclk_tx_srcclk_int;
  wire [0:0] gtwiz_userclk_tx_usrclk_int;
  wire [0:0] gtwiz_userclk_rx_srcclk_int;
  wire [0:0] gtwiz_userclk_rx_usrclk_int;

  // Reset controller
  wire [0:0] gtwiz_reset_tx_pll_and_datapath_int;
  wire [0:0] gtwiz_reset_tx_datapath_int;
  wire [0:0] gtwiz_reset_rx_cdr_stable_int;
  wire [0:0] gtwiz_reset_tx_done_int;
  wire [0:0] gtwiz_reset_rx_done_int;

  // TX/RX user data (64-bit serdes interface)
  wire [63:0] gtwiz_userdata_tx_int;
  wire [63:0] gtwiz_userdata_rx_int;

  // TX/RX header (64b/66b)
  wire [5:0] txheader_int;
  wire [6:0] txsequence_int;
  wire [5:0] rxheader_int;
  wire [1:0] rxheadervalid_int;
  wire [1:0] rxdatavalid_int;

  // Gearbox slip (driven by PHY)
  wire [0:0] rxgearboxslip_int;

  // GT common
  wire [8:0]  drpaddr_common_int = 9'd0;
  wire [0:0]  drpclk_common_int  = 1'b0;
  wire [15:0] drpdi_common_int   = 16'd0;
  wire [0:0]  drpen_common_int   = 1'b0;
  wire [0:0]  drpwe_common_int   = 1'b0;
  wire [0:0]  gtrefclk00_int;
  wire [0:0]  gtrefclk01_int     = 1'b0;
  wire [0:0]  qpll0pd_int        = 1'b0;
  wire [0:0]  qpll1pd_int        = 1'b1;
  wire [4:0]  qpllrsvd2_int      = 5'd0;
  wire [4:0]  qpllrsvd3_int      = 5'd0;
  wire [15:0] drpdo_common_int;
  wire [0:0]  drprdy_common_int;
  wire [0:0]  qpll0outclk_int;
  wire [0:0]  qpll0outrefclk_int;
  wire [0:0]  qpll1lock_int;
  wire [0:0]  qpll1outclk_int;
  wire [0:0]  qpll1outrefclk_int;

  // GT channel DRP (unused)
  wire [8:0]  drpaddr_int       = 9'd0;
  wire [0:0]  drpclk_int        = 1'b0;
  wire [15:0] drpdi_int         = 16'd0;
  wire [0:0]  drpen_int         = 1'b0;
  wire [0:0]  drpwe_int         = 1'b0;
  wire [15:0] drpdo_int;
  wire [0:0]  drprdy_int;

  // GT channel control (fixed or VIO-less defaults)
  wire [0:0]  eyescanreset_int  = 1'b0;
  wire [2:0]  loopback_int      = 3'b000;  // No loopback — receive from SFP
  wire [0:0]  rxcdrhold_int     = 1'b0;
  wire [0:0]  rxdfelpmreset_int = 1'b0;
  wire [0:0]  rxlpmen_int       = 1'b1;   // LPM mode
  wire [0:0]  rxpcsreset_int    = 1'b0;
  wire [1:0]  rxpd_int          = 2'b00;
  wire [1:0]  rxpllclksel_int   = 2'b11;
  wire [0:0]  rxpmareset_int    = 1'b0;
  wire [0:0]  rxpolarity_int    = 1'b0;   // Match Venturi board
  wire [0:0]  rxprbscntreset_int = 1'b0;
  wire [3:0]  rxprbssel_int     = 4'd0;
  wire [1:0]  rxsysclksel_int   = 2'b10;
  wire [3:0]  txdiffctrl_int    = 4'b1100;
  wire [0:0]  txelecidle_int    = 1'b0;
  wire [0:0]  txinhibit_int     = 1'b0;
  wire [6:0]  txmaincursor_int  = 7'b1000000;
  wire [0:0]  txpcsreset_int    = 1'b0;
  wire [1:0]  txpd_int          = 2'b00;
  wire [0:0]  txpdelecidlemode_int = 1'b0;
  wire [1:0]  txpllclksel_int   = 2'b11;
  wire [0:0]  txpmareset_int    = 1'b0;
  wire [0:0]  txpolarity_int    = 1'b1;   // Match Venturi board
  wire [4:0]  txpostcursor_int  = 5'd0;
  wire [0:0]  txprbsforceerr_int = 1'b0;
  wire [3:0]  txprbssel_int     = 4'd0;
  wire [4:0]  txprecursor_int   = 5'd0;
  wire [1:0]  txsysclksel_int   = 2'b10;

  // GT status
  wire [16:0] dmonitorout_int;
  wire [0:0]  eyescandataerror_int;
  wire [0:0]  gtpowergood_int;
  wire [0:0]  rxpmaresetdone_int;
  wire [0:0]  rxprbserr_int;
  wire [0:0]  rxprbslocked_int;
  wire [0:0]  rxprgdivresetdone_int;
  wire [1:0]  rxstartofseq_int;
  wire [0:0]  txpmaresetdone_int;
  wire [0:0]  txprgdivresetdone_int;

  // gtpowergood is internal — visible via ILA


  // ===================================================================================================================
  // BUFFERS
  // ===================================================================================================================

  // Buffer the reset-all input
  wire hb_gtwiz_reset_all_buf_int;
  wire hb_gtwiz_reset_all_init_int;
  wire hb_gtwiz_reset_all_int;

  IBUF ibuf_hb_gtwiz_reset_all_inst (
    .I (hb_gtwiz_reset_all_in),
    .O (hb_gtwiz_reset_all_buf_int)
  );

  assign hb_gtwiz_reset_all_int = hb_gtwiz_reset_all_buf_int || hb_gtwiz_reset_all_init_int;

  // Differential to single-ended for free-running clock, then global buffer
  wire hb_gtwiz_reset_clk_freerun_ibuf;
  wire hb_gtwiz_reset_clk_freerun_buf_int;

  IBUFDS ibufds_clk_freerun_inst (
    .I  (hb_gtwiz_reset_clk_freerun_p),
    .IB (hb_gtwiz_reset_clk_freerun_n),
    .O  (hb_gtwiz_reset_clk_freerun_ibuf)
  );

  BUFG bufg_clk_freerun_inst (
    .I (hb_gtwiz_reset_clk_freerun_ibuf),
    .O (hb_gtwiz_reset_clk_freerun_buf_int)
  );

  // Differential reference clock buffer
  wire mgtrefclk0_x0y0_int;

  IBUFDS_GTE3 #(
    .REFCLK_EN_TX_PATH  (1'b0),
    .REFCLK_HROW_CK_SEL (2'b00),
    .REFCLK_ICNTL_RX    (2'b00)
  ) IBUFDS_GTE3_MGTREFCLK0_X0Y0_INST (
    .I     (mgtrefclk0_x0y0_p),
    .IB    (mgtrefclk0_x0y0_n),
    .CEB   (1'b0),
    .O     (mgtrefclk0_x0y0_int),
    .ODIV2 ()
  );

  assign gtrefclk00_int = mgtrefclk0_x0y0_int;


  // ===================================================================================================================
  // USER CLOCKING RESETS
  // ===================================================================================================================

  assign gtwiz_userclk_tx_reset_int = ~(&txprgdivresetdone_int && &txpmaresetdone_int);
  assign gtwiz_userclk_rx_reset_int = ~(&rxprgdivresetdone_int && &rxpmaresetdone_int);


  // ===================================================================================================================
  // 10GBASE-R PHY (Corundum eth_phy_10g) — replaces PRBS stimulus/checker
  // ===================================================================================================================

  // PHY clock and reset
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

  // SERDES ↔ PHY wires
  wire [63:0] serdes_tx_data;
  wire [1:0]  serdes_tx_hdr;
  wire [63:0] serdes_rx_data;
  wire [1:0]  serdes_rx_hdr;
  wire        serdes_rx_bitslip;
  wire        serdes_rx_reset_req;

  // Connect SERDES TX to GT (with bit-reversal as in Corundum)
  assign gtwiz_userdata_tx_int = serdes_tx_data;
  assign txheader_int          = {4'd0, serdes_tx_hdr};
  assign txsequence_int        = 7'd0;

  // Connect GT RX to SERDES
  assign serdes_rx_data    = gtwiz_userdata_rx_int;
  assign serdes_rx_hdr     = rxheader_int[1:0];
  assign rxgearboxslip_int = serdes_rx_bitslip;

  // Instantiate the 10GBASE-R PHY
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

    // XGMII — directly from/to top-level ports
    .xgmii_txd           (xgmii_txd),
    .xgmii_txc           (xgmii_txc),
    .xgmii_rxd           (xgmii_rxd),
    .xgmii_rxc           (xgmii_rxc),

    // SERDES — to/from GT
    .serdes_tx_data      (serdes_tx_data),
    .serdes_tx_hdr       (serdes_tx_hdr),
    .serdes_rx_data      (serdes_rx_data),
    .serdes_rx_hdr       (serdes_rx_hdr),
    .serdes_rx_bitslip   (serdes_rx_bitslip),
    .serdes_rx_reset_req (serdes_rx_reset_req),

    // Status
    .tx_bad_block        (),
    .rx_error_count      (),
    .rx_bad_block        (),
    .rx_sequence_error   (),
    .rx_block_lock       (rx_block_lock),
    .rx_high_ber         (rx_high_ber),
    .rx_status           (rx_status),

    // Configuration
    .cfg_tx_prbs31_enable(1'b0),
    .cfg_rx_prbs31_enable(1'b0)
  );

  // Clocks and resets are internal — used by PHY, visible via ILA


  // ===================================================================================================================
  // INITIALIZATION — use rx_status (from PHY) as "data good" indicator
  // ===================================================================================================================

  wire hb_gtwiz_reset_rx_datapath_int;
  wire hb_gtwiz_reset_rx_datapath_init_int;

  assign gtwiz_reset_tx_pll_and_datapath_int = 1'b0;
  assign gtwiz_reset_tx_datapath_int         = 1'b0;

  // Synchronize PHY watchdog's rx_reset_req into freerun domain
  wire serdes_rx_reset_req_sync;
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bit_sync_rx_reset_req_inst (
    .clk_in (hb_gtwiz_reset_clk_freerun_buf_int),
    .i_in   (serdes_rx_reset_req),
    .o_out  (serdes_rx_reset_req_sync)
  );

  assign hb_gtwiz_reset_rx_datapath_int = hb_gtwiz_reset_rx_datapath_init_int || serdes_rx_reset_req_sync;

  // Synchronize rx_status into freerun clock domain
  wire rx_status_sync;
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bit_sync_rx_status_inst (
    .clk_in (hb_gtwiz_reset_clk_freerun_buf_int),
    .i_in   (rx_status),
    .o_out  (rx_status_sync)
  );

  wire       init_done_int;
  wire [3:0] init_retry_ctr_int;

  eth_xcvr_gth_full_example_init example_init_inst (
    .clk_freerun_in  (hb_gtwiz_reset_clk_freerun_buf_int),
    .reset_all_in    (hb_gtwiz_reset_all_int),
    .tx_init_done_in (gtwiz_reset_tx_done_int),
    .rx_init_done_in (gtwiz_reset_rx_done_int),
    .rx_data_good_in (rx_status_sync),
    .reset_all_out   (hb_gtwiz_reset_all_init_int),
    .reset_rx_out    (hb_gtwiz_reset_rx_datapath_init_int),
    .init_done_out   (init_done_int),
    .retry_ctr_out   (init_retry_ctr_int)
  );


  // ===================================================================================================================
  // ILA FOR DEBUG
  // ===================================================================================================================

  // Synchronize key signals into freerun domain for ILA
  wire gtwiz_reset_tx_done_sync, gtwiz_reset_rx_done_sync;
  wire rxprgdivresetdone_sync, txprgdivresetdone_sync;
  wire rxpmaresetdone_sync, txpmaresetdone_sync;
  wire rx_block_lock_sync, rx_status_freerun_sync;
  wire gtwiz_reset_rx_cdr_stable_sync;

  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_tx_done (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(gtwiz_reset_tx_done_int[0]),
    .o_out(gtwiz_reset_tx_done_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_rx_done (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(gtwiz_reset_rx_done_int[0]),
    .o_out(gtwiz_reset_rx_done_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_rxprgdiv (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(rxprgdivresetdone_int[0]),
    .o_out(rxprgdivresetdone_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_txprgdiv (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(txprgdivresetdone_int[0]),
    .o_out(txprgdivresetdone_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_rxpma (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(rxpmaresetdone_int[0]),
    .o_out(rxpmaresetdone_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_txpma (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(txpmaresetdone_int[0]),
    .o_out(txpmaresetdone_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_blocklock (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(rx_block_lock),
    .o_out(rx_block_lock_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_rxstatus (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(rx_status),
    .o_out(rx_status_freerun_sync));
  (* DONT_TOUCH = "TRUE" *)
  eth_xcvr_gth_full_example_bit_synchronizer bs_cdrstable (
    .clk_in(hb_gtwiz_reset_clk_freerun_buf_int), .i_in(gtwiz_reset_rx_cdr_stable_int[0]),
    .o_out(gtwiz_reset_rx_cdr_stable_sync));

  // ILA instance — you need to generate a matching ILA IP core in Vivado
  // with 13 probes (widths shown), clocked by hb_gtwiz_reset_clk_freerun_buf_int
  ila_gt_debug ila_inst (
    .clk    (hb_gtwiz_reset_clk_freerun_buf_int),
    .probe0 (gtwiz_reset_tx_done_sync),           // 1
    .probe1 (gtwiz_reset_rx_done_sync),           // 1
    .probe2 (rxprgdivresetdone_sync),             // 1
    .probe3 (txprgdivresetdone_sync),             // 1
    .probe4 (rxpmaresetdone_sync),                // 1
    .probe5 (txpmaresetdone_sync),                // 1
    .probe6 (rx_block_lock_sync),                 // 1
    .probe7 (rx_status_freerun_sync),             // 1
    .probe8 (gtwiz_reset_rx_cdr_stable_sync),     // 1
    .probe9 (gtpowergood_int[0]),                 // 1
    .probe10(init_done_int),                       // 1
    .probe11(init_retry_ctr_int),                  // 4
    .probe12(hb_gtwiz_reset_all_int)               // 1
  );


  // ILA instance for XGMII RX — clocked by phy_rx_clk (rxusrclk2, ~156.25 MHz)
  // Generate ila_xgmii_rx with 2 probes: probe0 = 64-bit, probe1 = 8-bit
  ila_xgmii_rx ila_xgmii_rx_inst (
    .clk    (phy_rx_clk),
    .probe0 (xgmii_rxd),    // 64
    .probe1 (xgmii_rxc)     // 8
  );


  // ===================================================================================================================
  // EXAMPLE WRAPPER INSTANCE (GT Wizard IP + reset controller)
  // ===================================================================================================================

  eth_xcvr_gth_full_example_wrapper example_wrapper_inst (
    .gthrxn_in                               (gthrxn_int)
   ,.gthrxp_in                               (gthrxp_int)
   ,.gthtxn_out                              (gthtxn_int)
   ,.gthtxp_out                              (gthtxp_int)
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
   ,.gtwiz_reset_clk_freerun_in              ({1{hb_gtwiz_reset_clk_freerun_buf_int}})
   ,.gtwiz_reset_all_in                      ({1{hb_gtwiz_reset_all_int}})
   ,.gtwiz_reset_tx_pll_and_datapath_in      (gtwiz_reset_tx_pll_and_datapath_int)
   ,.gtwiz_reset_tx_datapath_in              (gtwiz_reset_tx_datapath_int)
   ,.gtwiz_reset_rx_pll_and_datapath_in      (1'b0)
   ,.gtwiz_reset_rx_datapath_in              ({1{hb_gtwiz_reset_rx_datapath_int}})
   ,.gtwiz_reset_rx_cdr_stable_out           (gtwiz_reset_rx_cdr_stable_int)
   ,.gtwiz_reset_tx_done_out                 (gtwiz_reset_tx_done_int)
   ,.gtwiz_reset_rx_done_out                 (gtwiz_reset_rx_done_int)
   ,.gtwiz_userdata_tx_in                    (gtwiz_userdata_tx_int)
   ,.gtwiz_userdata_rx_out                   (gtwiz_userdata_rx_int)
   ,.drpaddr_common_in                       (drpaddr_common_int)
   ,.drpclk_common_in                        (drpclk_common_int)
   ,.drpdi_common_in                         (drpdi_common_int)
   ,.drpen_common_in                         (drpen_common_int)
   ,.drpwe_common_in                         (drpwe_common_int)
   ,.gtrefclk00_in                           (gtrefclk00_int)
   ,.gtrefclk01_in                           (gtrefclk01_int)
   ,.qpll0pd_in                              (qpll0pd_int)
   ,.qpll1pd_in                              (qpll1pd_int)
   ,.qpllrsvd2_in                            (qpllrsvd2_int)
   ,.qpllrsvd3_in                            (qpllrsvd3_int)
   ,.drpdo_common_out                        (drpdo_common_int)
   ,.drprdy_common_out                       (drprdy_common_int)
   ,.qpll0outclk_out                         (qpll0outclk_int)
   ,.qpll0outrefclk_out                      (qpll0outrefclk_int)
   ,.qpll1lock_out                           (qpll1lock_int)
   ,.qpll1outclk_out                         (qpll1outclk_int)
   ,.qpll1outrefclk_out                      (qpll1outrefclk_int)
   ,.drpaddr_in                              (drpaddr_int)
   ,.drpclk_in                               (drpclk_int)
   ,.drpdi_in                                (drpdi_int)
   ,.drpen_in                                (drpen_int)
   ,.drpwe_in                                (drpwe_int)
   ,.eyescanreset_in                         (eyescanreset_int)
   ,.loopback_in                             (loopback_int)
   ,.rxcdrhold_in                            (rxcdrhold_int)
   ,.rxdfelpmreset_in                        (rxdfelpmreset_int)
   ,.rxgearboxslip_in                        (rxgearboxslip_int)
   ,.rxlpmen_in                              (rxlpmen_int)
   ,.rxpcsreset_in                           (rxpcsreset_int)
   ,.rxpd_in                                 (rxpd_int)
   ,.rxpllclksel_in                          (rxpllclksel_int)
   ,.rxpmareset_in                           (rxpmareset_int)
   ,.rxpolarity_in                           (rxpolarity_int)
   ,.rxprbscntreset_in                       (rxprbscntreset_int)
   ,.rxprbssel_in                            (rxprbssel_int)
   ,.rxsysclksel_in                          (rxsysclksel_int)
   ,.txdiffctrl_in                           (txdiffctrl_int)
   ,.txelecidle_in                           (txelecidle_int)
   ,.txheader_in                             (txheader_int)
   ,.txinhibit_in                            (txinhibit_int)
   ,.txmaincursor_in                         (txmaincursor_int)
   ,.txpcsreset_in                           (txpcsreset_int)
   ,.txpd_in                                 (txpd_int)
   ,.txpdelecidlemode_in                     (txpdelecidlemode_int)
   ,.txpllclksel_in                          (txpllclksel_int)
   ,.txpmareset_in                           (txpmareset_int)
   ,.txpolarity_in                           (txpolarity_int)
   ,.txpostcursor_in                         (txpostcursor_int)
   ,.txprbsforceerr_in                       (txprbsforceerr_int)
   ,.txprbssel_in                            (txprbssel_int)
   ,.txprecursor_in                          (txprecursor_int)
   ,.txsequence_in                           (txsequence_int)
   ,.txsysclksel_in                          (txsysclksel_int)
   ,.dmonitorout_out                         (dmonitorout_int)
   ,.drpdo_out                               (drpdo_int)
   ,.drprdy_out                              (drprdy_int)
   ,.eyescandataerror_out                    (eyescandataerror_int)
   ,.gtpowergood_out                         (gtpowergood_int)
   ,.rxdatavalid_out                         (rxdatavalid_int)
   ,.rxheader_out                            (rxheader_int)
   ,.rxheadervalid_out                       (rxheadervalid_int)
   ,.rxpmaresetdone_out                      (rxpmaresetdone_int)
   ,.rxprbserr_out                           (rxprbserr_int)
   ,.rxprbslocked_out                        (rxprbslocked_int)
   ,.rxprgdivresetdone_out                   (rxprgdivresetdone_int)
   ,.rxstartofseq_out                        (rxstartofseq_int)
   ,.txpmaresetdone_out                      (txpmaresetdone_int)
   ,.txprgdivresetdone_out                   (txprgdivresetdone_int)
  );


endmodule
