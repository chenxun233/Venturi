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

function integer clog2;
    input integer value;
    integer i;
    begin
        clog2 = 0;
        for (i = value - 1; i > 0; i = i >> 1)
            clog2 = clog2 + 1;
    end
endfunction

localparam POINTER_WIDTH = (SYMBOL_NUM > 1) ? clog2(SYMBOL_NUM) : 1;
localparam integer LAST_IDX = SYMBOL_NUM - 1;

function [POINTER_WIDTH-1:0] wrap_idx;
    input [31:0] base;
    input integer offset;
    integer tmp;
    begin
        tmp = offset + base;
        if (tmp >= SYMBOL_NUM) begin
            tmp = tmp - SYMBOL_NUM;
        end
        wrap_idx = tmp[POINTER_WIDTH-1:0];
    end
endfunction

reg [POINTER_WIDTH-1:0] rr_pointer;
reg [POINTER_WIDTH-1:0] latch_pointer;
reg [POINTER_WIDTH-1:0] pointer;
reg [POINTER_WIDTH-1:0] scan_idx;
reg                     found_nonempty;

integer i;

always @(*) begin
    pointer        = rr_pointer;
    scan_idx       = rr_pointer;
    found_nonempty = 1'b0;

    for (i = 0; i < SYMBOL_NUM; i = i + 1) begin
        scan_idx = wrap_idx({{(32-POINTER_WIDTH){1'b0}}, rr_pointer}, i);
        if (!found_nonempty && i_src_not_empty[scan_idx]) begin
            pointer        = scan_idx;
            found_nonempty = 1'b1;
        end
    end
end

genvar src_idx;
generate
    for (src_idx = 0; src_idx < SYMBOL_NUM; src_idx = src_idx + 1) begin : g_src_pop
        assign o_src_pop[src_idx] = found_nonempty && (pointer == src_idx[POINTER_WIDTH-1:0]);
    end
endgenerate

assign o_valid   = i_src_valid[latch_pointer];
assign o_payload = i_src_payload[latch_pointer*PAYLOAD_W +: PAYLOAD_W];

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        rr_pointer    <= {POINTER_WIDTH{1'b0}};
        latch_pointer <= {POINTER_WIDTH{1'b0}};
    end else if (found_nonempty) begin
        latch_pointer <= pointer;
        if (pointer == LAST_IDX[POINTER_WIDTH-1:0]) begin
            rr_pointer <= {POINTER_WIDTH{1'b0}};
        end else begin
            rr_pointer <= pointer + {{(POINTER_WIDTH-1){1'b0}}, 1'b1};
        end
    end
end

endmodule
