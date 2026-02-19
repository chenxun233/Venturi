module order_book_parser #(
    parameter DATA_WIDTH = 64,
    parameter CTRL_WIDTH = 8,
    parameter BAR0_SIZE  = 16
) (
// mac layer rx interface
input   wire                        i_clk_156       ,
input   wire                        i_rst           , // active high
input   wire [DATA_WIDTH-1:0]       i_axi_rx_data   ,
input   wire                        i_axi_rx_valid  ,
input   wire [CTRL_WIDTH-1:0]       i_axi_rx_keep   ,
input   wire                        i_axi_rx_last   ,
output  reg                         o_axi_rx_ready  ,
// order book interface
output  reg                         o_done              , // 1 when all the parts are parsed.
output  reg [63:0]                  o_seq_num           , // sequence number
output  reg [7:0]                   o_msg_type          , //A, D, X, U, E, F
output  reg [15:0]                  o_stock_locate      , // the stock ID
output  reg [63:0]                  o_order_ref_num     , // (old, for type u)order reference number
output  reg [63:0]                  o_new_order_ref_num , // used for type U
output  reg [7:0]                   o_buy_sell          , // 1 for buy, 2 for sell, 0 for others
output  reg [31:0]                  o_shares            ,
output  reg [31:0]                  o_price             ,
// settings
input   wire [47:0]                 i_ctl_dst_mac   , // filter: only parse packets with this destination port
input   wire [31:0]                 i_ctl_dst_ip    , // active high
input   wire [15:0]                 i_ctl_dst_port  , // filter: only parse packets with this destination port
input   wire                        i_promiscuous   , //  Promiscuous mode
input   wire                        i_sync_fire     ,// active high
input   wire [BAR0_SIZE-1:0]        i_ctl_reg       // control register address for synchronization
);


localparam DEFAULT_MAC_ADDR         = 48'h0100_5E00_0001;
localparam DEFAULT_IP_ADDR          = 32'hE901_0203; // 233.1.2.3
localparam DEFAULT_PORT             = 16'h04d2; // 1234
localparam PROTOCAL                 = 8'h11; // UDP

// === all in decimal ===
localparam MAX_MSG_LEN_BIT          = 4 ; // maximum message length in bytes
// === Ethernet header fields ===   
localparam DST_MAC_LEN              = 6 ;
localparam SRC_MAC_LEN              = 6 ;
localparam TYPE_LEN                 = 2 ;
// === IP header fields     === 
localparam VERSION_HEADER_LEN       = 1 ;
localparam ToS_LEN                  = 1 ;
localparam TOTAL_LEN_LEN            = 2 ; // From IP header to the end, min 20 (No UDP and PAYLOAD).
localparam IDENTIFICATION_LEN       = 2 ;
localparam FLAGS_FRAG_OFFSET_LEN    = 2 ;
localparam TTL_LEN                  = 1 ;
localparam PROTOCOL_LEN             = 1 ;
localparam HEADER_CHECKSUM_LEN      = 2 ;
localparam SRC_IP_LEN               = 4 ;
localparam DST_IP_LEN               = 4 ;
// === UDP header fields    === 
localparam SRC_PORT_LEN             = 2 ;
localparam DST_PORT_LEN             = 2 ;
localparam LENGTH_LEN               = 2 ;
localparam CHECKSUM_LEN             = 2 ;
// === order book message header fields ===
localparam SESSION_LEN              = 10;
localparam SEQ_NUM_LEN              = 8 ;
localparam MSG_COUNT_LEN            = 2 ;
localparam MSG_LENTH_LEN            = 2 ;
localparam MSG_TYPE_LEN             = 1 ;
// === order book message body fields===
localparam STOCK_LOCATE_LEN         = 2 ;
localparam TRACKING_NUM_LEN         = 2 ;
localparam TIMESTAMP_LEN            = 6 ;
localparam ORDER_REF_NUM_LEN        = 8 ; // ORIGINAL, NEW
localparam BUY_SELL_LEN             = 1 ;
localparam SHARES_LEN               = 4 ;
localparam STOCK_SYMBOL_LEN         = 8 ;
localparam PRICE_LEN                = 4 ;
localparam CANCEL_SHARE_LEN         = 4 ;
localparam MATCH_NUM_LEN            = 8 ;
localparam ATTRIBUTION_LEN          = 4 ;
//======================================
localparam TYPE_A                   = 8'h41;
localparam TYPE_X                   = 8'h58;
localparam TYPE_D                   = 8'h44;
localparam TYPE_U                   = 8'h55;
localparam TYPE_E                   = 8'h45;
localparam TYPE_F                   = 8'h46;


