module arbiter #(
    parameter SYMBOL_NUM = 2,
    parameter PAYLOAD_W  = 274
) (
    input  wire                             i_clk_156,
    input  wire                             i_rst,
    input  wire [SYMBOL_NUM-1:0]            i_src_not_empty,
    input  wire [SYMBOL_NUM-1:0]            i_src_valid,
    input  wire [SYMBOL_NUM*PAYLOAD_W-1:0]  i_src_payload,
    output wire [SYMBOL_NUM-1:0]            o_src_pop,
    output wire                             o_valid,
    output wire [PAYLOAD_W-1:0]             o_payload
);


localparam POINTER_WIDTH    = (SYMBOL_NUM > 1) ? clog2(SYMBOL_NUM) : 1; // bit width.

reg  [POINTER_WIDTH-1:0] pointer;
reg                      found_nonempty;
reg [POINTER_WIDTH-1:0]  latch_pointer;

integer i;

always @(*) begin // scan from latch pointer to find the next non-empty source
    found_nonempty = 1'b0;
    for (i = latch_pointer; i < SYMBOL_NUM+latch_pointer; i = i + 1) begin
        if (i_src_not_empty[idx_wrapper(i)] && !found_nonempty) begin
            pointer = idx_wrapper(i);
            found_nonempty = 1'b1;
        end
    end
end


// just output 
assign o_src_pop = (found_nonempty) ? (1 << pointer) : 0;       
assign o_valid   = i_src_valid[pointer];
assign o_payload = i_src_payload[pointer*PAYLOAD_W+:PAYLOAD_W];

// latch the pointer to avoid starvation of other sources
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        latch_pointer <= 0;
    end else if ( found_nonempty) begin
        latch_pointer <= pointer;
    end 
end


// wrapper function to handle pointer wrapping
function idx_wrapper (input [POINTER_WIDTH-1:0] pointer);
    if (pointer > SYMBOL_NUM - 1) begin
        idx_wrapper = pointer - SYMBOL_NUM;
    end else begin
        idx_wrapper = pointer;
    end
endfunction

function integer clog2 (input integer value);
    integer i;
    begin
        clog2 = 0;
        for (i = value - 1; i > 0; i = i >> 1)
            clog2 = clog2 + 1;
    end
endfunction

endmodule