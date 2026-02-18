module order_book_parser #(
    parameter DATA_WIDTH = 64,
    parameter CTRL_WIDTH = 8,
    parameter MSG_LEN_BITS = 16
) (
// mac layer rx interface
input   wire                        i_clk_156       ,
input   wire                        i_rst           , // active high
input   wire [DATA_WIDTH-1:0]       i_axi_rx_data   ,
input   wire                        i_axi_rx_valid  ,
input   wire [CTRL_WIDTH-1:0]       i_axi_rx_keep   ,
input   wire                        i_axi_rx_last   ,
output  wire                        o_axi_rx_ready  ,
// order book interface
output  wire [63:0]                 o_seq_num, // sequence number
output  wire [MSG_LEN_BITS-1:0]     o_msg_len       ,
output  wire [7:0]                  o_msg_type      , //A, D, X, U, E, F
output  wire [15:0]                 o_stock_locate  , // the stock ID
output  wire [63:0]                 o_order_ref_num , // order reference number
output  wire [7:0]                  o_buy_sell      , 
output  wire [31:0]                 o_shares        ,
output  wire [31:0]                 o_price         ,
// settings
input   wire [47:0]                 i_ctl_dst_mac          , // filter: only parse packets with this destination port
input   wire [31:0]                 i_ctl_dst_ip           , // active high
input   wire [15:0]                 i_ctl_dst_port         , // filter: only parse packets with this destination port
input   wire                        i_sync_fire        // active high
);
assign o_axi_rx_ready  = 1'b1;
assign o_seq_num       = 64'd0;
assign o_msg_len       = {MSG_LEN_BITS{1'b0}};
assign o_msg_type      = 8'd0;
assign o_stock_locate  = 16'd0;
assign o_order_ref_num = 64'd0;
assign o_buy_sell      = 8'd0;
assign o_shares        = 32'd0;
assign o_price         = 32'd0;



endmodule