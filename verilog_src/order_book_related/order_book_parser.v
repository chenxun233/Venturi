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
input   wire [47:0]                 i_frame_ts              ,
output  wire                        o_axi_rx_ready          ,
// order book interface
output  reg                         o_msg_valid             , // 1 when all the parts are parsed.
output  reg [63:0]                  o_seq_num               , // sequence number
output  reg [7:0]                   o_msg_type              , //A, D, X, U, E, F
output  reg [15:0]                  o_stock_locate          , // the stock ID
output  reg [63:0]                  o_order_ref_num         , // (old, for type u)order reference number
output  reg [63:0]                  o_new_order_ref_num     , // used for type U
output  reg [7:0]                   o_buy_sell              , // 
output  reg [31:0]                  o_shares                ,
output  reg [31:0]                  o_price                 ,
output  reg [47:0]                  o_frame_ts              , // frame time stamp from the logic, not the UDP payload.
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
//======================================
localparam TYPE_A                   = 8'h41;
localparam TYPE_X                   = 8'h58;
localparam TYPE_D                   = 8'h44;
localparam TYPE_U                   = 8'h55;
localparam TYPE_E                   = 8'h45;
localparam TYPE_F                   = 8'h46;
localparam TYPE_C                   = 8'h43;


// TYPE A: add order:       STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + BUY_SELL_LEN + SHARES_LEN + STOCK_SYMBOL_LEN + PRICE_LEN
// TYPE X: order cancel:    STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + CANCEL_SHARE_LEN
// TYPE D: order delete:    STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN
// TYPE U: order replace:   STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORIGINAL_ORDER_REF_NUM_LEN    + NEW_ORDER_REF_NUM_LEN + SHARES_LEN + PRICE_LEN
// TYPE E: execution:       STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + SHARES_LEN + MATCH_NUM_LEN
// TYPE F:                : STOCK_LOCATE_LEN + TRACKING_NUM_LEN + TIMESTAMP_LEN + ORDER_REF_NUM_LEN             + BUY_SELL_LEN + SHARES_LEN + STOCK_SYMBOL_LEN + PRICE_LEN + ATTRIBUTION_LEN
// TYPE C:


// control registers
reg [47:0]  preset_dst_mac_addr         ;
reg [31:0]  preset_dst_ip_addr          ;
reg [15:0]  preset_dst_port             ;
reg         promiscuous                 ;
// parsing helper registers
reg [3:0]   head_counter                ;
reg [511:0] prev_buff                   ;// saves previous and current i_axi_rx_data for message parsing, especially for variable-length fields that may cross the boundary of two i_axi_rx_data.
wire [511:0] cur_buff = {prev_buff[447:0], i_axi_rx_data};  // mix of combinational and sequential logic, makes it one cycle faster than depending on prev_buff only. cur_buff[511:256] is the previous i_axi_rx_data, cur_buff[255:0] is the current i_axi_rx_data.
reg [47:0]  latch_frame_ts             ;// latch the frame timestamp for the current message being parsed, since the timestamp in the payload is not used.

