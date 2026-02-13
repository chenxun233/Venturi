vlib work
vlib activehdl

vlib activehdl/xil_defaultlib
vlib activehdl/xpm

vmap xil_defaultlib activehdl/xil_defaultlib
vmap xpm activehdl/xpm

vlog -work xil_defaultlib  -sv2k12 "+incdir+../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_gt_debug/hdl/verilog" \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \

vcom -work xpm -93 \
"/home/chenxun/Vivado/Vivado/2018.3/data/ip/xpm/xpm_VCOMP.vhd" \

vlog -work xil_defaultlib  -v2k5 "+incdir+../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_gt_debug/hdl/verilog" \
"../../../../eth_xcvr_gth_full_ex.srcs/sources_1/ip/ila_gt_debug/sim/ila_gt_debug.v" \

vlog -work xil_defaultlib \
"glbl.v"

