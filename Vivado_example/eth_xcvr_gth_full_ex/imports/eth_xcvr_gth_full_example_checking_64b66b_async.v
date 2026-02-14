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
// This example design checking module checks PRBS31 data at the appropriate parallel data width from the receiver,
// along with performing any data manipulation or sideband signaling necessary for the selected data decoding. This
// module instance checks data from a single transceiver channel for data reception demonstration purposes.
// =====================================================================================================================

module eth_xcvr_gth_full_example_checking_64b66b_async (
  input  wire         gtwiz_reset_all_in,
  input  wire         gtwiz_userclk_rx_usrclk2_in,
  input  wire         gtwiz_userclk_rx_active_in,
  input  wire   [1:0] rxdatavalid_in,
  output reg          rxgearboxslip_out = 1'b0,
  input  wire [63:0] rxdata_in,
  output reg          prbs_match_out = 1'b0
);


  // -------------------------------------------------------------------------------------------------------------------
  // Reset synchronizer
  // -------------------------------------------------------------------------------------------------------------------

  // Synchronize the example stimulus reset condition into the rxusrclk2 domain
  wire example_checking_reset_int = gtwiz_reset_all_in || ~gtwiz_userclk_rx_active_in;
  wire example_checking_reset_sync;

  (* DONT_TOUCH = "TRUE" *)
  rst_synchronizer example_checking_reset_synchronizer_inst (
    .clk_in  (gtwiz_userclk_rx_usrclk2_in),
    .rst_in  (example_checking_reset_int),
    .rst_out (example_checking_reset_sync)
  );


  // -------------------------------------------------------------------------------------------------------------------
  // Data reception declarations and assignments
  // -------------------------------------------------------------------------------------------------------------------

  // Bit-reverse the rxdata_int assignment to accomodate any differences between transmitter and receiver user interface
  // data widths, since gearbox modes transmit data MSb first.
  wire [63:0] rxdata_int;

  generate begin : gen_rxdata_int
    genvar i;
    for (i=0; i<64; i=i+1) begin : gen_all
      assign rxdata_int[i] = rxdata_in[63-i];
    end
  end
  endgenerate


  // -------------------------------------------------------------------------------------------------------------------
  // PRBS checker enable and sideband control generation
  // -------------------------------------------------------------------------------------------------------------------

  // For 64B/66B async gearbox mode data reception, the PRBS checker is always enabled
  wire prbs_any_chk_en_int = 1'b1;

  // Use the PRBS checker lock indicator as feedback, periodically pulsing the rxgearboxslip until lock is achieved
  reg [7:0] rxgearboxslip_ctr_int = 8'd0;

  always @(posedge gtwiz_userclk_rx_usrclk2_in) begin
    if (example_checking_reset_sync) begin
      rxgearboxslip_ctr_int <= 8'd0;
      rxgearboxslip_out     <= 1'b0;
    end
    else begin
      if (!prbs_match_out) begin
        rxgearboxslip_ctr_int <= rxgearboxslip_ctr_int + 8'd1;
        rxgearboxslip_out     <= &rxgearboxslip_ctr_int;
      end
      else
        rxgearboxslip_out <= 1'b0;
    end
  end


  // -------------------------------------------------------------------------------------------------------------------
  // PRBS checker block
  // -------------------------------------------------------------------------------------------------------------------

  // The prbs_any block, described in Xilinx Application Note 884 (XAPP884), "An Attribute-Programmable PRBS Generator
  // and Checker", generates or checks a parameterizable PRBS sequence. Instantiate and parameterize a prbs_any block
  // to check a PRBS31 sequence with parallel data sized to the receiver user data width.
  wire [63:0] prbs_any_chk_error_int;

  eth_xcvr_gth_full_prbs_any # (
    .CHK_MODE    (1),
    .INV_PATTERN (1),
    .POLY_LENGHT (31),
    .POLY_TAP    (28),
    .NBITS       (64)
  ) prbs_any_chk_inst (
    .RST      (example_checking_reset_sync),
    .CLK      (gtwiz_userclk_rx_usrclk2_in),
    .DATA_IN  (rxdata_int),
    .EN       (prbs_any_chk_en_int),
    .DATA_OUT (prbs_any_chk_error_int)
  );

  // The prbs_any block indicates a match of the parallel PRBS data when all DATA_OUT bits are 0. Register the result
  // of the NOR function as the PRBS match indicator.
  always @(posedge gtwiz_userclk_rx_usrclk2_in) begin
    if (example_checking_reset_sync)
      prbs_match_out <= 1'b0;
    else
      prbs_match_out <= ~(|prbs_any_chk_error_int);
  end


endmodule
