module arbiter #(
    parameter SYMBOL_NUM = 2
) (
    input  wire                          i_clk,
    input  wire                          i_rst,
    input  wire [SYMBOL_NUM-1:0]         i_req,
    input  wire                          i_accept,
    output reg                           o_has_winner,
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

reg [CL_SYMBOL_NUM-1:0] cur_start_ptr;

integer ptr_offset;
reg [CL_SYMBOL_NUM-1:0] candidate_que_idx;

always @(*) begin
    o_has_winner = 1'b0;
    o_que_idx = cur_start_ptr; //place holder
    for (ptr_offset = 0; ptr_offset < SYMBOL_NUM; ptr_offset = ptr_offset + 1) begin
        candidate_que_idx = wrap_que_idx(cur_start_ptr, ptr_offset);
        if (!o_has_winner && i_req[candidate_que_idx]) begin
            o_has_winner = 1'b1;
            o_que_idx = candidate_que_idx;
        end
    end
end

always @(posedge i_clk or posedge i_rst) begin
    if (i_rst) begin
        cur_start_ptr <= {CL_SYMBOL_NUM{1'b0}};
    end else if (i_accept && o_has_winner) begin
        cur_start_ptr <= wrap_que_idx(o_que_idx, 1);
    end
end

endmodule
