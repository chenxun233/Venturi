onbreak {quit -f}
onerror {quit -f}

vsim -voptargs="+acc" -t 1ps -L gtwizard_ultrascale_v1_7_5 -L xil_defaultlib -L unisims_ver -L unimacro_ver -L secureip -lib xil_defaultlib xil_defaultlib.eth_xcvr_gth_full xil_defaultlib.glbl

do {wave.do}

view wave
view structure
view signals

do {eth_xcvr_gth_full.udo}

run -all

quit -force