reg [6:0]   buffered_bytes             ; //how many valid bytes are currently in your window
wire [6:0]  valid_bytes = buffered_bytes < 7'd6 ? 7'd0 : buffered_bytes - 7'd6; // there is always a 6-byte gap between the prev_buff and the boundary to parse.
// wire [6:0]  valid_bytes = buffered_bytes ;
// header fields
reg [47:0]  dst_mac_addr                ;
reg [31:0]  dst_ip_addr                 ;
reg [15:0]  dst_port                    ;
reg [15:0]  frame_type                  ;
reg [15:0]  total_len                   ;
reg [7:0]   protocol                    ;
reg [79:0]  session                     ;
reg [15:0]  msg_count                   ;
reg [7:0]   peeked_type                 ; // the type field in the message, which is peeked in advance to determine the length of the message and when a whole message is parsed. This is useful for variable-length messages like type U.
reg [15:0]  msg_len_bytes; // the length of the message in bytes, which is determined by the type field. This is used to determine when a whole message is parsed.




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
// header parsing. These will not be output.
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        // o_rx_ingress_tick   <= 0;
        dst_mac_addr        <= 0;
        dst_ip_addr         <= 0;
        dst_port            <= 0;
        frame_type          <= 0;
        total_len           <= 0;
        protocol            <= 0;
        session             <= 0;
        o_seq_num           <= 0;
        head_counter        <= 0;
        latch_frame_ts     <= 0;
    end else if (i_axi_rx_valid && !i_axi_rx_last) begin
            case (head_counter)
            0: begin
                // o_rx_ingress_tick   <= i_axi_rx_ingress_tick;
                dst_mac_addr        <= i_axi_rx_data[63:24];
                head_counter        <= head_counter + 1;
                latch_frame_ts     <= i_frame_ts;
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
localparam   IDLE               = 5'b00000;
localparam   PARSING_HEADER     = 5'b00001;
localparam   PEEKING_MSG_COUNT  = 5'b00010;
localparam   PEEKING_TYPE       = 5'b00100;
localparam   PARSING_BODY       = 5'b01000;
localparam   ERROR              = 5'b10000;
reg [4:0]    state                       ;
wire [127:0] cur_buff_128 = cur_buff[127:0]; // the first 16 bytes in the current buffer, used for peeking message count and type.
wire [127:0] cur_buff_256 = cur_buff[255:128]; // the first 32 bytes in the current buffer, used for peeking message count and type.

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        state <= IDLE;
        o_msg_type          <= 0;
        o_stock_locate      <= 0;
        o_order_ref_num     <= 0;
        o_new_order_ref_num <= 0;
        o_buy_sell          <= 0;
        o_shares            <= 0;
        o_price             <= 0;
        o_frame_ts          <= 0;
        o_msg_valid         <= 0;
        buffered_bytes      <= 7'd8;  // the buffered_bytes is pure sequential logic, while cur_buff is a mix. To catch up, the it should start from 8.
        prev_buff           <= 0;
        msg_count           <= 0;
        peeked_type         <= 0;
    end else begin
        case (state)
            IDLE: begin
                prev_buff       <= 0;
                o_msg_valid     <= 0;
                buffered_bytes    <= 7'd8;
                o_msg_type      <= 0;
                msg_count       <= 0;
                if (head_counter == 6) begin
                    state <= PEEKING_MSG_COUNT;
                end
            end
            PEEKING_MSG_COUNT: begin
                o_msg_valid     <= 0;
                if (i_axi_rx_valid) begin
                    msg_count       <= i_axi_rx_data[31:16];
                    prev_buff       <= {prev_buff[447:0], i_axi_rx_data};
                    buffered_bytes    <= buffered_bytes + 8;
                    state           <= PEEKING_TYPE;
                end

            end
            PEEKING_TYPE: begin
                o_msg_valid         <= 0;
                o_stock_locate      <= 0;
                o_frame_ts          <= 0;
                o_order_ref_num     <= 0;
                o_new_order_ref_num <= 0;
                o_buy_sell          <= 0;
                o_shares            <= 0;
                o_price             <= 0;
                o_msg_type          <= 0;
                if (msg_count == 0) begin
                    state <= IDLE;
                end else 
                if (i_axi_rx_valid) begin
                    prev_buff       <= {prev_buff[447:0], i_axi_rx_data};
                    msg_len_bytes   <= take_data(cur_buff, valid_bytes, 2); // get the message length
                    peeked_type     <= take_data(cur_buff, valid_bytes-2, 1);
                    buffered_bytes    <= buffered_bytes + 8;
                    state           <= PARSING_BODY;
                end
            end
            PARSING_BODY: begin
                        if (buffered_bytes >= 64) begin
                            state <= ERROR;
                        end else if (valid_bytes >= msg_len_bytes + 16'd2) begin // the + 2 is for the length field itself, which is included in the message length but not in the valid bytes calculation.
                            prev_buff       <= {prev_buff[447:0], i_axi_rx_data};
                            buffered_bytes  <= buffered_bytes - msg_len_bytes - 16'd2 + 16'd8; // one more beat for the next message, so +8
                            state           <= PEEKING_TYPE;
                            // Clear fields that are not present in every message type.
                            case (peeked_type)
                                TYPE_A: begin
                                        o_msg_valid     <= 1;
                                        // skip length field,   2 bytes
                                        // skip type field,     1 byte
                                        o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                                        // skip tracking num, 2 bytes
                                        o_frame_ts      <= latch_frame_ts; // time stamp inside, instead of from the frame.
                                        o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                                        o_buy_sell      <= take_data(cur_buff, valid_bytes-21, 1);
                                        o_shares        <= take_data(cur_buff, valid_bytes-22, 4);
                                        //skip stock symbol, 8 bytes
                                        o_price         <= take_data(cur_buff, valid_bytes-34, 4);
                                        o_msg_type      <= TYPE_A;
                                        msg_count       <= msg_count - 1;
                                    end 
                                TYPE_X: begin
                                        o_msg_valid     <= 1;
                                        // skip length field, 2 bytes
                                        // skip type field,     1 byte
                                        o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                                        // skip tracking num, 2 bytes
                                        o_frame_ts      <= latch_frame_ts; // time stamp inside, instead of from the frame.
                                        o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                                        o_shares        <= take_data(cur_buff, valid_bytes-21, 4);
                                        msg_count       <= msg_count - 1;
                                        o_msg_type      <= TYPE_X;
                                    end 
                                TYPE_D: begin
                                        o_msg_valid     <= 1;
                                        // skip length field, 2 bytes
                                        // skip type field,     1 byte
                                        o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                                        // skip tracking num, 2 bytes
                                        o_frame_ts      <= latch_frame_ts; // time stamp inside, instead of from the frame.
                                        o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                                        msg_count       <= msg_count - 1;
                                        o_msg_type      <= TYPE_D;
                                        // other untouched fields:
                                    end 
                                TYPE_U: begin
                                        o_msg_valid     <= 1;
                                        // skip length field, 2 bytes
                                        // skip type field,     1 byte
                                        o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                                        // skip tracking num, 2 bytes
                                        o_frame_ts      <= latch_frame_ts; // time stamp inside, instead of from the frame.
                                        o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8); // original order ref num
                                        o_new_order_ref_num <= take_data(cur_buff, valid_bytes-21, 8);
                                        o_shares        <= take_data(cur_buff, valid_bytes-29, 4);
                                        o_price         <= take_data(cur_buff, valid_bytes-33, 4);
                                        msg_count       <= msg_count - 1;
                                        o_msg_type      <= TYPE_U;
                                    end 
                                TYPE_E: begin
                                        o_msg_valid     <= 1;
                                        // skip length field, 2 bytes
                                        // skip type field,     1 byte
                                        o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                                        // skip tracking num, 2 bytes
                                        o_frame_ts      <= latch_frame_ts; // time stamp inside, instead of from the frame.
                                        o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                                        o_shares        <= take_data(cur_buff, valid_bytes-21, 4); // execution shares
                                        // skip match num, 8 bytes
                                        msg_count       <= msg_count - 1;
                                        o_msg_type      <= TYPE_E;
                                    end 
                                TYPE_F: begin
                                        o_msg_valid     <= 1;
                                        // skip length field, 2 bytes
                                        // skip type field,     1 byte
                                        o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                                        // skip tracking num, 2 bytes
                                        o_frame_ts      <= latch_frame_ts; // time stamp inside, instead of from the frame.
                                        o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                                        o_buy_sell      <= take_data(cur_buff, valid_bytes-21, 1);
                                        o_shares        <= take_data(cur_buff, valid_bytes-22, 4);
                                        //skip stock symbol, 8 bytes
                                        o_price         <= take_data(cur_buff, valid_bytes-34, 4);
                                        // skip attribution, 4 bytes
                                        msg_count       <= msg_count - 1;
                                        o_msg_type      <= TYPE_F;
                                    end
                                TYPE_C: begin
                                        o_msg_valid     <= 1;
                                        // skip length field, 2 bytes
                                        // skip type field,     1 byte
                                        o_stock_locate  <= take_data(cur_buff, valid_bytes-3, 2);
                                        // skip tracking num, 2 bytes
                                        o_frame_ts      <= latch_frame_ts; // time stamp inside, instead of from the frame.
                                        o_order_ref_num <= take_data(cur_buff, valid_bytes-13, 8);
                                        o_shares        <= take_data(cur_buff, valid_bytes-21, 4);
                                        //skip match number, 8 bytes
                                        //skip Printable   , 1 byte 
                                        o_price         <= take_data(cur_buff, valid_bytes-34, 4);
                                        // skip attribution, 4 bytes
                                        msg_count       <= msg_count - 1;
                                        o_msg_type      <= TYPE_C;
                                    end
                                default: begin
                                        o_msg_valid         <= 0;
                                        o_stock_locate      <= 0;
                                        o_frame_ts          <= 0;
                                        o_order_ref_num     <= 0;
                                        o_new_order_ref_num <= 0;
                                        o_buy_sell          <= 0;
                                        o_price             <= 0;
                                        o_shares            <= 0;
                                        msg_count           <= msg_count - 1;
                                        o_msg_type          <= 0;
                                end
                            endcase
                        end else begin
                            prev_buff       <= {prev_buff[447:0], i_axi_rx_data};
                            buffered_bytes  <= buffered_bytes + 8;
                            o_msg_valid         <= 0;
                            o_msg_type          <= 0;
                            o_stock_locate      <= 0;
                            o_frame_ts          <= 0;
                            o_order_ref_num     <= 64'd0;
                            o_new_order_ref_num <= 64'd0;
                            o_buy_sell          <= 8'd0;
                            o_shares            <= 32'd0;
                            o_price             <= 32'd0;
                        end
                    
            end
            ERROR: begin
                // stay in this state until the end of the packet, which will be indicated by i_axi_rx_last, then go back to IDLE.
                if (i_axi_rx_last) begin
                    state <= IDLE;
                end
            end
            default: begin
            state <= IDLE;
            end
            
        endcase
    end
