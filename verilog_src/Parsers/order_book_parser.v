module order_book_parser #(
    parameter DATA_WIDTH = 64,
    parameter CTRL_WIDTH = 8,
    parameter BAR0_SIZE  = 16
) (
// mac layer rx interface
input   wire                        i_clk_156               ,
input   wire                        i_rst                   , // active high
input   wire [DATA_WIDTH-1:0]       i_axi_rx_data           ,
input   wire                        i_axi_rx_valid          ,
input   wire [CTRL_WIDTH-1:0]       i_axi_rx_keep           ,
input   wire                        i_axi_rx_last           ,
input   wire [63:0]                 i_axi_rx_ingress_tick   ,
output  wire                        o_axi_rx_ready          ,
// order book interface
output  reg                         o_msg_valid             , // 1 when all the parts are parsed.
output  reg [63:0]                  o_seq_num               , // sequence number
output  reg [63:0]                  o_rx_ingress_tick       , // local monotonic RX ingress timestamp
output  reg [7:0]                   o_msg_type              , //A, D, X, U, E, F
output  reg [15:0]                  o_stock_locate          , // the stock ID
output  reg [63:0]                  o_order_ref_num         , // (old, for type u)order reference number
output  reg [63:0]                  o_new_order_ref_num     , // used for type U
output  reg [7:0]                   o_buy_sell              , // 1 for buy, 2 for sell, 0 for others
output  reg [31:0]                  o_shares                ,
output  reg [31:0]                  o_price                 ,
output  reg [47:0]                  o_timestamp             , // timestamp from the packet.
// settings
input   wire [47:0]                 i_ctl_dst_mac           , // filter: only parse packets with this destination port
input   wire [31:0]                 i_ctl_dst_ip            , // active high
input   wire [15:0]                 i_ctl_dst_port          , // filter: only parse packets with this destination port
input   wire                        i_promiscuous           , //  Promiscuous mode
input   wire                        i_sync_fire             ,// active high
input   wire [BAR0_SIZE-1:0]        i_ctl_reg               // control register address for synchronization
);

assign o_axi_rx_ready      = 1'b1;

localparam DEFAULT_MAC_ADDR         = 48'h0100_5E00_0001;
localparam DEFAULT_IP_ADDR          = 32'hE901_0203; // 233.1.2.3
localparam DEFAULT_PORT             = 16'h04d2; // 1234
localparam PROTOCAL                 = 8'h11; // UDP

// === all in decimal ===
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






// control registers
reg [47:0]  preset_dst_mac_addr         ;
reg [31:0]  preset_dst_ip_addr          ;
reg [15:0]  preset_dst_port             ;
reg         promiscuous                 ;
// parsing helper registers
reg [3:0]   head_counter                ;
reg [511:0] buff                        ;// saves previous and current i_axi_rx_data for message parsing, especially for variable-length fields that may cross the boundary of two i_axi_rx_data.
wire [511:0] cur_buff = {buff[447:0], i_axi_rx_data}; 

reg [6:0]   buffed_bytes             ; //how many valid bytes are currently in your window
wire [6:0]  valid_bytes = buffed_bytes < 7'd6 ? 7'd0 : buffed_bytes - 7'd6;
// wire [6:0]  valid_bytes = buffed_bytes ;
// header fields
reg [47:0]  dst_mac_addr                ;
reg [31:0]  dst_ip_addr                 ;
reg [15:0]  dst_port                    ;
reg [15:0]  frame_type                  ;
reg [15:0]  total_len                   ;
reg [7:0]   protocol                    ;
reg [79:0]  session                     ;
reg [15:0]  msg_count                   ;
reg         peeked                      ;




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
                promiscuous            <= i_promiscuous;
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


