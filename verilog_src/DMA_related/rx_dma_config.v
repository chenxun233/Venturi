
module rx_dma_config #(
    parameter DATA_WIDTH = 256,
    parameter BAR0_SIZE = 16,
    parameter RX_QUE_COUNT = 2
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
    output wire [RX_QUE_COUNT*64-1:0]   o_rx_que_base_addr,
    output wire [RX_QUE_COUNT*64-1:0]   o_rx_que_slot_num,
    output wire [RX_QUE_COUNT*64-1:0]   o_rx_que_enable,
    output wire [RX_QUE_COUNT*64-1:0]   o_rx_que_cons_ptr,
    input  wire [RX_QUE_COUNT*64-1:0]   i_rx_que_prod_ptr,
    input  wire [RX_QUE_COUNT*64-1:0]   i_rx_que_drop_count,
    input  wire [RX_QUE_COUNT*64-1:0]   i_rx_que_status
);

localparam [3:0] TYPE_READ  = 4'b0000;
localparam [3:0] TYPE_WRITE = 4'b0001;

localparam [BAR0_SIZE-1:0] REG_ID                    = 16'h04;
localparam [BAR0_SIZE-1:0] REG_STATUS                = 16'h0C;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_BASE0          = 16'h40;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_STRIDE         = 16'h40;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_SLOT_NUM_OFFSET    = 16'h08;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_ENABLE_OFFSET    = 16'h10;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_CONS_PTR_OFFSET    = 16'h18;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_PROD_OFFSET    = 16'h20;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_DROP_OFFSET    = 16'h28;
localparam [BAR0_SIZE-1:0] REG_RX_QUE_STAT_OFFSET    = 16'h30;

localparam [63:0] MODULE_ID = 64'h4d5f52585f434647;

reg [63:0] reg_que_base [0:RX_QUE_COUNT-1];
reg [63:0] reg_que_slot_num [0:RX_QUE_COUNT-1];
reg [63:0] reg_que_cons [0:RX_QUE_COUNT-1];

reg                        rd_pending;
reg [BAR0_SIZE-1:0]        rd_reg_addr;
reg [15:0]                 rd_requester_id;
reg [7:0]                  rd_tag;
reg [2:0]                  rd_tc;
reg [6:0]                  rd_lower_addr;
reg [10:0]                 rd_dword_count;

integer que_idx;

function [63:0] get_reg_value (input [BAR0_SIZE-1:0] reg_addr);
    integer idx;
    begin
        get_reg_value = 64'd0;
        case (reg_addr)
            REG_ID:     get_reg_value = MODULE_ID;
            REG_STATUS: get_reg_value = 64'd0;
            default: begin
                for (idx = 0; idx < RX_QUE_COUNT; idx = idx + 1) begin
                    if (reg_addr == (REG_RX_QUE_BASE0 + idx*REG_RX_QUE_STRIDE))
                        get_reg_value = reg_que_base[idx];
                    else if (reg_addr == (REG_RX_QUE_BASE0 + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_SLOT_NUM_OFFSET))
                        get_reg_value = reg_que_slot_num[idx];
                    else if (reg_addr == (REG_RX_QUE_BASE0 + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_ENABLE_OFFSET))
                        get_reg_value = {63'd0, (reg_que_base[idx] != 64'd0) && (reg_que_slot_num[idx] != 64'd0)};
                    else if (reg_addr == (REG_RX_QUE_BASE0 + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_CONS_PTR_OFFSET))
                        get_reg_value = reg_que_cons[idx];
                    else if (reg_addr == (REG_RX_QUE_BASE0 + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_PROD_OFFSET))
                        get_reg_value = i_rx_que_prod_ptr[idx*64 +: 64];
                    else if (reg_addr == (REG_RX_QUE_BASE0 + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_DROP_OFFSET))
                        get_reg_value = i_rx_que_drop_count[idx*64 +: 64];
                    else if (reg_addr == (REG_RX_QUE_BASE0 + idx*REG_RX_QUE_STRIDE + REG_RX_QUE_STAT_OFFSET))
                        get_reg_value = i_rx_que_status[idx*64 +: 64];
                end
            end
        endcase
    end
endfunction

generate
    genvar que_idx_gen;
    for (que_idx_gen = 0; que_idx_gen < RX_QUE_COUNT; que_idx_gen = que_idx_gen + 1) begin : g_que_cfg
        assign o_rx_que_base_addr[que_idx_gen*64 +: 64]  = reg_que_base[que_idx_gen];
        assign o_rx_que_slot_num[que_idx_gen*64 +: 64] = reg_que_slot_num[que_idx_gen];
        assign o_rx_que_enable[que_idx_gen*64 +: 64]       =
            {63'd0, (reg_que_base[que_idx_gen] != 64'd0) && (reg_que_slot_num[que_idx_gen] != 64'd0)};
        assign o_rx_que_cons_ptr[que_idx_gen*64 +: 64]   = reg_que_cons[que_idx_gen];
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

        for (que_idx = 0; que_idx < RX_QUE_COUNT; que_idx = que_idx + 1) begin
            reg_que_base[que_idx]       <= 64'd0;
            reg_que_slot_num[que_idx] <= 64'd0;
            reg_que_cons[que_idx]       <= 64'd0;
        end
    end else begin
        if (cc_valid && cc_ready) begin
            cc_valid <= 1'b0;
            cc_last  <= 1'b0;
        end

        if (cq_valid) begin
            if (cq_type == TYPE_WRITE) begin
                for (que_idx = 0; que_idx < RX_QUE_COUNT; que_idx = que_idx + 1) begin
                    if (cq_reg_addr == (REG_RX_QUE_BASE0 + que_idx*REG_RX_QUE_STRIDE))
                        reg_que_base[que_idx] <= cq_payload;
                    else if (cq_reg_addr == (REG_RX_QUE_BASE0 + que_idx*REG_RX_QUE_STRIDE + REG_RX_QUE_SLOT_NUM_OFFSET))
                        reg_que_slot_num[que_idx] <= cq_payload;
                    else if (cq_reg_addr == (REG_RX_QUE_BASE0 + que_idx*REG_RX_QUE_STRIDE + REG_RX_QUE_CONS_PTR_OFFSET))
                        reg_que_cons[que_idx] <= cq_payload;
                end
            end else if (cq_type == TYPE_READ && !rd_pending && !cc_valid) begin
                rd_pending      <= 1'b1;
                rd_reg_addr     <= cq_reg_addr;
                rd_requester_id <= cq_requester_id;
                rd_tag          <= cq_tag;
                rd_tc           <= cq_tc;
                rd_lower_addr   <= cq_lower_addr;
                rd_dword_count  <= cq_payload_dw_count;
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
            cc_payload      <= {{64{1'b0}}, get_reg_value(rd_reg_addr)};
            cc_last         <= 1'b1;
            rd_pending      <= 1'b0;
        end
    end
end

endmodule
