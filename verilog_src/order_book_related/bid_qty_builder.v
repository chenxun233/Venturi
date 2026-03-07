module bid_qty_builder #(
    parameter QTY_MSG_BIT       = 2+32+1+32, // {o_bid_ask, price, is_add, d_shares}
    parameter QTY_PRICE_LVL_BIT = 10,        // trace 2^QTY_PRICE_LVL_BIT price levels
    parameter QTY_SHARE_BIT     = 32,        // number of bits to represent shares quantity at each price level
    parameter PRICE_BASE        = 32'd0
)(
    input   wire                          i_clk_156,
    input   wire                          i_rst,               // active high
    input   wire [QTY_MSG_BIT-1:0]        i_qty_msg,
    output  reg  [QTY_PRICE_LVL_BIT-1:0]  o_bid_price_idx,
    output  reg                           o_bid_price_changed
);
// o_price_idx indicating the price idx whose share changed between empty and non-empty.
// o_price_changed is a pulse indicating any price idx change, used to trigger bid_tree update.


reg [1:0]                                qty_upd_state;
reg [1:0]                                qty_op;
reg [QTY_SHARE_BIT-1:0]                  qty_i_shares;

localparam IDLE                          = 2'b00;
localparam FIRST_CYCLE                   = 2'b01;
localparam SECOND_CYCLE                  = 2'b10;
localparam READ                          = 2'b01;
localparam WRITE                         = 2'b10;
localparam BID                           = 2'b01;
localparam PRICE_DEPTH                   = 1 << QTY_PRICE_LVL_BIT;
wire [1:0]                  qty_bid_ask         = i_qty_msg[QTY_MSG_BIT-1:QTY_MSG_BIT-2];
wire                        ff_push             = qty_bid_ask == BID;
wire                        ff_not_empty;
wire                        ff_pop;
wire [QTY_MSG_BIT-1:0]      ff_o_qty_msg;
wire                        ff_o_valid;

wire [31:0]                 qty_price           = ff_o_qty_msg[QTY_MSG_BIT-3:QTY_MSG_BIT-34];
wire                        qty_is_add          = ff_o_qty_msg[QTY_MSG_BIT-35];
wire [31:0]                 qty_d_shares        = ff_o_qty_msg[QTY_MSG_BIT-36:QTY_MSG_BIT-67];

reg [QTY_PRICE_LVL_BIT-1:0] latch_qty_prc_idx;
reg                         latch_qty_is_add;
reg [QTY_SHARE_BIT-1:0]     latch_qty_d_shares;

wire [31:0]                 qty_o_shares;
wire [QTY_SHARE_BIT-1:0]    qty_cur             = qty_o_shares;
wire [QTY_SHARE_BIT-1:0]    qty_new             = latch_qty_is_add ?
                                                  (qty_cur + latch_qty_d_shares) :
                                                  ((qty_cur > latch_qty_d_shares) ? (qty_cur - latch_qty_d_shares) : {QTY_SHARE_BIT{1'b0}});
wire [QTY_PRICE_LVL_BIT-1:0] qty_addr           = latch_qty_prc_idx;

assign ff_pop = ff_not_empty && (qty_upd_state == IDLE) && !ff_o_valid;

fifo #(
    .DEPTH          (8),
    .DATA_W         (QTY_MSG_BIT)
) bid_qty_msg_fifo_inst (
    .i_clk          (i_clk_156),
    .i_rst          (i_rst),
    .i_do_push      (ff_push),
    .o_push_ready   (       ),
    .i_data         (i_qty_msg),
    .i_do_pop       (ff_pop),
    .o_not_empty    (ff_not_empty),
    .o_valid        (ff_o_valid),
    .o_data         (ff_o_qty_msg)
);

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        latch_qty_prc_idx    <= {QTY_PRICE_LVL_BIT{1'b0}};
        latch_qty_is_add     <= 1'b0;
        latch_qty_d_shares   <= {QTY_SHARE_BIT{1'b0}};
    end else if (ff_o_valid) begin
        latch_qty_prc_idx    <= cal_qty_book_addr(qty_price);
        latch_qty_is_add     <= qty_is_add;
        latch_qty_d_shares   <= qty_d_shares;
    end
end

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        qty_upd_state        <= IDLE;
        qty_op               <= IDLE;
        qty_i_shares         <= {QTY_SHARE_BIT{1'b0}};
    end else begin
        case (qty_upd_state)
            IDLE: begin
                if (ff_o_valid) begin
                    qty_op        <= READ;
                    qty_upd_state <= FIRST_CYCLE;
                end else begin
                    qty_op        <= IDLE;
                    qty_upd_state <= IDLE;
                    qty_i_shares  <= {QTY_SHARE_BIT{1'b0}};
                end
            end
            FIRST_CYCLE: begin
                qty_upd_state     <= SECOND_CYCLE;
            end
            SECOND_CYCLE: begin
                qty_op            <= WRITE;
                qty_i_shares      <= qty_new;
                qty_upd_state     <= IDLE;
            end
            default: begin
                qty_upd_state     <= IDLE;
                qty_i_shares      <= {QTY_SHARE_BIT{1'b0}};
                qty_op            <= IDLE;
            end
        endcase
    end
end









bram #(
    .ADDR_WIDTH     (QTY_PRICE_LVL_BIT),
    .DATA_WIDTH     (QTY_SHARE_BIT)
) bid_qty_bram_inst (
    .i_clk          (i_clk_156),
    .i_rst          (i_rst),
    .i_addr         (qty_addr),
    .i_op           (qty_op),
    .i_data         (qty_i_shares),
    .o_data         (qty_o_shares)
);

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        o_bid_price_idx          <= IDLE;
        o_bid_price_changed        <= IDLE;
    end else if (qty_upd_state == SECOND_CYCLE) begin
        if ((qty_cur == 0 && qty_new > 0)|| (qty_cur > 0 && qty_new == 0)) begin
            // empty -> non-empty
            o_bid_price_idx      <= qty_addr;
            o_bid_price_changed  <= 1'b1;
        end else begin
            o_bid_price_idx      <= 0;
            o_bid_price_changed  <= 1'b0;
        end
    end else begin
        o_bid_price_idx          <= 0;
        o_bid_price_changed      <= 1'b0;
    end
end


function [QTY_PRICE_LVL_BIT-1:0] cal_qty_book_addr;
    input [31:0] price;
    reg [29:0] price_offset_u30;
begin
    if (price >= PRICE_BASE) begin
        price_offset_u30 = (price - PRICE_BASE) >> 2;
        if (price_offset_u30 < PRICE_DEPTH) begin
            cal_qty_book_addr = price_offset_u30[QTY_PRICE_LVL_BIT-1:0];
        end else begin
            cal_qty_book_addr = {QTY_PRICE_LVL_BIT{1'b0}};
        end
    end else begin
        cal_qty_book_addr = {QTY_PRICE_LVL_BIT{1'b0}};
    end
end
endfunction

endmodule