// header parsing. These will not be output.
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        o_rx_ingress_tick   <= 0;
        dst_mac_addr        <= 0;
        dst_ip_addr         <= 0;
        dst_port            <= 0;
        frame_type          <= 0;
        total_len           <= 0;
        protocol            <= 0;
        session             <= 0;
        o_seq_num           <= 0;
        head_counter        <= 0;
    end else if (i_axi_rx_valid && !i_axi_rx_last) begin
            case (head_counter)
            0: begin
                o_rx_ingress_tick   <= i_axi_rx_ingress_tick;
                dst_mac_addr        <= i_axi_rx_data[63:24];
                head_counter        <= head_counter + 1;
                end
            1: begin
                frame_type          <= i_axi_rx_data[31:16];
                head_counter        <= head_counter + 1;
                end
            2: begin
                total_len           <= i_axi_rx_data[63:48];
                protocol            <= i_axi_rx_data[7:0];
                head_counter        <= head_counter + 1;
                end
            3: begin
                dst_ip_addr [31:16] <= i_axi_rx_data[15:0];
                head_counter        <= head_counter + 1;
                end
            4: begin
               dst_ip_addr [15:0]  <= i_axi_rx_data[63:48];
               dst_port            <= i_axi_rx_data[31:16];
               head_counter        <= head_counter + 1;
               end
            5: begin
                session[79:32]      <= i_axi_rx_data[47:0];
                head_counter        <= head_counter + 1;
                end
            6: begin
                session[31:0]       <= i_axi_rx_data[63:32];
                o_seq_num[63:32]    <= i_axi_rx_data[31:0];
                head_counter        <= head_counter + 1;
                end
            7: begin
                o_seq_num[31:0]     <= i_axi_rx_data[63:32];
                head_counter        <= head_counter+1;
                end
            default : begin
                head_counter        <= head_counter;
            end
            endcase
    end else if (i_axi_rx_last) begin
        head_counter <= 0;
    end
end