// TYPE A: add order:       STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + BUY_SELL_LEN + SHARES_LEN + STOCK_SYMBOL_LEN + PRICE_LEN
// TYPE X: order cancel:    STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + CANCEL_SHARE_LEN
// TYPE D: order delete:    STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN
// TYPE U: order replace:   STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORIGINAL_ORDER_REF_NUM_LEN    + NEW_ORDER_REF_NUM_LEN + SHARES_LEN + PRICE_LEN
// TYPE E: execution:       STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + SHARES_LEN + MATCH_NUM_LEN
// TYPE F:                : STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + BUY_SELL_LEN + SHARES_LEN + STOCK_SYMBOL_LEN + PRICE_LEN + ATTRIBUTION_LEN







reg [47:0]  preset_dst_mac_addr         ;
reg [31:0]  preset_dst_ip_addr          ;
reg [15:0]  preset_dst_port             ;
reg         promiscuous                 ;
reg [MAX_MSG_LEN_BIT-1:0] msg_counter   ;

reg [47:0]  dst_mac_addr                ;
reg [31:0]  dst_ip_addr                 ;
reg [15:0]  dst_port                    ;
reg [15:0]  type                        ;
reg [15:0]  total_len                   ;
reg [7:0]   protocol                    ;
reg [79:0]  session                     ;



reg         discard                     ; // flag to indicate whether to discard the current packet

// === settings synchronization ===
always@(posedge i_clk_156 or posedge i_rst or posedge i_sync_fire) begin
    if(i_rst) begin
        preset_dst_mac_addr    <= DEFAULT_MAC_ADDR;
        preset_dst_ip_addr     <= DEFAULT_IP_ADDR;
        preset_dst_port        <= DEFAULT_PORT;
        promiscuous            <= 1'b1;
    end else if(i_sync_fire) begin
        case (i_ctl_reg)
            16'h04: begin
                preset_dst_mac_addr    <= i_ctl_dst_mac;
            end
            16'h08: begin
                preset_dst_ip_addr     <= i_ctl_dst_ip;
            end
            16'h0C: begin
                preset_dst_port        <= i_ctl_dst_port;
            end
            16'h10: begin
                promiscuous <= i_promiscuous;
            end
            default: begin
                // do nothing for other addresses
            end
        endcase
    end
end
//  ==== packet parsing logic ====
// BEAT:
// 0: DST_MAC [47:0] + SRC_MAC [47:32]
// 1: SRC_MAC [31:0] + TYPE [15:0] + VERSION_HEADER [7:0] + ToS [7:0]
// 2: TOTAL_LEN [15:0] + IDENTIFICATION [15:0] + FLAGS_FRAG_OFFSET [15:0] + TTL [7:0] + PROTOCOL [7:0] 
// 3: HEADER_CHECKSUM [15:0] + SRC_IP [31:0] + DST_IP [31:16]
// 4. DST_IP [15:0] + SRC_PORT [15:0] + DST_PORT [15:0] + LENGTH [15:0]
// 5. CHECKSUM [15:0] + SESSION [79:32]
// 6. SESSION [31:0] + SEQ_NUM [63:32]
// 7. SEQ_NUM [31:0] + MSG_COUNT [15:0] + MSG_LENTH [15:0]
// 8. MSG_TYPE [7:0] + STOCK_LOCATE [15:0] + TRACKING_NUM [15:0] + TIMESTAMP [47:24]
// 9. TIMESTAMP [23:0] + ORDER_REF_NUM [63:24]

// TYPE A
// 10. ORDER_REF_NUM [23:0] + o_buy_sell [7:0] + o_shares [31:0]
// 11. STOCK_SYMBOL_LEN []
// 12. PRICE [31:0]

// TYPE X
// 10. ORDER_REF_NUM [23:0] + SHARE_LEN [31:0]

// TYPE D
// 10. ORDER_REF_NUM [23:0] + nothing

// TYPE U
// 10. ORDER_REF_NUM [23:0] + NEW_ORDER_REF_NUM [63:24]
// 11. NEW_ORDER_REF_NUM [23:0] + shares [31:0] + price [31:24]
// 12. price [23:0]

// TYPE E
// 10. ORDER_REF_NUM [23:0] + SHARES [31:0] + MATCH_NUM [63:56]
// 11. MATCH_NUM [55:0]

