-- Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2018.3 (lin64) Build 2405991 Thu Dec  6 23:36:41 MST 2018
-- Date        : Fri Feb 13 22:45:38 2026
-- Host        : chenxun-Z790-UD-AC running 64-bit Ubuntu 24.04.3 LTS
-- Command     : write_vhdl -force -mode synth_stub
--               /home/chenxun/Documents/Project/Venturi/Vivado_example/eth_xcvr_gth_full_ex/eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_xgmii_rx/ila_xgmii_rx_stub.vhdl
-- Design      : ila_xgmii_rx
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xcku035-fbva676-2-e
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity ila_xgmii_rx is
  Port ( 
    clk : in STD_LOGIC;
    probe0 : in STD_LOGIC_VECTOR ( 63 downto 0 );
    probe1 : in STD_LOGIC_VECTOR ( 7 downto 0 )
  );

end ila_xgmii_rx;

architecture stub of ila_xgmii_rx is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "clk,probe0[63:0],probe1[7:0]";
attribute X_CORE_INFO : string;
attribute X_CORE_INFO of stub : architecture is "ila,Vivado 2018.3";
begin
end;
