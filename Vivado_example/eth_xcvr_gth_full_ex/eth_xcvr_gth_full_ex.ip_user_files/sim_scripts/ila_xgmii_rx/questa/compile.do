vlib questa_lib/work
vlib questa_lib/msim

vlib questa_lib/msim/xil_defaultlib
vlib questa_lib/msim/xpm

vmap xil_defaultlib questa_lib/msim/xil_defaultlib
vmap xpm questa_lib/msim/xpm

vlog -work xil_defaultlib -64 -sv "+incdir+../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_xgmii_rx/hdl/verilog" \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \

vcom -work xpm -64 -93 \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_VCOMP.vhd" \

vlog -work xil_defaultlib -64 "+incdir+../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_xgmii_rx/hdl/verilog" \
"../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_xgmii_rx/sim/ila_xgmii_rx.v" \

vlog -work xil_defaultlib \
"glbl.v"

