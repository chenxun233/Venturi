-makelib xcelium_lib/xil_defaultlib -sv \
  "/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
  "/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \
-endlib
-makelib xcelium_lib/xpm \
  "/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_VCOMP.vhd" \
-endlib
-makelib xcelium_lib/xil_defaultlib \
  "../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_xgmii_rx/sim/ila_xgmii_rx.v" \
-endlib
-makelib xcelium_lib/xil_defaultlib \
  glbl.v
-endlib

