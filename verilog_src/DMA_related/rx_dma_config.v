
module rx_dma_config #(
    parameter DATA_WIDTH = 256,
    parameter BAR0_SIZE = 16,
    parameter SYMBOL_NUM = 2,
    parameter [SYMBOL_NUM*16-1:0] SYMBOL_STOCK_LOCATE_INIT = {SYMBOL_NUM{16'd0}},
    parameter [SYMBOL_NUM*32-1:0] SYMBOL_PRICE_BASE_INIT   = {SYMBOL_NUM{32'd0}}
) (
    input  wire                         user_clk,
    input  wire                         user_reset_p,
    input  wire                         cq_valid,
    input  wire [3:0]                   cq_type,
    input  wire [BAR0_SIZE-1:0]         cq_reg_addr,
    input  wire [63:0]                  cq_payload,
    input  wire [15:0]                  cq_requester_id,
    input  wire [7:0]                   cq_tag,
    input  wire [2:0]                   cq_tc,
    input  wire [6:0]                   cq_lower_addr,
    input  wire [10:0]                  cq_payload_dw_count,
    input  wire                         cc_ready,
    output reg                          cc_valid,
    output reg [15:0]                   cc_requester_id,
    output reg [7:0]                    cc_tag,
    output reg [2:0]                    cc_tc,
    output reg [6:0]                    cc_lower_addr,
    output reg [10:0]                   cc_dword_count,
    output reg [2:0]                    cc_status,
    output reg [DATA_WIDTH/2-1:0]       cc_payload,
    output reg                          cc_last,
    output wire [SYMBOL_NUM*64-1:0]     o_rx_que_iova_addr,
    output wire [SYMBOL_NUM*64-1:0]     o_rx_que_slot_num,
    output wire [SYMBOL_NUM*64-1:0]     o_rx_que_enable,
    output wire [SYMBOL_NUM*64-1:0]     o_rx_que_cons_ptr,
    output wire [SYMBOL_NUM*16-1:0]     o_symbol_stock_locate,
    output wire [SYMBOL_NUM*32-1:0]     o_symbol_price_base,
    output reg                          o_reg_reset,
    input  wire [SYMBOL_NUM*64-1:0]     i_rx_que_prod_ptr,
    input  wire [SYMBOL_NUM*64-1:0]     i_rx_que_drop_count,
    input  wire [SYMBOL_NUM*64-1:0]     i_rx_que_status,
    input  wire [63:0]                  i_rx_dbg_pcs_frame_count,
    input  wire [63:0]                  i_rx_dbg_frame_count,
    input  wire [63:0]                  i_rx_dbg_msg_count,
    input  wire [SYMBOL_NUM*64-1:0]     i_rx_dbg_event_count,
    input  wire [47:0]                  i_dma_timestamp
);

localparam [3:0] TYPE_READ  = 4'b0000;
localparam [3:0] TYPE_WRITE = 4'b0001;



localparam [BAR0_SIZE-1:0] REG_RX_IOVA_OFFSET               = 16'h00;
localparam [BAR0_SIZE-1:0] REG_RESET                        = 16'h00;
localparam [BAR0_SIZE-1:0] REG_ID                           = 16'h04;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_SLOT_NUM_OFFSET       = 16'h08;
localparam [BAR0_SIZE-1:0] REG_SYNC_ENABLE                  = 16'h0C;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_ENABLE_OFFSET         = 16'h10;
localparam [BAR0_SIZE-1:0] REG_RX_SYMBOL_NUM                = 16'h14;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_CONS_PTR_OFFSET       = 16'h18;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_PROD_OFFSET           = 16'h20;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_DROP_OFFSET           = 16'h28;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_STAT_OFFSET           = 16'h30;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_SYMBOL_LOC_OFFSET     = 16'h34;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_PRICE_BASE_OFFSET     = 16'h38;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_IOVA                  = 16'h40;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_STRIDE                = 16'h40;
localparam [BAR0_SIZE-1:0] REG_RX_DBG_PCS_FRAME_COUNT       = 16'h0F8;
localparam [BAR0_SIZE-1:0] REG_RX_DBG_FRAME_COUNT           = 16'h100;
localparam [BAR0_SIZE-1:0] REG_RX_DBG_MSG_COUNT             = 16'h108;
localparam [BAR0_SIZE-1:0] REG_RX_DBG_EVENT_COUNT_BASE      = 16'h110;
localparam [BAR0_SIZE-1:0] REG_RX_DBG_EVENT_COUNT_STRIDE    = 16'h08;
localparam [BAR0_SIZE-1:0] REG_SYNC_TIMESTAMP               = 16'h114;

localparam [63:0] MODULE_ID = 64'h4d5f52585f434647;

reg [63:0] reg_que_iova     [0:SYMBOL_NUM-1];
reg [63:0] reg_que_slot_num [0:SYMBOL_NUM-1];
reg [63:0] reg_que_cons     [0:SYMBOL_NUM-1];
reg [15:0] reg_symbol_stock_locate [0:SYMBOL_NUM-1];
reg [31:0] reg_symbol_price_base   [0:SYMBOL_NUM-1];
reg        reg_sync_cali                     ;

reg                        rd_pending;
reg [BAR0_SIZE-1:0]        rd_reg_addr;
reg [15:0]                 rd_requester_id;
reg [7:0]                  rd_tag;
reg [2:0]                  rd_tc;
reg [6:0]                  rd_lower_addr;
reg [10:0]                 rd_dword_count;
reg [47:0]                 in_MMIO_timestamp;
wire [63:0]                ave_timestamp = (in_MMIO_timestamp+i_dma_timestamp)>>1;


integer que_idx;


function is_prod_ptr_addr (input [BAR0_SIZE-1:0] addr);
    integer idx;
    begin
        is_prod_ptr_addr = 1'b0;
        for (idx = 0; idx < SYMBOL_NUM; idx = idx + 1) begin
            if (addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_PROD_OFFSET))
                is_prod_ptr_addr = 1'b1;
        end
    end
endfunction

function [63:0] get_reg_value (input [BAR0_SIZE-1:0] reg_addr);
    integer idx;
    begin
        get_reg_value = 64'd0;
        case (reg_addr)
            REG_ID:                     get_reg_value = MODULE_ID;
            REG_SYNC_ENABLE:            get_reg_value = {63'd0, reg_sync_cali};
            REG_RX_SYMBOL_NUM:          get_reg_value = SYMBOL_NUM;
            REG_RX_DBG_PCS_FRAME_COUNT: get_reg_value = i_rx_dbg_pcs_frame_count;
            REG_RX_DBG_FRAME_COUNT:     get_reg_value = i_rx_dbg_frame_count;
            REG_RX_DBG_MSG_COUNT:       get_reg_value = i_rx_dbg_msg_count;
            REG_SYNC_TIMESTAMP:         get_reg_value = ave_timestamp;
            default: begin
                for (idx = 0; idx < SYMBOL_NUM; idx = idx + 1) begin
                    if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_IOVA_OFFSET))
                        get_reg_value = reg_que_iova[idx];
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_SLOT_NUM_OFFSET))
                        get_reg_value = reg_que_slot_num[idx];
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_ENABLE_OFFSET))
                        get_reg_value = {63'd0, (reg_que_iova[idx] != 64'd0) && (reg_que_slot_num[idx] != 64'd0)};
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_CONS_PTR_OFFSET))
                        get_reg_value = reg_que_cons[idx];
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_PROD_OFFSET))
                        get_reg_value = i_rx_que_prod_ptr[idx*64 +: 64];
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_DROP_OFFSET))
                        get_reg_value = i_rx_que_drop_count[idx*64 +: 64];
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_STAT_OFFSET))
                        get_reg_value = i_rx_que_status[idx*64 +: 64];
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_SYMBOL_LOC_OFFSET))
                        get_reg_value = {48'd0, reg_symbol_stock_locate[idx]};
                    else if (reg_addr == (REG_RX_QUE_IOVA + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_PRICE_BASE_OFFSET))
                        get_reg_value = {32'd0, reg_symbol_price_base[idx]};
                    else if (reg_addr == (REG_RX_DBG_EVENT_COUNT_BASE + idx*REG_RX_DBG_EVENT_COUNT_STRIDE))
                        get_reg_value = i_rx_dbg_event_count[idx*64 +: 64];
                end
            end
        endcase
    end
endfunction

generate
    genvar que_idx_gen;
    for (que_idx_gen = 0; que_idx_gen < SYMBOL_NUM; que_idx_gen = que_idx_gen + 1) begin : g_que_cfg
        assign o_rx_que_iova_addr[que_idx_gen*64 +: 64]  = reg_que_iova[que_idx_gen];
        assign o_rx_que_slot_num[que_idx_gen*64 +: 64] = reg_que_slot_num[que_idx_gen];
        assign o_rx_que_enable[que_idx_gen*64 +: 64]       =
            {63'd0, (reg_que_iova[que_idx_gen] != 64'd0) && (reg_que_slot_num[que_idx_gen] != 64'd0)};
        assign o_rx_que_cons_ptr[que_idx_gen*64 +: 64]   = reg_que_cons[que_idx_gen];
        assign o_symbol_stock_locate[que_idx_gen*16 +: 16] = reg_symbol_stock_locate[que_idx_gen];
        assign o_symbol_price_base[que_idx_gen*32 +: 32]   = reg_symbol_price_base[que_idx_gen];
    end
endgenerate

always @(posedge user_clk or posedge user_reset_p) begin
    if (user_reset_p) begin
        rd_pending      <= 1'b0;
        rd_reg_addr     <= {BAR0_SIZE{1'b0}};
        rd_requester_id <= 16'd0;
        rd_tag          <= 8'd0;
        rd_tc           <= 3'd0;
        rd_lower_addr   <= 7'd0;
        rd_dword_count  <= 11'd0;
        cc_valid        <= 1'b0;
        cc_requester_id <= 16'd0;
        cc_tag          <= 8'd0;
        cc_tc           <= 3'd0;
        cc_lower_addr   <= 7'd0;
        cc_dword_count  <= 11'd0;
        cc_status       <= 3'd0;
        cc_payload      <= {(DATA_WIDTH>>1){1'b0}};
        cc_last         <= 1'b0;
        o_reg_reset     <= 1'b0;
        reg_sync_cali   <= 1'b0;
        in_MMIO_timestamp <= 48'd0;
        for (que_idx = 0; que_idx < SYMBOL_NUM; que_idx = que_idx + 1) begin
            reg_que_iova[que_idx]       <= 64'd0;
            reg_que_slot_num[que_idx]   <= 64'd0;
            reg_que_cons[que_idx]       <= 64'd0;
            reg_symbol_stock_locate[que_idx] <= SYMBOL_STOCK_LOCATE_INIT[que_idx*16 +: 16];
            reg_symbol_price_base[que_idx]   <= SYMBOL_PRICE_BASE_INIT[que_idx*32 +: 32];
        end
    end else begin
        if (cc_valid && cc_ready) begin
            cc_valid <= 1'b0;
            cc_last  <= 1'b0;
        end
        if (o_reg_reset == 1'b1) begin
            o_reg_reset     <= 1'b0;
            reg_sync_cali   <= 1'b0;
            for (que_idx = 0; que_idx < SYMBOL_NUM; que_idx = que_idx + 1) begin
                reg_que_iova        [que_idx]       <= 64'd0;
                reg_que_slot_num    [que_idx]       <= 64'd0;
                reg_que_cons        [que_idx]       <= 64'd0;
                reg_symbol_stock_locate[que_idx]    <= SYMBOL_STOCK_LOCATE_INIT[que_idx*16 +: 16];
                reg_symbol_price_base[que_idx]      <= SYMBOL_PRICE_BASE_INIT[que_idx*32 +: 32];
            end
        end else if (cq_valid) begin
            if (cq_type == TYPE_WRITE) begin
                if  (cq_reg_addr == REG_RESET) begin
                    o_reg_reset <= 1'b1;
                end else if (cq_reg_addr == REG_SYNC_ENABLE) begin
                    reg_sync_cali <= cq_payload[0];
                end else begin
                    for (que_idx = 0; que_idx < SYMBOL_NUM; que_idx = que_idx + 1) begin
                        if (cq_reg_addr == (REG_RX_QUE_IOVA + que_idx*REG_RX_QUE_STRIDE + REG_RX_IOVA_OFFSET))
                            reg_que_iova[que_idx] <= cq_payload;
                        else if (cq_reg_addr == (REG_RX_QUE_IOVA + que_idx*REG_RX_QUE_STRIDE + REG_RX_QUE_SLOT_NUM_OFFSET))
                            reg_que_slot_num[que_idx] <= cq_payload;
                        else if (cq_reg_addr == (REG_RX_QUE_IOVA + que_idx*REG_RX_QUE_STRIDE + REG_RX_QUE_CONS_PTR_OFFSET))
                            reg_que_cons[que_idx] <= cq_payload;
                        else if (cq_reg_addr == (REG_RX_QUE_IOVA + que_idx*REG_RX_QUE_STRIDE + REG_RX_QUE_SYMBOL_LOC_OFFSET))
                            reg_symbol_stock_locate[que_idx] <= cq_payload[15:0];
                        else if (cq_reg_addr == (REG_RX_QUE_IOVA + que_idx*REG_RX_QUE_STRIDE + REG_RX_QUE_PRICE_BASE_OFFSET))
                            reg_symbol_price_base[que_idx] <= cq_payload[31:0];
                    end
                end
            end else if (cq_type == TYPE_READ && !rd_pending && !cc_valid) begin
                rd_pending      <= 1'b1;
                rd_reg_addr     <= cq_reg_addr;
                rd_requester_id <= cq_requester_id;
                rd_tag          <= cq_tag;
                rd_tc           <= cq_tc;
                rd_lower_addr   <= cq_lower_addr;
                rd_dword_count  <= cq_payload_dw_count;
                in_MMIO_timestamp <= i_dma_timestamp;
            end
        end
        if (rd_pending && !cc_valid) begin
            cc_valid        <= 1'b1;
            cc_requester_id <= rd_requester_id;
            cc_tag          <= rd_tag;
            cc_tc           <= rd_tc;
            cc_lower_addr   <= rd_lower_addr;
            cc_dword_count  <= rd_dword_count;
            cc_status       <= 3'b000;
            if (reg_sync_cali == 1'b1 && is_prod_ptr_addr(rd_reg_addr)) begin
                cc_payload      <= { ave_timestamp,get_reg_value(rd_reg_addr)};
            end else begin
                cc_payload      <= {{64{1'b0}}, get_reg_value(rd_reg_addr)};
            end
            cc_last         <= 1'b1;
            rd_pending      <= 1'b0;
        end
    end
end


endmodule
