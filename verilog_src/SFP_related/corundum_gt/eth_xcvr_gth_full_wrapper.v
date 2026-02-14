//------------------------------------------------------------------------------
//  (c) Copyright 2013-2018 Xilinx, Inc. All rights reserved.
//
//  This file contains confidential and proprietary information
//  of Xilinx, Inc. and is protected under U.S. and
//  international copyright and other intellectual property
//  laws.
//
//  DISCLAIMER
//  This disclaimer is not a license and does not grant any
//  rights to the materials distributed herewith. Except as
//  otherwise provided in a valid license issued to you by
//  Xilinx, and to the maximum extent permitted by applicable
//  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
//  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
//  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
//  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
//  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
//  (2) Xilinx shall not be liable (whether in contract or tort,
//  including negligence, or under any other theory of
//  liability) for any loss or damage of any kind or nature
//  related to, arising under or in connection with these
//  materials, including for any direct, or any indirect,
//  special, incidental, or consequential loss or damage
//  (including loss of data, profits, goodwill, or any type of
//  loss or damage suffered as a result of any action brought
//  by a third party) even if such damage or loss was
//  reasonably foreseeable or Xilinx had been advised of the
//  possibility of the same.
//
//  CRITICAL APPLICATIONS
//  Xilinx products are not designed or intended to be fail-
//  safe, or for use in any application requiring fail-safe
//  performance, such as life-support or safety devices or
//  systems, Class III medical devices, nuclear facilities,
//  applications related to the deployment of airbags, or any
//  other applications that could lead to death, personal
//  injury, or severe property or environmental damage
//  (individually and collectively, "Critical
//  Applications"). Customer assumes the sole risk and
//  liability of any use of Xilinx products in Critical
//  Applications, subject only to applicable laws and
//  regulations governing limitations on product liability.
//
//  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
//  PART OF THIS FILE AT ALL TIMES.
//------------------------------------------------------------------------------


