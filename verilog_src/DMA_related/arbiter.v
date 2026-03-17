module arbiter #(
    parameter SYMBOL_NUM = 2
) (
    input  wire                          i_clk,
    input  wire                          i_rst,
    input  wire [SYMBOL_NUM-1:0]         i_req,
    input  wire                          i_accept,
    output reg                           o_valid,
    output reg  [((SYMBOL_NUM > 1) ? $clog2(SYMBOL_NUM) : 1)-1:0] o_que_idx
);

function integer clog2 (input integer value);
    integer idx;
    begin
        clog2 = 0;
        for (idx = value - 1; idx > 0; idx = idx >> 1)
            clog2 = clog2 + 1;
    end
endfunction

localparam CL_SYMBOL_NUM = (SYMBOL_NUM > 1) ? clog2(SYMBOL_NUM) : 1;

function [CL_SYMBOL_NUM-1:0] wrap_que_idx (
    input [CL_SYMBOL_NUM-1:0] base_que_idx,
    input integer offset
);
    integer next_que_idx;
    begin
        next_que_idx = base_que_idx + offset;
        if (next_que_idx >= SYMBOL_NUM)
            next_que_idx = next_que_idx - SYMBOL_NUM;
        wrap_que_idx = next_que_idx[CL_SYMBOL_NUM-1:0];
    end
endfunction

reg [CL_SYMBOL_NUM-1:0] rr_pointer;

integer rr_adder;
integer selected_int;
reg [CL_SYMBOL_NUM-1:0] candidate_que_idx;

always @(*) begin
    o_valid = 1'b0;
    o_que_idx = rr_pointer;
    selected_int = 0;

    for (rr_adder = 0; rr_adder < SYMBOL_NUM; rr_adder = rr_adder + 1) begin
        candidate_que_idx = wrap_que_idx(rr_pointer, rr_adder);
        if (!o_valid && i_req[candidate_que_idx]) begin
            o_valid = 1'b1;
            selected_int = candidate_que_idx;
            o_que_idx = selected_int[CL_SYMBOL_NUM-1:0];
        end
    end
end

always @(posedge i_clk or posedge i_rst) begin
    if (i_rst) begin
        rr_pointer <= {CL_SYMBOL_NUM{1'b0}};
    end else if (i_accept && o_valid) begin
        rr_pointer <= wrap_que_idx(o_que_idx, 1);
    end
end

endmodule
