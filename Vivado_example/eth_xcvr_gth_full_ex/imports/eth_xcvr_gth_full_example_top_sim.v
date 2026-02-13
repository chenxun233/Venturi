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
// This example design top simulation module instantiates the example design top module, provides basic stimulus to it
// while looping back transceiver data from transmit to receive, and utilizes the PRBS checker-based link status
// indicator to demonstrate simple data integrity checking of the design. This module is for use in simulation only.
// =====================================================================================================================

module eth_xcvr_gth_full_example_top_sim ();


  // -------------------------------------------------------------------------------------------------------------------
  // Signal declarations and basic example design stimulus
  // -------------------------------------------------------------------------------------------------------------------

  // Declare wires to loop back serial data ports for transceiver channel 0
  wire ch0_gthxn;
  wire ch0_gthxp;

  // Declare register to drive reference clock at location MGTREFCLK0_X0Y0
  reg mgtrefclk0_x0y0 = 1'b0;

  // Drive that reference clock at the appropriate frequency
  // NOTE: the following simulation reference clock period may be up to +/- 2ps from its nominal value, due to rounding
  // within Verilog timescale granularity, especially when transmitter and receiver reference clock frequencies differ
  initial begin
    mgtrefclk0_x0y0 = 1'b0;
    forever
      mgtrefclk0_x0y0 = #3103 ~mgtrefclk0_x0y0;
  end

  // Declare registers to drive reset helper block(s)
  reg hb_gtwiz_reset_clk_freerun = 1'b0;
  reg hb_gtwiz_reset_all         = 1'b1;

  // Drive the helper block free running clock
  initial begin
    hb_gtwiz_reset_clk_freerun = 1'b0;
    forever
      hb_gtwiz_reset_clk_freerun = #5000 ~hb_gtwiz_reset_clk_freerun;
  end

  // Drive the helper block "reset all" input high, then low after some time
  initial begin
    hb_gtwiz_reset_all = 1'b1;
    #5E6;
    repeat (100)
      @(hb_gtwiz_reset_clk_freerun);
    hb_gtwiz_reset_all = 1'b0;
  end

  // Declare registers and wires to interface to the PRBS-based link status ports
  reg  link_down_latched_reset = 1'b0;
  wire link_status;
  wire link_down_latched;

  // -------------------------------------------------------------------------------------------------------------------
  // Basic data integrity checking, making use of PRBS-based link status ports
  // -------------------------------------------------------------------------------------------------------------------

  // Create a basic timeout indicator which is used to abort the simulation of no link is achieved after 2ms
  reg simulation_timeout_check = 1'b0;
  initial begin
    simulation_timeout_check = 1'b0;
    #2E9;
    simulation_timeout_check = 1'b1;
  end

  // Create a basic stable link monitor which is set after 2048 consecutive cycles of link up and is reset after any
  // link loss
  reg [10:0] link_up_ctr = 11'd0;
  reg        link_stable = 1'b0;
  always @(posedge hb_gtwiz_reset_clk_freerun) begin
    if (link_status !== 1'b1) begin
      link_up_ctr <= 11'd0;
      link_stable <= 1'b0;
    end
    else begin
      if (&link_up_ctr)
        link_stable <= 1'b1;
      else
        link_up_ctr <= link_up_ctr + 11'd1;
    end
  end

  // Perform basic checking of the simulation outcome based on stable link monitor
  initial begin

    // Await de-assertion of the master reset signal
    @(negedge hb_gtwiz_reset_all);

    // Await assertion of initial link indication or simulation timeout indicator
    @(posedge link_stable, simulation_timeout_check);

    // If the simulation timeout indicator was asserted, the simulation failed to achieve initial link up in a
    // reasonable time, so display an error message and quit
    if (simulation_timeout_check) begin
      $display("Time : %12d ps   FAIL: simulation timeout. Link never achieved.", $time);
      $display("** Error: Test did not complete successfully");
      $finish;
    end

    // If the initial link was achieved, display this message and continue checks as follows
    else begin
      $display("Time : %12d ps   Initial link achieved across all transceiver channels.", $time);

      // Reset the latched link down indicator, which is always set prior to initially achieving link
      $display("Time : %12d ps   Resetting latched link down indicator.", $time);
      link_down_latched_reset = 1'b1;
      repeat (5)
        @(hb_gtwiz_reset_clk_freerun);
      link_down_latched_reset = 1'b0;

      // Continue to run the simulation for long enough to detect any subsequent errors causing link loss which may
      // occur within a reasonable simulation time
      $display("Time : %12d ps   Continuing simulation for 50us to check for maintenance of link.", $time);
      #5E7;

      // At simulation completion, if the link indicator is still high and no intermittent link loss was detected,
      // display a success message. Otherwise, display a failure message. Complete the simulation in either case.
      if ((link_status === 1'b1) && (link_down_latched === 1'b0)) begin
        $display("Time : %12d ps   PASS: simulation completed with maintained link.", $time);
        $display("** Test completed successfully");
      end
      else begin
        $display("Time : %12d ps   FAIL: simulation completed with subsequent link loss after after initial link.", $time);
        $display("** Error: Test did not complete successfully");
      end

      $finish;
    end
  end

  // -------------------------------------------------------------------------------------------------------------------
  // Instantiate example design top module as the simulation DUT
  // -------------------------------------------------------------------------------------------------------------------

  eth_xcvr_gth_full_example_top example_top_inst (
    .mgtrefclk0_x0y0_p (mgtrefclk0_x0y0),
    .mgtrefclk0_x0y0_n (~mgtrefclk0_x0y0),
    .ch0_gthrxn_in (ch0_gthxn),
    .ch0_gthrxp_in (ch0_gthxp),
    .ch0_gthtxn_out (ch0_gthxn),
    .ch0_gthtxp_out (ch0_gthxp),
    .hb_gtwiz_reset_clk_freerun_in (hb_gtwiz_reset_clk_freerun),
    .hb_gtwiz_reset_all_in (hb_gtwiz_reset_all),
    .link_down_latched_reset_in (link_down_latched_reset),
    .link_status_out (link_status),
    .link_down_latched_out (link_down_latched)
  );


endmodule
