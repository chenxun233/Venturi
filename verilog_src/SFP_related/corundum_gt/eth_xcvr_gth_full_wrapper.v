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
  input  wire         i_gthrxn
 ,input  wire         i_gthrxp
 ,output wire         o_gthtxn
 ,output wire         o_gthtxp
 ,output wire         o_gtwiz_userclk_tx_usrclk2
 ,output wire         o_gtwiz_userclk_rx_usrclk2
 ,input  wire         i_drp_clk
 ,input  wire         i_reset_all
 ,input  wire         i_rx_data_good
 ,input  wire         i_rx_reset_req
 ,output wire         o_gtwiz_reset_tx_done
 ,output wire         o_gtwiz_reset_rx_done
 ,input  wire [63:0]  i_gtwiz_userdata_tx
 ,output wire [63:0]  o_gtwiz_userdata_rx
 ,input  wire         i_gtrefclk00
 ,input  wire         i_rxgearboxslip
 ,input  wire [5:0]   i_txheader
 ,input  wire [6:0]   i_txsequence
 ,output wire [5:0]   o_rxheader
);







  wire gtwiz_userclk_tx_active;
  wire gtwiz_userclk_rx_active;


  // Combine the appropriate PLL lock signals such that the reset controller can sense when all PLLs which clock each
  // data direction are locked, regardless of what PLL source is used
  wire gtwiz_reset_plllock_tx_int;
  wire gtwiz_reset_plllock_rx_int;

  wire  qpll0lock;


  // Combine the power good, reset done, and CDR lock indicators across all channels, per data direction
  wire  gtpowergood_int;
  wire  rxcdrlock_int;
  wire  txresetdone_int;
  wire  rxresetdone_int;
  wire gtwiz_reset_gtpowergood_int;
  wire gtwiz_reset_rxcdrlock_int;
  wire gtwiz_reset_txresetdone_int;
  wire gtwiz_reset_rxresetdone_int;

  assign gtwiz_reset_gtpowergood_int = &gtpowergood_int;
  assign gtwiz_reset_rxcdrlock_int   = &rxcdrlock_int;

  wire  txresetdone_sync;
  wire  rxresetdone_sync;
  
  (* DONT_TOUCH = "TRUE" *)
  bit_synchronizer bit_synchronizer_txresetdone_inst (
    .i_clk (i_drp_clk),
    .i_in   (txresetdone_int),
    .o_out  (txresetdone_sync)
  );
  (* DONT_TOUCH = "TRUE" *)
  bit_synchronizer bit_synchronizer_rxresetdone_inst (
    .i_clk (i_drp_clk),
    .i_in   (rxresetdone_int),
    .o_out  (rxresetdone_sync)
  );
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

  wire gtwiz_reset_rx_cdr_stable_int;

  // Instantiate the single reset controller
  eth_xcvr_gth_full_example_gtwiz_reset gtwiz_reset_inst (
    .drp_clk_100                        (i_drp_clk),
    .gtwiz_reset_all_in                 (i_reset_all),
    .gtwiz_reset_tx_pll_and_datapath_in (1'b0),
    .gtwiz_reset_tx_datapath_in         (1'b0),
    .gtwiz_reset_rx_pll_and_datapath_in (1'b0),
    .gtwiz_reset_rx_datapath_in         (i_rx_reset_req),
    .gtwiz_reset_rx_cdr_stable_out      (gtwiz_reset_rx_cdr_stable_int),
    .gtwiz_reset_tx_done_out            (o_gtwiz_reset_tx_done),
    .gtwiz_reset_rx_done_out            (o_gtwiz_reset_rx_done),
    .gtwiz_reset_userclk_tx_active_in   (gtwiz_userclk_tx_active),
    .gtwiz_reset_userclk_rx_active_in   (gtwiz_userclk_rx_active),
    .gtpowergood_in                     (gtwiz_reset_gtpowergood_int),
    .txusrclk2_in                       (o_gtwiz_userclk_tx_usrclk2),
    .plllock_tx_in                      (qpll0lock),
    .txresetdone_in                     (gtwiz_reset_txresetdone_int),
    .rxusrclk2_in                       (o_gtwiz_userclk_rx_usrclk2),
    .plllock_rx_in                      (qpll0lock),
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
  wire  qpll0reset_int;

  assign qpll0reset_int = {1{gtwiz_reset_pllreset_tx_int || gtwiz_reset_pllreset_rx_int}};

  // Fan out appropriate reset controller outputs to all transceiver channels
  wire  txprogdivreset_int;
  wire  gttxreset_int;
  wire  txuserrdy_int;
  wire  rxprogdivreset_int;
  wire  gtrxreset_int;
  wire  rxuserrdy_int;

  assign txprogdivreset_int  = {1{gtwiz_reset_txprogdivreset_int}};
  assign gttxreset_int       = {1{gtwiz_reset_gttxreset_int}};
  assign txuserrdy_int       = {1{gtwiz_reset_txuserrdy_int}};
  assign rxprogdivreset_int  = {1{gtwiz_reset_rxprogdivreset_int}};
  assign gtrxreset_int       = {1{gtwiz_reset_gtrxreset_int}};
  assign rxuserrdy_int       = {1{gtwiz_reset_rxuserrdy_int}};



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

  wire [16:0] dmonitorout_int;
  wire [15:0] drpdo_int;
  wire        drprdy_int;
  wire [1:0]  rxdatavalid_int;
  wire [1:0]  rxheadervalid_int;
  wire        rxprbserr_int;
  wire        rxprbslocked_int;
  wire [1:0]  rxstartofseq_int;

  eth_xcvr_gth_full eth_xcvr_gth_full_inst (
    .gthrxn_in                               (i_gthrxn)
   ,.gthrxp_in                               (i_gthrxp)
   ,.gthtxn_out                              (o_gthtxn)
   ,.gthtxp_out                              (o_gthtxp)
   ,.gtwiz_userclk_tx_reset_in               (gtwiz_userclk_tx_reset_in)
   ,.gtwiz_userclk_tx_srcclk_out             ()
   ,.gtwiz_userclk_tx_usrclk_out             ()
   ,.gtwiz_userclk_tx_usrclk2_out            (o_gtwiz_userclk_tx_usrclk2)
   ,.gtwiz_userclk_tx_active_out             (gtwiz_userclk_tx_active)
   ,.gtwiz_userclk_rx_reset_in               (gtwiz_userclk_rx_reset_in)
   ,.gtwiz_userclk_rx_usrclk2_out            (o_gtwiz_userclk_rx_usrclk2)
   ,.gtwiz_userclk_rx_active_out             (gtwiz_userclk_rx_active)
   ,.gtwiz_reset_tx_done_in                  (o_gtwiz_reset_tx_done)
   ,.gtwiz_reset_rx_done_in                  (o_gtwiz_reset_rx_done)
   ,.gtwiz_userdata_tx_in                    (i_gtwiz_userdata_tx)
   ,.gtwiz_userdata_rx_out                   (o_gtwiz_userdata_rx)
   ,.gtrefclk00_in                           (i_gtrefclk00)
   ,.qpll0reset_in                           (qpll0reset_int)
   ,.qpll1reset_in                           (1'b1)
   ,.drpdo_common_out                        ()
   ,.drprdy_common_out                       ()
   ,.qpll0lock_out                           (qpll0lock)
   ,.qpll0outclk_out                         ()
   ,.qpll0outrefclk_out                      ()
   ,.qpll1lock_out                           ()
   ,.qpll1outclk_out                         ()
   ,.qpll1outrefclk_out                      ()
   ,.gtrxreset_in                            (gtrxreset_int)
   ,.gttxreset_in                            (gttxreset_int)
   ,.loopback_in                             (3'b000)
   ,.rxgearboxslip_in                        (i_rxgearboxslip)
   ,.rxlpmen_in                              (1'b1)
   ,.rxpllclksel_in                          (2'b11)
   ,.rxpmareset_in                           (1'b0)
   ,.rxpolarity_in                           (1'b0)
   ,.rxprogdivreset_in                       (rxprogdivreset_int)
   ,.rxsysclksel_in                          (2'b10)
   ,.rxuserrdy_in                            (rxuserrdy_int)
   ,.txdiffctrl_in                           (4'b1100)
   ,.txheader_in                             (i_txheader)
   ,.txmaincursor_in                         (7'b1000000)
   ,.txpllclksel_in                          (2'b11)
   ,.txpolarity_in                           (1'b1)
   ,.txprogdivreset_in                       (txprogdivreset_int)
   ,.txsequence_in                           (i_txsequence)
   ,.txsysclksel_in                          (2'b10)
   ,.txuserrdy_in                            (txuserrdy_int)
   ,.dmonitorout_out                         (dmonitorout_int)
   ,.drpdo_out                               (drpdo_int)
   ,.drprdy_out                              (drprdy_int)
   ,.gtpowergood_out                         (gtpowergood_int)
   ,.rxcdrlock_out                           (rxcdrlock_int)
   ,.rxdatavalid_out                         (rxdatavalid_int)
   ,.rxheader_out                            (o_rxheader)
   ,.rxheadervalid_out                       (rxheadervalid_int)
   ,.rxpmaresetdone_out                      (rxpmaresetdone_out)
   ,.rxprbserr_out                           (rxprbserr_int)
   ,.rxprbslocked_out                        (rxprbslocked_int)
   ,.rxprgdivresetdone_out                   (rxprgdivresetdone_out)
   ,.rxresetdone_out                         (rxresetdone_int)
   ,.rxstartofseq_out                        (rxstartofseq_int)
   ,.txpmaresetdone_out                      (txpmaresetdone_out)
   ,.txprgdivresetdone_out                   (txprgdivresetdone_out)
   ,.txresetdone_out                         (txresetdone_int)
);

endmodule
