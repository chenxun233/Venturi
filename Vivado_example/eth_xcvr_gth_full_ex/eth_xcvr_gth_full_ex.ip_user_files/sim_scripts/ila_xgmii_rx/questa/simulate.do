onbreak {quit -f}
onerror {quit -f}

vsim -t 1ps -lib xil_defaultlib ila_xgmii_rx_opt

do {wave.do}

view wave
view structure
view signals

do {ila_xgmii_rx.udo}

run -all

quit -force
