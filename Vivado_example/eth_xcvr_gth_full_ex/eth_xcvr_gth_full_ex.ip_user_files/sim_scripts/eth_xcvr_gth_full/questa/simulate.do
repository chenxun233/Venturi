onbreak {quit -f}
onerror {quit -f}

vsim -t 1ps -lib xil_defaultlib eth_xcvr_gth_full_opt

do {wave.do}

view wave
view structure
view signals

do {eth_xcvr_gth_full.udo}

run -all

quit -force