// wire have_type              = buffed_bytes >=16 ;
// wire have_type              = buffed_bytes >=8 ;

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        o_msg_type          <= 0;
        o_stock_locate      <= 0;
        o_order_ref_num     <= 0;
        o_new_order_ref_num <= 0;
        o_buy_sell          <= 0;
        o_shares            <= 0;
        o_price             <= 0;
        o_timestamp         <= 0;
        o_msg_valid         <= 0;
        buffed_bytes        <= 7'd8;
        buff                <= 0;
        msg_count           <= 0;
    end else if ((head_counter == 7 || msg_count > 0)) begin // one cycle latency, for data_window to be prepared
            if (head_counter == 7) begin
                msg_count       <= i_axi_rx_data[31:16];
            end
            buff <= {buff[447:0], i_axi_rx_data};
            if (!peeked && head_counter == 8) begin
                // skip length field, 2 bytes, then take 1 byte for type
                o_msg_type <= take_data(cur_buff, valid_bytes-2, 1) & 8'h7F; // the highest bit of msg type is always 0, can be used to check if the parsing is correct.
                peeked <= 1;
            end
            if (o_msg_type != 0) begin
                case (o_msg_type)
                    TYPE_A: begin
                        if (valid_bytes >= msg_len_bytes(TYPE_A)) begin
                            peeked          <= 0;
                            o_msg_valid     <= 1;
                            // skip length field,   2 bytes
                            // skip type field,     1 byte
                            o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                            // skip tracking num, 2 bytes
                            o_timestamp     <= take_data(cur_buff, valid_bytes-7, 6); 
                            o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                            o_buy_sell      <= take_data(cur_buff, valid_bytes-21, 1);
                            o_shares        <= take_data(cur_buff, valid_bytes-22, 4);
                            //skip stock symbol, 8 bytes
                            o_price         <= take_data(cur_buff, valid_bytes-34, 4);
                            buffed_bytes    <= buffed_bytes - msg_len_bytes(TYPE_A) + 8; // one more beat for the next message, so +8
                            msg_count       <= msg_count - 1;
                        end else begin
                            o_msg_valid     <= 0;
                            if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                            else                    buffed_bytes <= 64;
                        end
                    end
                    TYPE_X: begin
                        if (valid_bytes >= msg_len_bytes(TYPE_X)) begin
                            peeked          <= 0;
                            o_msg_valid     <= 1;
                            // skip length field, 2 bytes
                            // skip type field,     1 byte
                            o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                            // skip tracking num, 2 bytes
                            o_timestamp     <= take_data(cur_buff, valid_bytes-7, 6); 
                            o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                            o_shares        <= take_data(cur_buff, valid_bytes-21, 4);
                            buffed_bytes <= buffed_bytes - msg_len_bytes(TYPE_X) + 8; // one more beat for the next message, so +8
                            msg_count       <= msg_count - 1;
                        end else begin
                            o_msg_valid     <= 0;
                            if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                            else                    buffed_bytes <= 64;
                        end
                    end
                    TYPE_D: begin
                        if (valid_bytes >= msg_len_bytes(TYPE_D)) begin
                            peeked          <= 0;
                            o_msg_valid     <= 1;
                            // skip length field, 2 bytes
                            // skip type field,     1 byte
                            o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                            // skip tracking num, 2 bytes
                            o_timestamp     <= take_data(cur_buff, valid_bytes-7, 6); 
                            o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                            buffed_bytes <= buffed_bytes - msg_len_bytes(TYPE_D) + 8; // one more beat for the next message, so +8
                            msg_count       <= msg_count - 1;
                        end else begin
                            o_msg_valid     <= 0;
                            if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                            else                    buffed_bytes <= 64;
                        end
                    end
                    TYPE_U: begin
                        if (valid_bytes >= msg_len_bytes(TYPE_U)) begin
                            peeked          <= 0;
                            o_msg_valid     <= 1;
                            // skip length field, 2 bytes
                            // skip type field,     1 byte
                            o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                            // skip tracking num, 2 bytes
                            o_timestamp     <= take_data(cur_buff, valid_bytes-7, 6); 
                            o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8); // original order ref num
                            o_new_order_ref_num <= take_data(cur_buff, valid_bytes-21, 8);
                            o_shares        <= take_data(cur_buff, valid_bytes-29, 4);
                            o_price         <= take_data(cur_buff, valid_bytes-33, 4);
                            buffed_bytes <= buffed_bytes - msg_len_bytes(TYPE_U) + 8; // one more beat for the next message, so +8
                            msg_count       <= msg_count - 1;
                        end else begin
                            o_msg_valid     <= 0;
                            if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                            else                    buffed_bytes <= 64;
                        end
                    end
                    TYPE_E: begin
                        if (valid_bytes >= msg_len_bytes(TYPE_E)) begin
                            peeked          <= 0;
                            o_msg_valid     <= 1;
                            // skip length field, 2 bytes
                            // skip type field,     1 byte
                            o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                            // skip tracking num, 2 bytes
                            o_timestamp     <= take_data(cur_buff, valid_bytes-7, 6); 
                            o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                            o_shares        <= take_data(cur_buff, valid_bytes-21, 4); // execution shares
                            // skip match num, 8 bytes
                            buffed_bytes <= buffed_bytes - msg_len_bytes(TYPE_E) + 8; // one more beat for the next message, so +8
                            msg_count       <= msg_count - 1;
                        end else begin
                            o_msg_valid     <= 0;
                            if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                            else                    buffed_bytes <= 64;
                        end
                    end
                    TYPE_F: begin
                        if (valid_bytes >= msg_len_bytes(TYPE_F)) begin
                            peeked          <= 0;
                            o_msg_valid     <= 1;
                            // skip length field, 2 bytes
                            // skip type field,     1 byte
                            o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                            // skip tracking num, 2 bytes
                            o_timestamp     <= take_data(cur_buff, valid_bytes-7, 6); 
                            o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                            o_buy_sell      <= take_data(cur_buff, valid_bytes-21, 1);
                            o_shares        <= take_data(cur_buff, valid_bytes-22, 4);
                            //skip stock symbol, 8 bytes
                            o_price         <= take_data(cur_buff, valid_bytes-34, 4);
                            // skip attribution, 4 bytes
                            buffed_bytes <= buffed_bytes - msg_len_bytes(TYPE_F) + 8; // one more beat for the next message, so +8
                            msg_count       <= msg_count - 1;
                        end else begin
                            o_msg_valid     <= 0;
                            if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                            else                    buffed_bytes <= 64;
                        end
                    end
                    default:
                        begin
                            o_msg_valid     <= 0;
                            if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                            else                    buffed_bytes <= 64;
                        end
                endcase
            end else begin
                if (buffed_bytes <= 56) buffed_bytes <= buffed_bytes + 8;
                else                    buffed_bytes <= 64;
            end
    end else begin
        buff            <= 0;
        o_msg_valid     <= 0;
        buffed_bytes    <= 7'd8;
        o_msg_type      <= 0;
        peeked          <= 0;
    end
end





// returns total length of the message in bytes. "Message Length" in the fields does not contain itself.
// Thus, + 2 is appiled to each indicated length.
function automatic [6:0] msg_len_bytes(input [7:0] t);
  case (t)
    TYPE_A: msg_len_bytes   = 7'd38;
    TYPE_F: msg_len_bytes   = 7'd42;
    TYPE_D: msg_len_bytes   = 7'd21;
    TYPE_U: msg_len_bytes   = 7'd37;
    TYPE_E: msg_len_bytes   = 7'd33;
    TYPE_X: msg_len_bytes   = 7'd25;
    default: msg_len_bytes  = 7'd0;
  endcase
endfunction

function automatic [63:0] take_data(
    input [511:0] buff,
    input [6:0] available_bytes,
    input [3:0] width_bytes
);
integer i;
begin
  take_data = 64'd0;
  for (i = 0; i < 8; i = i + 1) begin
    if (i < width_bytes) begin
      // Walk source bytes from high to low index in buff, and keep field byte order.
      take_data[(width_bytes-1-i)*8 +: 8] = buff[(available_bytes-1-i)*8 +: 8];
    end
  end
end
endfunction

endmodule
