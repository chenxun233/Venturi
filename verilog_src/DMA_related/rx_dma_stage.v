module rx_dma_stage #(
    parameter SYMBOL_NUM = 2,
    parameter PAYLOAD_W  = 274
) (
    input  wire                             i_clk,
    input  wire                             i_rst,
    input  wire [SYMBOL_NUM-1:0]            i_event_empty,
    input  wire [SYMBOL_NUM-1:0]            i_event_valid,
    input  wire [SYMBOL_NUM*PAYLOAD_W-1:0]  i_event_payload,
    output reg  [SYMBOL_NUM-1:0]            o_event_pop,
    input  wire [SYMBOL_NUM*64-1:0]         i_ring_base_addr,
    input  wire [SYMBOL_NUM*64-1:0]         i_ring_size,
    input  wire [SYMBOL_NUM*64-1:0]         i_ring_ctrl,
    input  wire [SYMBOL_NUM*64-1:0]         i_ring_cons_ptr,
    output wire [SYMBOL_NUM*64-1:0]         o_ring_prod_ptr,
    output wire [SYMBOL_NUM*64-1:0]         o_ring_drop_count,
    output wire [SYMBOL_NUM*64-1:0]         o_ring_status,
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
localparam [10:0] RECORD_DW_COUNT = 11'd16;
localparam [1:0] ST_IDLE      = 2'd0;
localparam [1:0] ST_WAIT_FIFO = 2'd1;
localparam [1:0] ST_SEND_0    = 2'd2;
localparam [1:0] ST_SEND_1    = 2'd3;

function integer clog2;
    input integer value;
    integer idx;
    begin
        clog2 = 0;
        for (idx = value - 1; idx > 0; idx = idx >> 1)
            clog2 = clog2 + 1;
    end
endfunction

function [511:0] pack_record;
    input [PAYLOAD_W-1:0] payload;
    begin
        pack_record = 512'd0;
        pack_record[PAYLOAD_W-1:0] = payload;
    end
endfunction

function [63:0] calc_write_addr;
    input [63:0] base_addr;
    input [63:0] ring_size_slots;
    input [63:0] prod_ptr;
    reg   [63:0] slot_index;
    begin
        if (ring_size_slots == 64'd0)
            slot_index = 64'd0;
        else
            slot_index = prod_ptr % ring_size_slots;
        calc_write_addr = base_addr + (slot_index << 6);
    end
endfunction

localparam CL_SYMBOL_NUM = (SYMBOL_NUM > 1) ? clog2(SYMBOL_NUM) : 1;

reg [63:0] ring_prod_ptr [0:SYMBOL_NUM-1];
reg [63:0] ring_drop_count [0:SYMBOL_NUM-1];

reg [1:0]                state_reg;
reg [CL_SYMBOL_NUM-1:0]  current_ring_reg;
reg [CL_SYMBOL_NUM-1:0]  rr_pointer_reg;
reg                      drop_current_reg;
reg [511:0]              record_buffer_reg;
reg [63:0]               write_addr_reg;

reg                      selected_valid_reg;
reg [CL_SYMBOL_NUM-1:0]  selected_ring_reg;
reg                      selected_drop_reg;

wire [SYMBOL_NUM-1:0] ring_full_vec;
wire [SYMBOL_NUM-1:0] ring_enabled_vec;
wire [SYMBOL_NUM-1:0] ring_clear_vec;
wire [SYMBOL_NUM-1:0] ring_busy_vec;

integer ring_idx;
integer rr_offset;
integer candidate_ring;
reg [63:0] ring_size_value;
reg [63:0] ring_cons_value;
reg [63:0] ring_outstanding;
reg        ring_enabled;
reg        ring_full;

generate
    genvar gi;
    for (gi = 0; gi < SYMBOL_NUM; gi = gi + 1) begin : g_ring_status
        assign o_ring_prod_ptr[gi*64 +: 64]   = ring_prod_ptr[gi];
        assign o_ring_drop_count[gi*64 +: 64] = ring_drop_count[gi];
        assign ring_enabled_vec[gi]           = i_ring_ctrl[gi*64];
        assign ring_clear_vec[gi]             = i_ring_ctrl[gi*64 + 1];
        assign ring_full_vec[gi]              =
            (i_ring_size[gi*64 +: 64] == 64'd0) ? 1'b1 :
            ((ring_prod_ptr[gi] - i_ring_cons_ptr[gi*64 +: 64]) >= i_ring_size[gi*64 +: 64]);
        assign ring_busy_vec[gi]              = (state_reg != ST_IDLE) && (current_ring_reg == gi[CL_SYMBOL_NUM-1:0]);
        assign o_ring_status[gi*64 +: 64]     =
            {60'd0, ring_clear_vec[gi], ring_busy_vec[gi], ring_full_vec[gi], ring_enabled_vec[gi]};
    end
endgenerate

always @(*) begin
    selected_valid_reg = 1'b0;
    selected_ring_reg  = rr_pointer_reg;
    selected_drop_reg  = 1'b0;

    for (rr_offset = 0; rr_offset < SYMBOL_NUM; rr_offset = rr_offset + 1) begin
        candidate_ring = rr_pointer_reg + rr_offset;
        if (candidate_ring >= SYMBOL_NUM)
            candidate_ring = candidate_ring - SYMBOL_NUM;

        ring_size_value = i_ring_size[candidate_ring*64 +: 64];
        ring_cons_value = i_ring_cons_ptr[candidate_ring*64 +: 64];
        ring_outstanding = ring_prod_ptr[candidate_ring] - ring_cons_value;
        ring_enabled = i_ring_ctrl[candidate_ring*64] && !i_ring_ctrl[candidate_ring*64 + 1];
        ring_full = (ring_size_value == 64'd0) || (ring_outstanding >= ring_size_value);

        if (!selected_valid_reg && !i_event_empty[candidate_ring]) begin
            selected_valid_reg = 1'b1;
            selected_ring_reg  = candidate_ring[CL_SYMBOL_NUM-1:0];
            selected_drop_reg  = !ring_enabled || ring_full;
        end
    end
end

always @(posedge i_clk or posedge i_rst) begin
    if (i_rst) begin
        state_reg        <= ST_IDLE;
        current_ring_reg <= {CL_SYMBOL_NUM{1'b0}};
        rr_pointer_reg   <= {CL_SYMBOL_NUM{1'b0}};
        drop_current_reg <= 1'b0;
        record_buffer_reg <= 512'd0;
        write_addr_reg   <= 64'd0;
        o_event_pop       <= {SYMBOL_NUM{1'b0}};

        for (ring_idx = 0; ring_idx < SYMBOL_NUM; ring_idx = ring_idx + 1) begin
            ring_prod_ptr[ring_idx]   <= 64'd0;
            ring_drop_count[ring_idx] <= 64'd0;
        end
    end else begin
        o_event_pop <= {SYMBOL_NUM{1'b0}};

        for (ring_idx = 0; ring_idx < SYMBOL_NUM; ring_idx = ring_idx + 1) begin
            if (ring_clear_vec[ring_idx]) begin
                ring_prod_ptr[ring_idx]   <= 64'd0;
                ring_drop_count[ring_idx] <= 64'd0;
            end
        end

        case (state_reg)
            ST_IDLE: begin
                if (selected_valid_reg) begin
                    current_ring_reg <= selected_ring_reg;
                    drop_current_reg <= selected_drop_reg;
                    o_event_pop[selected_ring_reg] <= 1'b1;
                    state_reg <= ST_WAIT_FIFO;
                end
            end
            ST_WAIT_FIFO: begin
                if (i_event_valid[current_ring_reg]) begin
                    if (drop_current_reg) begin
                        ring_drop_count[current_ring_reg] <= ring_drop_count[current_ring_reg] + 64'd1;
                        rr_pointer_reg <= current_ring_reg + {{(CL_SYMBOL_NUM-1){1'b0}}, 1'b1};
                        state_reg <= ST_IDLE;
                    end else begin
                        record_buffer_reg <= pack_record(i_event_payload[current_ring_reg*PAYLOAD_W +: PAYLOAD_W]);
                        write_addr_reg <= calc_write_addr(
                            i_ring_base_addr[current_ring_reg*64 +: 64],
                            i_ring_size[current_ring_reg*64 +: 64],
                            ring_prod_ptr[current_ring_reg]
                        );
                        state_reg <= ST_SEND_0;
                    end
                end
            end
            ST_SEND_0: begin
                if (i_rq_ready)
                    state_reg <= ST_SEND_1;
            end
            ST_SEND_1: begin
                if (i_rq_ready) begin
                    ring_prod_ptr[current_ring_reg] <= ring_prod_ptr[current_ring_reg] + 64'd1;
                    rr_pointer_reg <= current_ring_reg + {{(CL_SYMBOL_NUM-1){1'b0}}, 1'b1};
                    state_reg <= ST_IDLE;
                end
            end
            default: begin
                state_reg <= ST_IDLE;
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

    case (state_reg)
        ST_SEND_0: begin
            o_rq_valid            = 1'b1;
            o_rq_type             = TYPE_WRITE;
            o_rq_payload_last     = 1'b0;
            o_rq_addr             = write_addr_reg;
            o_rq_payload_dw_count = RECORD_DW_COUNT;
            o_rq_tag              = 8'h40 | current_ring_reg;
            o_rq_payload          = record_buffer_reg[255:0];
        end
        ST_SEND_1: begin
            o_rq_valid            = 1'b1;
            o_rq_type             = TYPE_WRITE;
            o_rq_payload_last     = 1'b1;
            o_rq_addr             = write_addr_reg;
            o_rq_payload_dw_count = RECORD_DW_COUNT;
            o_rq_tag              = 8'h40 | current_ring_reg;
            o_rq_payload          = record_buffer_reg[511:256];
        end
        default: begin
            o_rq_valid            = 1'b0;
        end
    endcase
end

endmodule
