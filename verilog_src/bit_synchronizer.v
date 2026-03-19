//------------------------------------------------------------------------------
// bit_synchronizer
//
// Multi-stage synchronizer for one or more bits from another clock domain into
// the i_clk domain. Uses a 3-stage pipeline (meta + 2 sync stages) to reduce
// metastability; output is delayed by 2 i_clk cycles. No asynchronous reset;
// use for level/status bits (e.g. lock, done), not for resets.
//
// Parameter:
//   BIT_WIDTH  Width of i_in / o_out (number of bits to synchronize in parallel)
//
// Ports:
//   i_clk   Destination clock
//   i_in     Input bits from source clock domain
//   o_out    Synchronized output in i_clk domain (2-cycle latency)
//------------------------------------------------------------------------------

`timescale 1ps/1ps


module bit_synchronizer #(
  parameter BIT_WIDTH = 1
)(
  input  wire                 i_clk,
  input  wire [BIT_WIDTH-1:0] i_in,
  output wire [BIT_WIDTH-1:0] o_out

);
  // Use 3 flip-flops as a single synchronizer, and tag each declaration with the appropriate synthesis attribute to
  // enable clustering. Their GSR default values are provided by the INITIALIZE parameter.

  (* ASYNC_REG = "TRUE" *) reg [BIT_WIDTH-1:0]  i_in_meta  = 0;
  (* ASYNC_REG = "TRUE" *) reg [BIT_WIDTH-1:0]  i_in_sync1 = 0;
                           reg [BIT_WIDTH-1:0]  i_in_out   = 0;

  always @(posedge i_clk) begin
    i_in_meta  <= i_in;
    i_in_sync1 <= i_in_meta;
    i_in_out   <= i_in_sync1;
  end

  assign o_out = i_in_out;


endmodule
