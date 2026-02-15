//------------------------------------------------------------------------------
// rst_synchronizer
//
// Reset synchronizer: brings an asynchronous reset into the i_clk domain with
// safe, glitch-free deassertion. Assertion of reset is asynchronous (immediate);
// deassertion is synchronous (after a short pipeline) so release is aligned to
// i_clk and no runt pulses occur.
//
// Parameters:
//   IN_ACTIVE_HIGH  1 = rst_in is active high (1 = reset), 0 = active low (0 = reset)
//   OUT_ACTIVE_HIGH 1 = rst_out is active high, 0 = active low
//
// Ports:
//   i_clk   Destination clock domain
//   rst_in   Asynchronous reset input (polarity set by IN_ACTIVE_HIGH)
//   rst_out  Synchronized reset output (polarity set by OUT_ACTIVE_HIGH)
//
// Use IN/OUT parameters to match your reset conventions (e.g. active-low in,
// active-high out for downstream logic).
//------------------------------------------------------------------------------

`timescale 1ps/1ps


module rst_synchronizer #(
  parameter IN_ACTIVE_HIGH  = 1,   // 1: rst_in active high (1 = reset), 0: active low (0 = reset)
  parameter OUT_ACTIVE_HIGH = 1    // 1: rst_out active high, 0: active low
)(
  input  wire i_clk,
  input  wire rst_in,
  output wire rst_out
);

  // Convert to internal active-high: 1 = in reset
  wire rst_active = IN_ACTIVE_HIGH ? rst_in : ~rst_in;

  (* ASYNC_REG = "TRUE" *) reg rst_in_meta  = 1'b0;
  (* ASYNC_REG = "TRUE" *) reg rst_in_sync1 = 1'b0;
                           reg rst_in_out   = 1'b0;

  always @(posedge i_clk, posedge rst_active) begin
    if (rst_active) begin
      rst_in_meta  <= 1'b1;
      rst_in_sync1 <= 1'b1;
      rst_in_out   <= 1'b1;
    end
    else begin
      rst_in_meta  <= 1'b0;
      rst_in_sync1 <= rst_in_meta;
      rst_in_out   <= rst_in_sync1;
    end
  end

  assign rst_out = OUT_ACTIVE_HIGH ? rst_in_out : ~rst_in_out;

endmodule
