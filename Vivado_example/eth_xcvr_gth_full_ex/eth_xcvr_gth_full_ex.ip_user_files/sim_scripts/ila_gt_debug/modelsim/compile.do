vlib modelsim_lib/work
vlib modelsim_lib/msim

vlib modelsim_lib/msim/xil_defaultlib
vlib modelsim_lib/msim/xpm

vmap xil_defaultlib modelsim_lib/msim/xil_defaultlib
vmap xpm modelsim_lib/msim/xpm

vlog -work xil_defaultlib -64 -incr -sv "+incdir+../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_gt_debug/hdl/verilog" \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \

vcom -work xpm -64 -93 \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_VCOMP.vhd" \

vlog -work xil_defaultlib -64 -incr "+incdir+../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_gt_debug/hdl/verilog" \
"../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_gt_debug/sim/ila_gt_debug.v" \

vlog -work xil_defaultlib \
"glbl.v"

