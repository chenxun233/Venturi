module order_book_builder #(
    parameter STOCK_BITS       = 6,     // number of tracked stocks = 2^STOCK_BITS
    parameter PRICE_BITS       = 10,    // number of price levels per stock = 2^PRICE_BITS
    parameter SYMBOL_NUM       = 1,     // How many symbols to track.
    parameter ORDER_ENTRY_W    = 82     // 1+16+1+32+32 = 82 bits per order entry: valid, stock_locate, side, price, shares_remaining
) (
    // order book parser interface
    input   wire                        i_clk_156,
    input   wire                        i_rst,               // active high
    input   wire                        i_msg_valid,
    input   wire [63:0]                 i_seq_num,
    input   wire [63:0]                 i_rx_ingress_tick,
    input   wire [7:0]                  i_msg_type,          // A, D, X, U, E, F
    input   wire [15:0]                 i_stock_locate,
    input   wire [63:0]                 i_order_ref_num,     // old order_ref for U
    input   wire [63:0]                 i_new_order_ref_num, // used for U
    input   wire [7:0]                  i_buy_sell,          // 'B' or 'S' for A/F
    input   wire [31:0]                 i_shares,
    input   wire [31:0]                 i_price,
    input   wire [47:0]                 i_timestamp,
    // control plane interface
    input   wire                        i_ctl_fire           // fire signal from control plane to trigger actions
);

localparam NULL = 2'd0;
localparam BUY  = 2'd1;
localparam SELL = 2'd2;

reg [1:0] side;

always @(*) begin
    case (i_buy_sell)
        8'h42:      side = BUY;  // 'B'
        8'h53:      side = SELL; // 'S'
        default:    side = NULL;
    endcase
end


symbol_book #(
    .SHARE_PER_PRICE_BIT(32), // number of bits to represent shares quantity at each price level
    .PRICE_ADDR_WIDTH(10),
    .BOOK_ADDR_WIDTH (12 ),
    .PRICE_BASE      (32'd0000_0000   ), // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    .STOCK_LOCATE    (16'h000d       )  // stock locate for this symbol book, assigned by control plane
) symbol_book_inst (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_msg_valid        (i_msg_valid        ),
    .i_rx_ingress_tick  (i_rx_ingress_tick  ),
    .i_msg_type         (i_msg_type         ),
    .i_stock_locate     (i_stock_locate     ),
    .i_order_ref_num    (i_order_ref_num    ),
    .i_new_order_ref_num(i_new_order_ref_num),
    .i_side             (side               ),
    .i_shares           (i_shares           ),
    .i_price            (i_price            ),
    .i_timestamp        (i_timestamp        )
);

endmodule