// TYPE F
// 10. ORDER_REF_NUM [23:0] + o_buy_sell [7:0] + o_shares [31:0]
// 11. STOCK_SYMBOL_LEN []
// 12. PRICE [31:0] + ATTRIBUTION [31:0]
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        msg_counter         <= 0;
        discard             <= 1'b0;
        o_axi_rx_ready      <= 1'b1;
        o_seq_num           <= 64'd0;
        o_msg_type          <= 8'd0;
        o_stock_locate      <= 16'd0;
        o_order_ref_num     <= 64'd0;
        o_new_order_ref_num <= 64'd0;
        o_buy_sell          <= 8'd0;
        o_shares            <= 32'd0;
        o_price             <= 32'd0;
    end else begin
        if (i_axi_rx_valid) begin
            case (msg_counter)
            0: begin
                dst_mac_addr        <= i_axi_rx_data[63:24];
                end
            1: begin
                type                <= i_axi_rx_data[31:16];
                end
            2: begin
                total_len           <= i_axi_rx_data[63:48];
                protocol            <= i_axi_rx_data[7:0];
                end
            3: begin
                dst_ip_addr [31:16] <= i_axi_rx_data[15:0];
                end
             4: begin
                dst_ip_addr [15:0]  <= i_axi_rx_data[63:48];
                dst_port            <= i_axi_rx_data[31:16];
                end
             5: begin
                session[79:32]      <= i_axi_rx_data[47:0];
                end
            6: begin
                session[31:0]       <= i_axi_rx_data[63:32];
                o_seq_num[63:32]    <= i_axi_rx_data[31:0];
                end
            7: begin
                o_seq_num[31:0]     <= i_axi_rx_data[63:32];
                end
            8: begin
                o_msg_type          <= i_axi_rx_data[63:56];
                o_stock_locate      <= i_axi_rx_data[55:40];
                end
            9: begin
                o_order_ref_num[63:24] <= i_axi_rx_data[47:0];
                end
            10: begin
                o_order_ref_num[23:0]       <= i_axi_rx_data[63:40];
                o_buy_sell                  <= i_axi_rx_data[39:32];
                o_new_order_ref_num[63:24]  <= i_axi_rx_data[39:0];
                case (o_msg_type)
                    TYPE_A, TYPE_F: begin
                        o_shares            <= i_axi_rx_data[31:0];
                    end
                    TYPE_X, TYPE_E: begin
                        o_shares            <= i_axi_rx_data[39:8];
                    end
                endcase
                end
            11: begin
                o_new_order_ref_num[23:0]  <= i_axi_rx_data[63:40];
                case (o_msg_type)
                    TYPE_U: begin
                        o_shares           <= i_axi_rx_data[39:8];
                        o_price[31:24]     <= i_axi_rx_data[7:0];
                    end
                endcase
                end
            12: begin
                case (o_msg_type)
                    TYPE_A, TYPE_F: begin
                        o_price             <= i_axi_rx_data[63:32];
                    end
                    TYPE_U: begin
                        o_price[23:0]       <= i_axi_rx_data[63:40];
                    end
                    default: begin
                        // do nothing for other types
                    end
                endcase
                end
            endcase
            // when done, reset the counter.
            if (o_done || i_axi_rx_last) begin
                msg_counter <= 0;
            end else if (msg_counter == 4'd14) begin
                msg_counter <= 0;
            end else begin
                msg_counter <= msg_counter+1; // hold until end-of-frame
            end
        end
    end
end

// === o_done generation ===
always @(posedge i_clk_156) begin
    if (i_rst) begin
        o_done <= 1'b0;
    end
    else if (promiscuous || (dst_mac_addr == preset_dst_mac_addr && dst_ip_addr == preset_dst_ip_addr && dst_port == preset_dst_port )) begin
        case (o_msg_type)
            TYPE_A, TYPE_U, TYPE_F: begin
                if (msg_counter == 12) begin //
                    o_done <= 1'b1;
                end
            end
            TYPE_X, TYPE_D: begin
                if (msg_counter == 10) begin
                    o_done <= 1'b1;
                end
            end
            TYPE_E: begin
                if (msg_counter == 11) begin
                    o_done <= 1'b1;
                end
            end
            default: begin
                    o_done <= 1'b0;
            end
        endcase
    end
    if (o_done) begin
        o_done <= 1'b0; // reset o_done after one cycle
    end
end
endmodule
