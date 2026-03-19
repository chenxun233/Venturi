`timescale 1ns / 1ps

module gray_to_binary #(
    parameter WIDTH = 48
)(
    input  wire [WIDTH-1:0] i_gray,
    output wire [WIDTH-1:0] o_binary
);

function [WIDTH-1:0] gray_to_binary_fn;
    input [WIDTH-1:0] gray;
    integer bit_idx;
    begin
        gray_to_binary_fn[WIDTH-1] = gray[WIDTH-1];
        for (bit_idx = WIDTH-2; bit_idx >= 0; bit_idx = bit_idx - 1) begin
            gray_to_binary_fn[bit_idx] = gray_to_binary_fn[bit_idx + 1] ^ gray[bit_idx];
        end
    end
endfunction

assign o_binary = gray_to_binary_fn(i_gray);

endmodule
