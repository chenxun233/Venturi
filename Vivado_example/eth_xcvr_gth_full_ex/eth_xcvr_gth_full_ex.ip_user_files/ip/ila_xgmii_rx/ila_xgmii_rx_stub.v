// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2018.3 (lin64) Build 2405991 Thu Dec  6 23:36:41 MST 2018
// Date        : Fri Feb 13 22:45:38 2026
// Host        : chenxun-Z790-UD-AC running 64-bit Ubuntu 24.04.3 LTS
// Command     : write_verilog -force -mode synth_stub
//               /home/chenxun/Documents/Project/Venturi/Vivado_example/eth_xcvr_gth_full_ex/eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_xgmii_rx/ila_xgmii_rx_stub.v
// Design      : ila_xgmii_rx
// Purpose     : Stub declaration of top-level module interface
// Device      : xcku035-fbva676-2-e
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "ila,Vivado 2018.3" *)
module ila_xgmii_rx(clk, probe0, probe1)
/* synthesis syn_black_box black_box_pad_pin="clk,probe0[63:0],probe1[7:0]" */;
  input clk;
  input [63:0]probe0;
  input [7:0]probe1;
endmodule