end

// returns total length of the message in bytes. "Message Length" in the fields does not contain itself.
// Thus, + 2 is appiled to each indicated length.
// function automatic [6:0] msg_len_bytes(input [7:0] t);
//   case (t)
//     TYPE_A: msg_len_bytes   = 7'd38;
//     TYPE_F: msg_len_bytes   = 7'd42;
//     TYPE_D: msg_len_bytes   = 7'd21;
//     TYPE_U: msg_len_bytes   = 7'd37;
//     TYPE_E: msg_len_bytes   = 7'd33;
//     TYPE_X: msg_len_bytes   = 7'd25;
//     TYPE_C: msg_len_bytes   = 7'd38;
//     default: msg_len_bytes  = 7'd0;
//   endcase
// endfunction

function automatic [63:0] take_data(
    input [511:0] prev_buff,
    input [6:0] index_bytes, // the start byte index (highest) of one field. 
    input [3:0] width_bytes
);
integer i;
begin
  take_data = 64'd0;
  for (i = 0; i < 8; i = i + 1) begin
    if (i < width_bytes) begin
      // Walk source bytes from high to low index in prev_buff, and keep field byte order.
      take_data[(width_bytes-i)*8-1 -: 8] = prev_buff[(index_bytes-i)*8-1 -: 8];
    end
  end
end
endfunction

endmodule
