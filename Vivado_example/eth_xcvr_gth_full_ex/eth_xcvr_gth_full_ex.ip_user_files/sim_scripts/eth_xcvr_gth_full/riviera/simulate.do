onbreak {quit -force}
onerror {quit -force}

asim -t 1ps +access +r +m+eth_xcvr_gth_full -L gtwizard_ultrascale_v1_7_5 -L xil_defaultlib -L unisims_ver -L unimacro_ver -L secureip -O5 xil_defaultlib.eth_xcvr_gth_full xil_defaultlib.glbl

do {wave.do}

view wave
view structure

do {eth_xcvr_gth_full.udo}

run -all

endsim

quit -force
