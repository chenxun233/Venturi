`timescale 1ns / 1ps

module bin_to_gray #(
    parameter WIDTH = 48
) (
    input  wire [WIDTH-1:0] i_binary,
    output wire [WIDTH-1:0] o_gray
);

assign o_gray = (i_binary >> 1) ^ i_binary;

endmodule
