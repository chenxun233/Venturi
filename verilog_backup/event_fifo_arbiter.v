module event_fifo_arbiter #(
    parameter SYMBOL_NUM = 2,
    parameter PAYLOAD_W  = 274
) (
    input  wire                          i_clk_156,
    input  wire                          i_rst,
    input  wire                          i_ff_pop,
    input  wire [SYMBOL_NUM-1:0]         i_src_not_empty,
    input  wire [SYMBOL_NUM-1:0]         i_src_valid,
    input  wire [SYMBOL_NUM*PAYLOAD_W-1:0] i_src_payload,
    output wire [SYMBOL_NUM-1:0]         o_src_pop,
    output wire                          o_not_empty,
    output wire                          o_has_winner,
    output wire [PAYLOAD_W-1:0]          o_payload
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

localparam POINTER_WIDTH    = (SYMBOL_NUM > 1) ? clog2(SYMBOL_NUM) : 1; // bit width.
localparam integer LAST_IDX = SYMBOL_NUM - 1;

function [POINTER_WIDTH-1:0] wrap_idx (input [31:0] base, input integer offset);
    integer tmp;
    begin
        tmp = offset;
        tmp = tmp + base;
        if (tmp >= SYMBOL_NUM) begin
            tmp = tmp - SYMBOL_NUM;
        end
        wrap_idx = tmp[POINTER_WIDTH-1:0];
    end
endfunction

reg  [POINTER_WIDTH-1:0] rr_ptr_q;
reg  [POINTER_WIDTH-1:0] grant_idx_q;
reg  [POINTER_WIDTH-1:0] grant_idx_c;
reg  [POINTER_WIDTH-1:0] scan_idx_c;
reg              have_grant_c;

integer i;

always @(*) begin
    grant_idx_c  = rr_ptr_q;
    scan_idx_c   = rr_ptr_q;
    have_grant_c = 1'b0;

    for (i = 0; i < SYMBOL_NUM; i = i + 1) begin
        scan_idx_c = wrap_idx({{(32-POINTER_WIDTH){1'b0}}, rr_ptr_q}, i);   // {0000,{pointer}}
        if (!have_grant_c && i_src_not_empty[scan_idx_c]) begin
            grant_idx_c  = scan_idx_c;
            have_grant_c = 1'b1;
        end
    end
end

assign o_not_empty = |i_src_not_empty;

genvar src_idx;
generate
    for (src_idx = 0; src_idx < SYMBOL_NUM; src_idx = src_idx + 1) begin : g_src_pop
        assign o_src_pop[src_idx] = i_ff_pop && have_grant_c && (grant_idx_c == src_idx[POINTER_WIDTH-1:0]);
    end
endgenerate

assign o_has_winner   = i_src_valid[grant_idx_q];
assign o_payload = i_src_payload[grant_idx_q*PAYLOAD_W +: PAYLOAD_W];

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        rr_ptr_q    <= {POINTER_WIDTH{1'b0}};
        grant_idx_q <= {POINTER_WIDTH{1'b0}};
    end else if (i_ff_pop && have_grant_c) begin
        grant_idx_q <= grant_idx_c;
        if (grant_idx_c == LAST_IDX[POINTER_WIDTH-1:0]) begin
            rr_ptr_q <= {POINTER_WIDTH{1'b0}};
        end else begin
            rr_ptr_q <= grant_idx_c + {{(POINTER_WIDTH-1){1'b0}}, 1'b1};
        end
    end
end

endmodule