`timescale 1ps/1ps

// =====================================================================================================================
// This example design wrapper module instantiates the core and any helper blocks which the user chose to exclude from
// the core, connects them as appropriate, and maps enabled ports
// =====================================================================================================================

module eth_xcvr_gth_full_wrapper (
  input  wire [0:0] gthrxn_in
 ,input  wire [0:0] gthrxp_in
 ,output wire [0:0] gthtxn_out
 ,output wire [0:0] gthtxp_out
 ,input  wire [0:0] gtwiz_userclk_tx_reset_in
 ,output wire [0:0] gtwiz_userclk_tx_srcclk_out
 ,output wire [0:0] gtwiz_userclk_tx_usrclk_out
 ,output wire [0:0] gtwiz_userclk_tx_usrclk2_out
 ,output wire [0:0] gtwiz_userclk_tx_active_out
 ,input  wire [0:0] gtwiz_userclk_rx_reset_in
 ,output wire [0:0] gtwiz_userclk_rx_srcclk_out
 ,output wire [0:0] gtwiz_userclk_rx_usrclk_out
 ,output wire [0:0] gtwiz_userclk_rx_usrclk2_out
 ,output wire [0:0] gtwiz_userclk_rx_active_out
 ,input  wire [0:0] drp_clk_100
 ,input  wire [0:0] gtwiz_reset_all_in
 ,input  wire [0:0] gtwiz_reset_tx_pll_and_datapath_in
 ,input  wire [0:0] gtwiz_reset_tx_datapath_in
 ,input  wire [0:0] gtwiz_reset_rx_pll_and_datapath_in
 ,input  wire [0:0] gtwiz_reset_rx_datapath_in
 ,output wire [0:0] gtwiz_reset_rx_cdr_stable_out
 ,output wire [0:0] gtwiz_reset_tx_done_out
 ,output wire [0:0] gtwiz_reset_rx_done_out
 ,input  wire [63:0] gtwiz_userdata_tx_in
 ,output wire [63:0] gtwiz_userdata_rx_out
 ,input  wire [8:0] drpaddr_common_in
 ,input  wire [0:0] drpclk_common_in
 ,input  wire [15:0] drpdi_common_in
 ,input  wire [0:0] drpen_common_in
 ,input  wire [0:0] drpwe_common_in
 ,input  wire [0:0] gtrefclk00_in
 ,input  wire [0:0] gtrefclk01_in
 ,input  wire [0:0] qpll0pd_in
 ,input  wire [0:0] qpll1pd_in
 ,input  wire [4:0] qpllrsvd2_in
 ,input  wire [4:0] qpllrsvd3_in
 ,output wire [15:0] drpdo_common_out
 ,output wire [0:0] drprdy_common_out
 ,output wire [0:0] qpll0outclk_out
 ,output wire [0:0] qpll0outrefclk_out
 ,output wire [0:0] qpll1lock_out
 ,output wire [0:0] qpll1outclk_out
 ,output wire [0:0] qpll1outrefclk_out
 ,input  wire [8:0] drpaddr_in
 ,input  wire [0:0] drpclk_in
 ,input  wire [15:0] drpdi_in
 ,input  wire [0:0] drpen_in
 ,input  wire [0:0] drpwe_in
 ,input  wire [0:0] eyescanreset_in
 ,input  wire [2:0] loopback_in
 ,input  wire [0:0] rxcdrhold_in
 ,input  wire [0:0] rxdfelpmreset_in
 ,input  wire [0:0] rxgearboxslip_in
 ,input  wire [0:0] rxlpmen_in
 ,input  wire [0:0] rxpcsreset_in
 ,input  wire [1:0] rxpd_in
 ,input  wire [1:0] rxpllclksel_in
 ,input  wire [0:0] rxpmareset_in
 ,input  wire [0:0] rxpolarity_in
 ,input  wire [0:0] rxprbscntreset_in
 ,input  wire [3:0] rxprbssel_in
 ,input  wire [1:0] rxsysclksel_in
 ,input  wire [3:0] txdiffctrl_in
 ,input  wire [0:0] txelecidle_in
 ,input  wire [5:0] txheader_in
 ,input  wire [0:0] txinhibit_in
 ,input  wire [6:0] txmaincursor_in
 ,input  wire [0:0] txpcsreset_in
 ,input  wire [1:0] txpd_in
 ,input  wire [0:0] txpdelecidlemode_in
 ,input  wire [1:0] txpllclksel_in
 ,input  wire [0:0] txpmareset_in
 ,input  wire [0:0] txpolarity_in
 ,input  wire [4:0] txpostcursor_in
 ,input  wire [0:0] txprbsforceerr_in
 ,input  wire [3:0] txprbssel_in
 ,input  wire [4:0] txprecursor_in
 ,input  wire [6:0] txsequence_in
 ,input  wire [1:0] txsysclksel_in
 ,output wire [16:0] dmonitorout_out
 ,output wire [15:0] drpdo_out
 ,output wire [0:0] drprdy_out
 ,output wire [0:0] eyescandataerror_out
 ,output wire [0:0] gtpowergood_out
 ,output wire [1:0] rxdatavalid_out
 ,output wire [5:0] rxheader_out
 ,output wire [1:0] rxheadervalid_out
 ,output wire [0:0] rxprbserr_out
 ,output wire [0:0] rxprbslocked_out
 ,output wire [1:0] rxstartofseq_out
);


  // ===================================================================================================================
  // PARAMETERS AND FUNCTIONS
  // ===================================================================================================================

  // Declare and initialize local parameters and functions used for HDL generation
  localparam [191:0] P_CHANNEL_ENABLE = 192'b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001;
  `include "eth_xcvr_gth_full_example_wrapper_functions.vh"
  localparam integer P_TX_MASTER_CH_PACKED_IDX = f_calc_pk_mc_idx(0);
  localparam integer P_RX_MASTER_CH_PACKED_IDX = f_calc_pk_mc_idx(0);


  // ===================================================================================================================
  // HELPER BLOCKS
  // ===================================================================================================================

  // Any helper blocks which the user chose to exclude from the core will appear below. In addition, some signal
  // assignments related to optionally-enabled ports may appear below.

  // -------------------------------------------------------------------------------------------------------------------
  // Reset controller helper block
  // -------------------------------------------------------------------------------------------------------------------

  // Generate a single module instance which controls all PLLs and all channels within the core

  // Depending on the number of user clocking network helper blocks, either use the single user clock active indicator
  // or a logical combination of per-channel user clock active indicators as the user clock active indicator for use in
  // this block
  wire gtwiz_reset_userclk_tx_active_int;
  wire gtwiz_reset_userclk_rx_active_int;

  assign gtwiz_reset_userclk_tx_active_int = gtwiz_userclk_tx_active_out;
  assign gtwiz_reset_userclk_rx_active_int = gtwiz_userclk_rx_active_out;

  // Combine the appropriate PLL lock signals such that the reset controller can sense when all PLLs which clock each
  // data direction are locked, regardless of what PLL source is used
  wire gtwiz_reset_plllock_tx_int;
  wire gtwiz_reset_plllock_rx_int;

  wire [0:0] qpll0lock_int;

  assign gtwiz_reset_plllock_tx_int = &qpll0lock_int;
  assign gtwiz_reset_plllock_rx_int = &qpll0lock_int;

  // Combine the power good, reset done, and CDR lock indicators across all channels, per data direction
  wire [0:0] gtpowergood_int;
  wire [0:0] rxcdrlock_int;
  wire [0:0] txresetdone_int;
  wire [0:0] rxresetdone_int;
  wire gtwiz_reset_gtpowergood_int;
  wire gtwiz_reset_rxcdrlock_int;
  wire gtwiz_reset_txresetdone_int;
  wire gtwiz_reset_rxresetdone_int;

  assign gtwiz_reset_gtpowergood_int = &gtpowergood_int;
  assign gtwiz_reset_rxcdrlock_int   = &rxcdrlock_int;

  wire [0:0] txresetdone_sync;
  wire [0:0] rxresetdone_sync;
  genvar gi_ch_xrd;
  generate for (gi_ch_xrd = 0; gi_ch_xrd < 1; gi_ch_xrd = gi_ch_xrd + 1) begin : gen_ch_xrd
    (* DONT_TOUCH = "TRUE" *)
    bit_synchronizer bit_synchronizer_txresetdone_inst (
      .clk_in (drp_clk_100),
      .i_in   (txresetdone_int[gi_ch_xrd]),
      .o_out  (txresetdone_sync[gi_ch_xrd])
    );
    (* DONT_TOUCH = "TRUE" *)
    bit_synchronizer bit_synchronizer_rxresetdone_inst (
      .clk_in (drp_clk_100),
      .i_in   (rxresetdone_int[gi_ch_xrd]),
      .o_out  (rxresetdone_sync[gi_ch_xrd])
    );
  end
  endgenerate
  assign gtwiz_reset_txresetdone_int = &txresetdone_sync;
  assign gtwiz_reset_rxresetdone_int = &rxresetdone_sync;

  wire gtwiz_reset_pllreset_tx_int;
  wire gtwiz_reset_txprogdivreset_int;
  wire gtwiz_reset_gttxreset_int;
  wire gtwiz_reset_txuserrdy_int;
  wire gtwiz_reset_pllreset_rx_int;
  wire gtwiz_reset_rxprogdivreset_int;
  wire gtwiz_reset_gtrxreset_int;
  wire gtwiz_reset_rxuserrdy_int;

  // Instantiate the single reset controller
  eth_xcvr_gth_full_example_gtwiz_reset gtwiz_reset_inst (
    .drp_clk_100                        (drp_clk_100),
    .gtwiz_reset_all_in                 (gtwiz_reset_all_in),
    .gtwiz_reset_tx_pll_and_datapath_in (gtwiz_reset_tx_pll_and_datapath_in),
    .gtwiz_reset_tx_datapath_in         (gtwiz_reset_tx_datapath_in),
    .gtwiz_reset_rx_pll_and_datapath_in (gtwiz_reset_rx_pll_and_datapath_in),
    .gtwiz_reset_rx_datapath_in         (gtwiz_reset_rx_datapath_in),
    .gtwiz_reset_rx_cdr_stable_out      (gtwiz_reset_rx_cdr_stable_out),
    .gtwiz_reset_tx_done_out            (gtwiz_reset_tx_done_out),
    .gtwiz_reset_rx_done_out            (gtwiz_reset_rx_done_out),
    .gtwiz_reset_userclk_tx_active_in   (gtwiz_reset_userclk_tx_active_int),
    .gtwiz_reset_userclk_rx_active_in   (gtwiz_reset_userclk_rx_active_int),
    .gtpowergood_in                     (gtwiz_reset_gtpowergood_int),
    .txusrclk2_in                       (gtwiz_userclk_tx_usrclk2_out),
    .plllock_tx_in                      (gtwiz_reset_plllock_tx_int),
    .txresetdone_in                     (gtwiz_reset_txresetdone_int),
    .rxusrclk2_in                       (gtwiz_userclk_rx_usrclk2_out),
    .plllock_rx_in                      (gtwiz_reset_plllock_rx_int),
    .rxcdrlock_in                       (gtwiz_reset_rxcdrlock_int),
    .rxresetdone_in                     (gtwiz_reset_rxresetdone_int),
    .pllreset_tx_out                    (gtwiz_reset_pllreset_tx_int),
    .txprogdivreset_out                 (gtwiz_reset_txprogdivreset_int),
    .gttxreset_out                      (gtwiz_reset_gttxreset_int),
    .txuserrdy_out                      (gtwiz_reset_txuserrdy_int),
    .pllreset_rx_out                    (gtwiz_reset_pllreset_rx_int),
    .rxprogdivreset_out                 (gtwiz_reset_rxprogdivreset_int),
    .gtrxreset_out                      (gtwiz_reset_gtrxreset_int),
    .rxuserrdy_out                      (gtwiz_reset_rxuserrdy_int),
    .tx_enabled_tie_in                  (1'b1),
    .rx_enabled_tie_in                  (1'b1),
    .shared_pll_tie_in                  (1'b1)
  );

  // Drive the internal PLL reset inputs with the appropriate PLL reset signals produced by the reset controller. The
  // single reset controller instance generates independent transmit PLL reset and receive PLL reset outputs, which are
  // used across all such PLLs in the core.
  wire [0:0] qpll0reset_int;

  assign qpll0reset_int = {1{gtwiz_reset_pllreset_tx_int || gtwiz_reset_pllreset_rx_int}};

  // Fan out appropriate reset controller outputs to all transceiver channels
  wire [0:0] txprogdivreset_int;
  wire [0:0] gttxreset_int;
  wire [0:0] txuserrdy_int;
  wire [0:0] rxprogdivreset_int;
  wire [0:0] gtrxreset_int;
  wire [0:0] rxuserrdy_int;

  assign txprogdivreset_int  = {1{gtwiz_reset_txprogdivreset_int}};
  assign gttxreset_int       = {1{gtwiz_reset_gttxreset_int}};
  assign txuserrdy_int       = {1{gtwiz_reset_txuserrdy_int}};
  assign rxprogdivreset_int  = {1{gtwiz_reset_rxprogdivreset_int}};
  assign gtrxreset_int       = {1{gtwiz_reset_gtrxreset_int}};
  assign rxuserrdy_int       = {1{gtwiz_reset_rxuserrdy_int}};
  wire [0:0] qpll1reset_int;

  // Required assignment to expose the QPLL1RESET port per user request
  assign qpll1reset_int = {1{1'b1}};

  // Required assignment to expose the GTPOWERGOOD port per user request
  assign gtpowergood_out = gtpowergood_int;
  wire [0:0] qpll1lock_int;

  // Required assignment to expose the QPLL1LOCK port per user request
  assign qpll1lock_out = qpll1lock_int;


  // ===================================================================================================================
  // CORE INSTANCE
  // ===================================================================================================================

  // Instantiate the core, mapping its enabled ports to example design ports and helper blocks as appropriate
  wire gtwiz_userclk_tx_reset_in;
  wire gtwiz_userclk_rx_reset_in;
  wire txprgdivresetdone_out    ;
  wire rxprgdivresetdone_out    ;
  wire txpmaresetdone_out       ;
  wire rxpmaresetdone_out       ;
  assign gtwiz_userclk_tx_reset_in = ~(txprgdivresetdone_out && txpmaresetdone_out);
  assign gtwiz_userclk_rx_reset_in = ~(rxprgdivresetdone_out && rxpmaresetdone_out);
  eth_xcvr_gth_full eth_xcvr_gth_full_inst (
    .gthrxn_in                               (gthrxn_in)
   ,.gthrxp_in                               (gthrxp_in)
   ,.gthtxn_out                              (gthtxn_out)
   ,.gthtxp_out                              (gthtxp_out)
   ,.gtwiz_userclk_tx_reset_in               (gtwiz_userclk_tx_reset_in)
   ,.gtwiz_userclk_tx_srcclk_out             (gtwiz_userclk_tx_srcclk_out)
   ,.gtwiz_userclk_tx_usrclk_out             (gtwiz_userclk_tx_usrclk_out)
   ,.gtwiz_userclk_tx_usrclk2_out            (gtwiz_userclk_tx_usrclk2_out)
   ,.gtwiz_userclk_tx_active_out             (gtwiz_userclk_tx_active_out)
   ,.gtwiz_userclk_rx_reset_in               (gtwiz_userclk_rx_reset_in)
   ,.gtwiz_userclk_rx_srcclk_out             (gtwiz_userclk_rx_srcclk_out)
   ,.gtwiz_userclk_rx_usrclk_out             (gtwiz_userclk_rx_usrclk_out)
   ,.gtwiz_userclk_rx_usrclk2_out            (gtwiz_userclk_rx_usrclk2_out)
   ,.gtwiz_userclk_rx_active_out             (gtwiz_userclk_rx_active_out)
   ,.gtwiz_reset_tx_done_in                  (gtwiz_reset_tx_done_out)
   ,.gtwiz_reset_rx_done_in                  (gtwiz_reset_rx_done_out)
   ,.gtwiz_userdata_tx_in                    (gtwiz_userdata_tx_in)
   ,.gtwiz_userdata_rx_out                   (gtwiz_userdata_rx_out)
   ,.drpaddr_common_in                       (drpaddr_common_in)
   ,.drpclk_common_in                        (drpclk_common_in)
   ,.drpdi_common_in                         (drpdi_common_in)
   ,.drpen_common_in                         (drpen_common_in)
   ,.drpwe_common_in                         (drpwe_common_in)
   ,.gtrefclk00_in                           (gtrefclk00_in)
   ,.gtrefclk01_in                           (gtrefclk01_in)
   ,.qpll0pd_in                              (qpll0pd_in)
   ,.qpll0reset_in                           (qpll0reset_int)
   ,.qpll1pd_in                              (qpll1pd_in)
   ,.qpll1reset_in                           (qpll1reset_int)
   ,.qpllrsvd2_in                            (qpllrsvd2_in)
   ,.qpllrsvd3_in                            (qpllrsvd3_in)
   ,.drpdo_common_out                        (drpdo_common_out)
   ,.drprdy_common_out                       (drprdy_common_out)
   ,.qpll0lock_out                           (qpll0lock_int)
   ,.qpll0outclk_out                         (qpll0outclk_out)
   ,.qpll0outrefclk_out                      (qpll0outrefclk_out)
   ,.qpll1lock_out                           (qpll1lock_int)
   ,.qpll1outclk_out                         (qpll1outclk_out)
   ,.qpll1outrefclk_out                      (qpll1outrefclk_out)
   ,.drpaddr_in                              (drpaddr_in)
   ,.drpclk_in                               (drpclk_in)
   ,.drpdi_in                                (drpdi_in)
   ,.drpen_in                                (drpen_in)
   ,.drpwe_in                                (drpwe_in)
   ,.eyescanreset_in                         (eyescanreset_in)
   ,.gtrxreset_in                            (gtrxreset_int)
   ,.gttxreset_in                            (gttxreset_int)
   ,.loopback_in                             (loopback_in)
   ,.rxcdrhold_in                            (rxcdrhold_in)
   ,.rxdfelpmreset_in                        (rxdfelpmreset_in)
   ,.rxgearboxslip_in                        (rxgearboxslip_in)
   ,.rxlpmen_in                              (rxlpmen_in)
   ,.rxpcsreset_in                           (rxpcsreset_in)
   ,.rxpd_in                                 (rxpd_in)
   ,.rxpllclksel_in                          (rxpllclksel_in)
   ,.rxpmareset_in                           (rxpmareset_in)
   ,.rxpolarity_in                           (rxpolarity_in)
   ,.rxprbscntreset_in                       (rxprbscntreset_in)
   ,.rxprbssel_in                            (rxprbssel_in)
   ,.rxprogdivreset_in                       (rxprogdivreset_int)
   ,.rxsysclksel_in                          (rxsysclksel_in)
   ,.rxuserrdy_in                            (rxuserrdy_int)
   ,.txdiffctrl_in                           (txdiffctrl_in)
   ,.txelecidle_in                           (txelecidle_in)
   ,.txheader_in                             (txheader_in)
   ,.txinhibit_in                            (txinhibit_in)
   ,.txmaincursor_in                         (txmaincursor_in)
   ,.txpcsreset_in                           (txpcsreset_in)
   ,.txpd_in                                 (txpd_in)
   ,.txpdelecidlemode_in                     (txpdelecidlemode_in)
   ,.txpllclksel_in                          (txpllclksel_in)
   ,.txpmareset_in                           (txpmareset_in)
   ,.txpolarity_in                           (txpolarity_in)
   ,.txpostcursor_in                         (txpostcursor_in)
   ,.txprbsforceerr_in                       (txprbsforceerr_in)
   ,.txprbssel_in                            (txprbssel_in)
   ,.txprecursor_in                          (txprecursor_in)
   ,.txprogdivreset_in                       (txprogdivreset_int)
   ,.txsequence_in                           (txsequence_in)
   ,.txsysclksel_in                          (txsysclksel_in)
   ,.txuserrdy_in                            (txuserrdy_int)
   ,.dmonitorout_out                         (dmonitorout_out)
   ,.drpdo_out                               (drpdo_out)
   ,.drprdy_out                              (drprdy_out)
   ,.eyescandataerror_out                    (eyescandataerror_out)
   ,.gtpowergood_out                         (gtpowergood_int)
   ,.rxcdrlock_out                           (rxcdrlock_int)
   ,.rxdatavalid_out                         (rxdatavalid_out)
   ,.rxheader_out                            (rxheader_out)
   ,.rxheadervalid_out                       (rxheadervalid_out)
   ,.rxpmaresetdone_out                      (rxpmaresetdone_out)
   ,.rxprbserr_out                           (rxprbserr_out)
   ,.rxprbslocked_out                        (rxprbslocked_out)
   ,.rxprgdivresetdone_out                   (rxprgdivresetdone_out)
   ,.rxresetdone_out                         (rxresetdone_int)
   ,.rxstartofseq_out                        (rxstartofseq_out)
   ,.txpmaresetdone_out                      (txpmaresetdone_out)
   ,.txprgdivresetdone_out                   (txprgdivresetdone_out)
   ,.txresetdone_out                         (txresetdone_int)
);

endmodule
