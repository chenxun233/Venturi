module rx_dma_stage #(
    parameter SYMBOL_NUM = 2,
    parameter PAYLOAD_W  = 192
) (
    input  wire                             i_clk,
    input  wire                             i_rst,
    input  wire [47:0]                      i_dma_timestamp,
    input  wire [SYMBOL_NUM-1:0]            i_event_empty,
    input  wire [SYMBOL_NUM-1:0]            i_event_valid,
    input  wire [SYMBOL_NUM*PAYLOAD_W-1:0]  i_event_payload,
    output reg  [SYMBOL_NUM-1:0]            o_event_pop,
    input  wire [SYMBOL_NUM*64-1:0]         i_que_iova_addr,
    input  wire [SYMBOL_NUM*64-1:0]         i_que_slot_num,
    input  wire [SYMBOL_NUM*64-1:0]         i_que_enable,
    input  wire [SYMBOL_NUM*64-1:0]         i_que_cons_ptr,
    input  wire                             i_reg_reset,
    output wire [SYMBOL_NUM*64-1:0]         o_que_prod_ptr,
    output wire [SYMBOL_NUM*64-1:0]         o_que_drop_count,
    output wire [SYMBOL_NUM*64-1:0]         o_que_status,
    input  wire                             i_rq_ready,
    output reg                              o_rq_valid,
    output reg  [3:0]                       o_rq_type,
    output reg                              o_rq_payload_last,
    output reg  [63:0]                      o_rq_addr,
    output reg  [10:0]                      o_rq_payload_dw_count,
    output reg  [7:0]                       o_rq_tag,
    output reg  [2:0]                       o_rq_tc,
    output reg  [255:0]                     o_rq_payload
);

localparam [3:0] TYPE_WRITE = 4'b0001;
localparam [10:0] RECORD_DW_COUNT = 11'd8;
localparam [1:0] ST_IDLE      = 2'd0;
localparam [1:0] ST_WAIT_FIFO = 2'd1;
localparam [1:0] ST_SEND      = 2'd2;
localparam integer RECORD_W   = 248;


function integer clog2 (input integer value);
    integer idx;
    begin
        clog2 = 0;
        for (idx = value - 1; idx > 0; idx = idx >> 1)
            clog2 = clog2 + 1;
    end
endfunction

function [255:0] pack_record (input [RECORD_W-1:0] payload);
    begin
        pack_record = 256'd0; // 256 bits is 32 bytes.
        pack_record[RECORD_W-1:0] = payload;
    end
endfunction

function [63:0] calc_write_addr (input [63:0] base_addr, input [63:0] slot_index);
    begin
        calc_write_addr = base_addr + (slot_index << 5); // per slot is 32 bytes, this converts slot index to bytes
    end
endfunction

