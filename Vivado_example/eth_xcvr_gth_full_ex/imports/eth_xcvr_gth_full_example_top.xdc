#------------------------------------------------------------------------------
# XDC for eth_xcvr_gth_full_example_top on Venturi board
# Maps GT Wizard example design to SFP channel 0
#------------------------------------------------------------------------------

# ==============================================================================
# SFP Channel 0 — GT serial pins
# ==============================================================================
set_property PACKAGE_PIN D2 [get_ports ch0_gthrxp_in]
set_property PACKAGE_PIN D1 [get_ports ch0_gthrxn_in]
set_property PACKAGE_PIN E4 [get_ports ch0_gthtxp_out]
set_property PACKAGE_PIN E3 [get_ports ch0_gthtxn_out]

# ==============================================================================
# SFP Reference Clock (161.1328125 MHz)
# ==============================================================================
set_property PACKAGE_PIN H6 [get_ports mgtrefclk0_x0y0_p]
set_property PACKAGE_PIN H5 [get_ports mgtrefclk0_x0y0_n]

# ==============================================================================
# Free-running clock (100 MHz LVDS)
# ==============================================================================
set_property PACKAGE_PIN D18 [get_ports hb_gtwiz_reset_clk_freerun_p]
set_property PACKAGE_PIN C18 [get_ports hb_gtwiz_reset_clk_freerun_n]
set_property IOSTANDARD LVDS [get_ports hb_gtwiz_reset_clk_freerun_p]
set_property IOSTANDARD LVDS [get_ports hb_gtwiz_reset_clk_freerun_n]

# ==============================================================================
# Reset input (directly Active-High — active high reset all)
# Directly Active-High — active high reset all
# ==============================================================================
set_property PACKAGE_PIN AC22 [get_ports hb_gtwiz_reset_all_in]
set_property IOSTANDARD LVCMOS18 [get_ports hb_gtwiz_reset_all_in]
set_property PULLDOWN true [get_ports hb_gtwiz_reset_all_in]

# ==============================================================================
# SFP Control Outputs
# ==============================================================================
set_property PACKAGE_PIN AA12 [get_ports sfp_0_tx_disable]
set_property IOSTANDARD LVCMOS18 [get_ports sfp_0_tx_disable]
set_property SLEW SLOW [get_ports sfp_0_tx_disable]
set_property DRIVE 12 [get_ports sfp_0_tx_disable]

set_property PACKAGE_PIN B25 [get_ports sfp_0_rs]
set_property IOSTANDARD LVCMOS18 [get_ports sfp_0_rs]
set_property SLEW SLOW [get_ports sfp_0_rs]
set_property DRIVE 12 [get_ports sfp_0_rs]

set_property PACKAGE_PIN W14 [get_ports sfp_1_tx_disable]
set_property IOSTANDARD LVCMOS18 [get_ports sfp_1_tx_disable]
set_property SLEW SLOW [get_ports sfp_1_tx_disable]
set_property DRIVE 12 [get_ports sfp_1_tx_disable]

set_false_path -to [get_ports {sfp_0_tx_disable sfp_1_tx_disable sfp_0_rs}]
set_output_delay 0.000 [get_ports {sfp_0_tx_disable sfp_1_tx_disable sfp_0_rs}]

# ==============================================================================
# Clock Constraints
# ==============================================================================
create_clock -name clk_freerun -period 10.0 [get_ports hb_gtwiz_reset_clk_freerun_p]
create_clock -name clk_mgtrefclk0_x0y0_p -period 6.206 [get_ports mgtrefclk0_x0y0_p]

# ==============================================================================
# Bitstream Configuration
# ==============================================================================
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_VOLTAGE 1.8 [current_design]
set_property CFGBVS GND [current_design]

# ==============================================================================
# False path constraints for CDC synchronizers
# ==============================================================================
set_false_path -to [get_cells -hierarchical -filter {NAME =~ *bit_synchronizer*inst/i_in_meta_reg}]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*D} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_meta*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*PRE} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_meta*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*PRE} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_sync1*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*PRE} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_sync2*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*PRE} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_sync3*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*PRE} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_out*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*CLR} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_meta*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*CLR} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_sync1*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*CLR} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_sync2*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*CLR} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_sync3*}]]
set_false_path -to [get_pins -filter {REF_PIN_NAME=~*CLR} -of_objects [get_cells -hierarchical -filter {NAME =~ *reset_synchronizer*inst/rst_in_out*}]]

# Corundum sync_reset false paths
set_false_path -to [get_cells -hierarchical -filter {NAME =~ *sync_reset*sync_reg[0]}]