function [63:0] calc_next_slot_index (input [63:0] slot_index, input [63:0] que_slot_num);
    begin
        if (que_slot_num <= 64'd1)
            calc_next_slot_index = 64'd0;
        else if (slot_index + 64'd1 >= que_slot_num)
            calc_next_slot_index = 64'd0;
        else
            calc_next_slot_index = slot_index + 64'd1;
    end
endfunction

localparam CL_SYMBOL_NUM = (SYMBOL_NUM > 1) ? clog2(SYMBOL_NUM) : 1;

reg [63:0] que_prod_ptr [0:SYMBOL_NUM-1];
reg [63:0] que_slot_index [0:SYMBOL_NUM-1];
reg [63:0] que_drop_count [0:SYMBOL_NUM-1];
reg [SYMBOL_NUM*48-1:0] prev_frame_ts;

reg [1:0]                state;
reg [CL_SYMBOL_NUM-1:0]  current_que_idx;
reg                      drop_current; // drop the queue entry or not.
reg [PAYLOAD_W+8-1:0]    record_que;
reg [63:0]               write_addr;

wire                     selected_valid;
wire [CL_SYMBOL_NUM-1:0] selected_que_idx;
wire                     selected_accept;
wire                     selected_drop;
wire [SYMBOL_NUM-1:0]    que_request_vec;

wire [SYMBOL_NUM-1:0]   que_full_vec;
wire [SYMBOL_NUM-1:0]   que_enabled_vec;
wire [SYMBOL_NUM-1:0]   que_busy_vec;

wire [31:0]   ask_best_price        = i_event_payload [PAYLOAD_W*current_que_idx+191 -: 32];
wire [31:0]   ask_best_shares       = i_event_payload [PAYLOAD_W*current_que_idx+159 -: 32];
wire [31:0]   bid_best_price        = i_event_payload [PAYLOAD_W*current_que_idx+127 -: 32];
wire [31:0]   bid_best_shares       = i_event_payload [PAYLOAD_W*current_que_idx+95 -: 32];
wire [47:0]      cur_frame_ts       = i_event_payload [PAYLOAD_W*current_que_idx+63 -: 48]; // the tick when frame arrives at MAC layer
wire [47:0]     event_tk            = i_dma_timestamp ;
wire [15:0]   stock_locate          = i_event_payload [PAYLOAD_W*current_que_idx+15 -: 16];   
// wire [47:0]   frame_latency         = latch_dma_ts > frame_start_tk ? latch_dma_ts - frame_start_tk : 48'd0; // the latency from the frame starts to DMA stage output.
integer que_idx;
integer prev_sel_idx;
reg [47:0] prev_frame_ts_cur;
wire [7:0] current_is_first_event = (cur_frame_ts != prev_frame_ts_cur) ? 8'h01 : 8'h00;

always @(*) begin
    prev_frame_ts_cur = 48'd0;
    for (prev_sel_idx = 0; prev_sel_idx < SYMBOL_NUM; prev_sel_idx = prev_sel_idx + 1) begin
        if (current_que_idx == prev_sel_idx[CL_SYMBOL_NUM-1:0])
            prev_frame_ts_cur = prev_frame_ts[prev_sel_idx*48 +: 48];
    end
end

generate
    genvar gi;
    for (gi = 0; gi < SYMBOL_NUM; gi = gi + 1) begin : g_que_status
        assign o_que_prod_ptr[gi*64 +: 64]      = que_prod_ptr[gi];
        assign o_que_drop_count[gi*64 +: 64]    = que_drop_count[gi];
        assign que_enabled_vec[gi]              = i_que_enable[gi*64];
        assign que_full_vec[gi]                 =
            (i_que_slot_num[gi*64 +: 64] == 64'd0) ? 1'b1 :
            ((que_prod_ptr[gi] - i_que_cons_ptr[gi*64 +: 64]) >= i_que_slot_num[gi*64 +: 64]);
        assign que_busy_vec[gi]              = (state != ST_IDLE) && (current_que_idx == gi[CL_SYMBOL_NUM-1:0]);
        assign o_que_status[gi*64 +: 64]   =
            {61'd0, que_busy_vec[gi], que_full_vec[gi], que_enabled_vec[gi]};
    end
endgenerate

assign que_request_vec = ~i_event_empty;
assign selected_accept = (state == ST_IDLE) && selected_valid;

arbiter #(
    .SYMBOL_NUM (SYMBOL_NUM)
) que_arbiter_inst (
    .i_clk     (i_clk),
    .i_rst     (i_rst),
    .i_req     (que_request_vec),
    .i_accept  (selected_accept),
    .o_valid   (selected_valid),
    .o_que_idx (selected_que_idx)
);


assign selected_drop            = selected_valid ? (!que_enabled || que_full) : 1'b0;
wire [63:0] que_slot_num_value  = i_que_slot_num[selected_que_idx*64 +: 64];
wire [63:0] que_cons_value      = i_que_cons_ptr[selected_que_idx*64 +: 64];
wire [63:0] que_outstanding     = que_prod_ptr[selected_que_idx] - que_cons_value;
wire        que_enabled         = i_que_enable[selected_que_idx*64];
wire        que_full            = (que_slot_num_value == 64'd0) || (que_outstanding >= que_slot_num_value);





always @(posedge i_clk or posedge i_rst) begin
    if (i_rst) begin
        state               <= ST_IDLE;
        current_que_idx     <= {CL_SYMBOL_NUM{1'b0}};
        drop_current        <= 1'b0;
        record_que          <= {(PAYLOAD_W+8){1'b0}};
        write_addr          <= 64'd0;
        o_event_pop         <= {SYMBOL_NUM{1'b0}};

        for (que_idx = 0; que_idx < SYMBOL_NUM; que_idx = que_idx + 1) begin
            que_prod_ptr[que_idx]   <= 64'd0;
            que_slot_index[que_idx] <= 64'd0;
            que_drop_count[que_idx] <= 64'd0;
            prev_frame_ts[que_idx*48 +: 48] <= 48'd0;
        end
    end else if (i_reg_reset) begin
        state                   <= ST_IDLE;
        current_que_idx         <= {CL_SYMBOL_NUM{1'b0}};
        record_que              <= {(PAYLOAD_W+8){1'b0}};
        write_addr              <= 64'd0;
        for (que_idx = 0; que_idx < SYMBOL_NUM; que_idx = que_idx + 1) begin
            que_prod_ptr[que_idx]   <= 64'd0;
            que_slot_index[que_idx] <= 64'd0;
            que_drop_count[que_idx] <= 64'd0;
            prev_frame_ts[que_idx*48 +: 48] <= 48'd0;
  
        end
    end
    else begin
        o_event_pop <= {SYMBOL_NUM{1'b0}};

        case (state)
            ST_IDLE: begin
                if (selected_valid) begin   
                    current_que_idx                 <= selected_que_idx;
                    drop_current                    <= selected_drop;
                    o_event_pop[selected_que_idx]   <= 1'b1;
                    state                           <= ST_WAIT_FIFO;
                end
            end
            ST_WAIT_FIFO: begin
                if (i_event_valid[current_que_idx]) begin
                    if (drop_current) begin
                        que_drop_count[current_que_idx] <= que_drop_count[current_que_idx] + 64'd1;
                        state <= ST_IDLE;
                    end else begin
                        record_que          <= {current_is_first_event, ask_best_price, ask_best_shares, bid_best_price, bid_best_shares, cur_frame_ts, stock_locate};
                        if (current_is_first_event != 8'd0) begin
                            prev_frame_ts[current_que_idx*48 +: 48] <= cur_frame_ts;
                        end
                        write_addr          <= calc_write_addr(
                            i_que_iova_addr[current_que_idx*64 +: 64],
                            que_slot_index[current_que_idx]
                        );
                        state <= ST_SEND;
                    end
                end
            end
            ST_SEND: begin
                if (i_rq_ready) begin
                    que_prod_ptr[current_que_idx] <= que_prod_ptr[current_que_idx] + 64'd1;
                    que_slot_index[current_que_idx] <= calc_next_slot_index(
                        que_slot_index[current_que_idx],
                        i_que_slot_num[current_que_idx*64 +: 64]
                    );
                    state <= ST_IDLE;
                end
            end
            default: begin
                state <= ST_IDLE;
            end
        endcase
    end
end

always @(*) begin
    o_rq_valid            = 1'b0;
    o_rq_type             = 4'b0;
    o_rq_payload_last     = 1'b0;
    o_rq_addr             = 64'd0;
    o_rq_payload_dw_count = 11'd0;
    o_rq_tag              = 8'd0;
    o_rq_tc               = 3'd0;
    o_rq_payload          = 256'd0;

    case (state)
        ST_SEND: begin
            o_rq_valid            = 1'b1;
            o_rq_type             = TYPE_WRITE;
            o_rq_payload_last     = 1'b1;
            o_rq_addr             = write_addr;
            o_rq_payload_dw_count = RECORD_DW_COUNT;
            o_rq_tag              = 8'h40 | current_que_idx;
            o_rq_payload          = pack_record({record_que[PAYLOAD_W+8-1:16], event_tk, record_que[15:0]});
        end
        default: begin
            o_rq_valid            = 1'b0;
        end
    endcase
end

endmodule
