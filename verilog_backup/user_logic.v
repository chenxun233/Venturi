// =============================================================================
// user_logic.v - User Logic for FPGA PCIe Hello World Example
// =============================================================================

// =============================================================================

module user_logic #(
    parameter DATA_WIDTH = 256,
    parameter BAR0_SIZE  = 16,
    parameter RX_RING_COUNT = 2
)(
    input wire                          user_clk,
    input wire                          user_reset_p, //active high
    // CQ Parser Outputs (Host → FPGA MMIO Requests)
    input wire                          cq_valid,
    input wire [3:0]                    cq_type,
    input wire [BAR0_SIZE-1:0]          cq_reg_addr,
    input wire [63:0]                   cq_payload,
    input wire [2:0]                    cq_bar_id,
    input wire [15:0]                   cq_requester_id,
    input wire [7:0]                    cq_tag,
    input wire [2:0]                    cq_tc,
    input wire [6:0]                    cq_lower_addr,
    input wire [10:0]                   cq_payload_dw_count,
    input wire                          cq_last,
    // CC Formatter Inputs (FPGA → Host Read Responses)
    input wire                          cc_ready,
    output reg                          cc_valid,
    output reg[15:0]                    cc_requester_id,
    output reg[7:0]                     cc_tag,
    output reg[2:0]                     cc_tc,
    output reg[6:0]                     cc_lower_addr,
    output reg[10:0]                    cc_dword_count,
    output reg[2:0]                     cc_status,
    output reg[DATA_WIDTH/2-1:0]        cc_payload,
    output reg                          cc_last,
    // RQ Formatter Inputs (FPGA → Host DMA Requests)
    input wire                          rq_ready,
    output reg                          rq_valid,
    output reg[3:0]                     rq_type,
    output reg                          rq_payload_sop,
    output reg                          rq_payload_last,        // the last cycle of data sending
    output reg[63:0]                    rq_addr,
    output reg[10:0]                    rq_payload_dw_count, // Total Data DWords (1-1024) in this burst. Does not include header.
    output reg[7:0]                     rq_tag,
    output reg[2:0]                     rq_tc,
    output reg[255:0]                   rq_payload,
    // RC Parser Outputs (Host → FPGA DMA Read Completions) - Realigned by gearbox
    input  wire  [11:0]                 rc_lower_addr,
    input  wire  [3:0]                  rc_err_code,
    input  wire  [12:0]                 rc_payload_byte_count,
    input  wire                         rc_request_completed,
    input  wire  [15:0]                 rc_requester_id,
    input  wire  [7:0]                  rc_tag,
    input  wire                         rc_valid,               // valid for all
    input  wire                         rc_payload_last,         // End of packet
    input  wire [255:0]                 rc_payload,             // Realigned 256-bit payload
    input  wire [DATA_WIDTH / 32-1:0]   rc_payload_dw_keep,         // DW enables
    input  wire                         rc_posioned,
    input  wire [12:0]                  rc_payload_dw_count,
    // Configuration Outputs
    input wire [2:0]                    cfg_max_payload,
    input wire [2:0]                    cfg_max_read_req,
    input wire [15:0]                   pcie_requester_id,
    output wire [RX_RING_COUNT*64-1:0]  o_rx_ring_base_addr,
    output wire [RX_RING_COUNT*64-1:0]  o_rx_ring_size,
    output wire [RX_RING_COUNT*64-1:0]  o_rx_ring_ctrl,
    output wire [RX_RING_COUNT*64-1:0]  o_rx_ring_cons_ptr,
    input  wire [RX_RING_COUNT*64-1:0]  i_rx_ring_prod_ptr,
    input  wire [RX_RING_COUNT*64-1:0]  i_rx_ring_drop_count,
    input  wire [RX_RING_COUNT*64-1:0]  i_rx_ring_status
);
    // =========================================================================
    // Register Address Map
    localparam [BAR0_SIZE-1:0] REG_SCRATCH      = 16'h00;
    localparam [BAR0_SIZE-1:0] REG_ID           = 16'h04;
    localparam [BAR0_SIZE-1:0] REG_INT_CTRL     = 16'h08;
    localparam [BAR0_SIZE-1:0] REG_STATUS       = 16'h0C;
    localparam [BAR0_SIZE-1:0] REG_DMA_ADDR     = 16'h10;  // 64-bit DMA target address
    localparam [BAR0_SIZE-1:0] REG_DMA_CTRL     = 16'h18;
    localparam [BAR0_SIZE-1:0] REG_DMA_STATUS   = 16'h1C;
    localparam [BAR0_SIZE-1:0] REG_RT_SRC_ADDR  = 16'h20;
    localparam [BAR0_SIZE-1:0] REG_RT_DST_ADDR  = 16'h28;
    localparam [BAR0_SIZE-1:0] REG_RT_CTRL      = 16'h30;
    localparam [BAR0_SIZE-1:0] REG_RT_STATUS    = 16'h34;
    localparam [BAR0_SIZE-1:0] REG_RX_RING0_BASE = 16'h40;
    localparam [BAR0_SIZE-1:0] REG_RX_RING0_SIZE = 16'h48;
    localparam [BAR0_SIZE-1:0] REG_RX_RING0_CTRL = 16'h50;
    localparam [BAR0_SIZE-1:0] REG_RX_RING0_CONS = 16'h58;
    localparam [BAR0_SIZE-1:0] REG_RX_RING0_PROD = 16'h60;
    localparam [BAR0_SIZE-1:0] REG_RX_RING0_DROP = 16'h68;
    localparam [BAR0_SIZE-1:0] REG_RX_RING0_STAT = 16'h70;
    localparam [BAR0_SIZE-1:0] REG_RX_RING1_BASE = 16'h80;
    localparam [BAR0_SIZE-1:0] REG_RX_RING1_SIZE = 16'h88;
    localparam [BAR0_SIZE-1:0] REG_RX_RING1_CTRL = 16'h90;
    localparam [BAR0_SIZE-1:0] REG_RX_RING1_CONS = 16'h98;
    localparam [BAR0_SIZE-1:0] REG_RX_RING1_PROD = 16'hA0;
    localparam [BAR0_SIZE-1:0] REG_RX_RING1_DROP = 16'hA8;
    localparam [BAR0_SIZE-1:0] REG_RX_RING1_STAT = 16'hB0;

// =========================================================================
    localparam [3:0] TYPE_READ = 4'b0000;
    localparam [3:0] TYPE_WRITE= 4'b0001;

    // =========================================================================
    // Register Storage, corresponding to BAR0 address map
    // =========================================================================
    reg [63:0] cq_val_REG_SCRATCH      ;
    reg [63:0] cq_val_REG_ID           ;
    reg [63:0] cq_val_REG_INT_CTRL     ;
    reg [63:0] cq_val_REG_STATUS       ;
    reg [63:0] cq_val_REG_DMA_ADDR     ;
    reg [63:0] cq_val_REG_DMA_CTRL     ;
    reg [63:0] cq_val_REG_DMA_STATUS   ;
    reg [63:0] cq_val_REG_RT_SRC_ADDR  ;
    reg [63:0] cq_val_REG_RT_DST_ADDR  ;
    reg [63:0] cq_val_REG_RT_CTRL      ;
    reg [63:0] in_val_REG_RT_STATUS    ;
    reg [63:0] cq_val_REG_RX_RING_BASE [0:RX_RING_COUNT-1];
    reg [63:0] cq_val_REG_RX_RING_SIZE [0:RX_RING_COUNT-1];
    reg [63:0] cq_val_REG_RX_RING_CTRL [0:RX_RING_COUNT-1];
    reg [63:0] cq_val_REG_RX_RING_CONS [0:RX_RING_COUNT-1];

    localparam [63:0] RT_CTRL_SMALL    = 64'd1;
    localparam [63:0] RT_CTRL_LARGE    = 64'd2;
    localparam [63:0] RT_CTRL_LONG     = 64'd3;

    localparam [63:0] RT_STATUS_BUSY   = 64'h1;
    localparam [63:0] RT_STATUS_DONE   = 64'h2;
    localparam [63:0] RT_STATUS_ERROR  = 64'h4;

    localparam [10:0] RT_SMALL_DW      = 11'd4;
    localparam [10:0] RT_LARGE_DW      = 11'd12;
    localparam [10:0] RT_LONG_DW       = 11'd20;

    localparam [1:0]  RT_SMALL_BEATS   = 2'd1;
    localparam [1:0]  RT_LARGE_BEATS   = 2'd2;
    localparam [1:0]  RT_LONG_BEATS    = 2'd3;

    reg [255:0] rt_buffer [0:2];
    reg [7:0]   rt_buffer_dw_keep [0:2];
    reg [1:0]   rc_beat_count;
    reg [1:0]   write_beat_index;
    reg [10:0]  rt_expected_dw_count;
    reg [1:0]   rt_expected_beats;
    reg [12:0]  rt_expected_byte_count;
    integer     i;

generate
    genvar ring_cfg_idx;
    for (ring_cfg_idx = 0; ring_cfg_idx < RX_RING_COUNT; ring_cfg_idx = ring_cfg_idx + 1) begin : g_rx_ring_cfg
        assign o_rx_ring_base_addr[ring_cfg_idx*64 +: 64] = cq_val_REG_RX_RING_BASE[ring_cfg_idx];
        assign o_rx_ring_size[ring_cfg_idx*64 +: 64]      = cq_val_REG_RX_RING_SIZE[ring_cfg_idx];
        assign o_rx_ring_ctrl[ring_cfg_idx*64 +: 64]      = cq_val_REG_RX_RING_CTRL[ring_cfg_idx];
        assign o_rx_ring_cons_ptr[ring_cfg_idx*64 +: 64]  = cq_val_REG_RX_RING_CONS[ring_cfg_idx];
    end
endgenerate


// cq channel ======================================================
    reg [BAR0_SIZE-1:0] temp_cq_reg_addr          ;
    reg [15:0]          temp_cq_requester_id      ;   
    reg [7:0]           temp_cq_tag               ;
    reg [2:0]           temp_cq_tc                ;
    reg [6:0]           temp_cq_lower_addr        ;
    reg [10:0]          temp_cq_payload_dw_count  ;  
    


always @(posedge user_clk or posedge user_reset_p) begin
    if (user_reset_p) begin
        // Initialize registers
        cq_val_REG_SCRATCH         <= {64{1'b0}};
        cq_val_REG_ID              <= {64{1'b0}};
        cq_val_REG_INT_CTRL        <= {64{1'b0}};
        cq_val_REG_STATUS          <= {64{1'b0}};
        cq_val_REG_DMA_ADDR        <= {64{1'b0}};
        cq_val_REG_DMA_CTRL        <= {64{1'b0}};
        cq_val_REG_DMA_STATUS      <= {64{1'b0}};
        cq_val_REG_RT_SRC_ADDR     <= {64{1'b0}};
        cq_val_REG_RT_DST_ADDR     <= {64{1'b0}};
        cq_val_REG_RT_CTRL         <= {64{1'b0}};
        for (i = 0; i < RX_RING_COUNT; i = i + 1) begin
            cq_val_REG_RX_RING_BASE[i] <= 64'd0;
            cq_val_REG_RX_RING_SIZE[i] <= 64'd0;
            cq_val_REG_RX_RING_CTRL[i] <= 64'd0;
            cq_val_REG_RX_RING_CONS[i] <= 64'd0;
        end
        temp_cq_reg_addr        <= {BAR0_SIZE{1'b0}};
        temp_cq_requester_id    <= 16'b0;
        temp_cq_tag             <= 8'b0;
        temp_cq_tc              <= 3'b0;
        temp_cq_lower_addr      <= 7'b0;
        temp_cq_payload_dw_count<= 11'b0;
    end else begin
        // Monitor for CQ Requests
        if (cq_valid == 1) begin
            if (cq_type == TYPE_WRITE) begin
                case (cq_reg_addr)
                    REG_SCRATCH     :   cq_val_REG_SCRATCH    <= cq_payload;
                    REG_ID          :   cq_val_REG_ID         <= cq_payload;
                    REG_INT_CTRL    :   cq_val_REG_INT_CTRL   <= cq_payload;
                    REG_STATUS      :   cq_val_REG_STATUS     <= cq_payload;
                    REG_DMA_ADDR    :   cq_val_REG_DMA_ADDR   <= cq_payload;
                    REG_DMA_CTRL    :   cq_val_REG_DMA_CTRL   <= cq_payload;
                    REG_DMA_STATUS  :   cq_val_REG_DMA_STATUS <= cq_payload;
                    REG_RT_SRC_ADDR :   cq_val_REG_RT_SRC_ADDR<= cq_payload;
                    REG_RT_DST_ADDR :   cq_val_REG_RT_DST_ADDR<= cq_payload;
                    REG_RT_CTRL     :   cq_val_REG_RT_CTRL    <= cq_payload;
                    REG_RX_RING0_BASE:  cq_val_REG_RX_RING_BASE[0] <= cq_payload;
                    REG_RX_RING0_SIZE:  cq_val_REG_RX_RING_SIZE[0] <= cq_payload;
                    REG_RX_RING0_CTRL:  cq_val_REG_RX_RING_CTRL[0] <= cq_payload;
                    REG_RX_RING0_CONS:  cq_val_REG_RX_RING_CONS[0] <= cq_payload;
                    REG_RX_RING1_BASE:  cq_val_REG_RX_RING_BASE[1] <= cq_payload;
                    REG_RX_RING1_SIZE:  cq_val_REG_RX_RING_SIZE[1] <= cq_payload;
                    REG_RX_RING1_CTRL:  cq_val_REG_RX_RING_CTRL[1] <= cq_payload;
                    REG_RX_RING1_CONS:  cq_val_REG_RX_RING_CONS[1] <= cq_payload;
                default: ; // Do nothing for RO or undefined registers
                endcase
            end else if (cq_type == TYPE_READ) begin
                temp_cq_reg_addr        <= cq_reg_addr          ; 
                temp_cq_requester_id    <= cq_requester_id      ;
                temp_cq_tag             <= cq_tag               ;
                temp_cq_tc              <= cq_tc                ;
                temp_cq_lower_addr      <= cq_lower_addr        ;
                temp_cq_payload_dw_count<= cq_payload_dw_count  ;
            end
        end
    end
end

// cc channel ======================================================
    // State Machine States
    reg [1:0]  cc_state;
    localparam IDLE          = 2'd0;
    localparam CC_RESP       = 2'd1;
always @(posedge user_clk or posedge user_reset_p) begin
    if (user_reset_p) begin
        cc_state           <= IDLE;
    end else if (cq_valid &&  cq_type == TYPE_READ) begin
        cc_state          <= CC_RESP;
    end else if (cc_state == CC_RESP) begin
        cc_state          <= IDLE;
    end
end

function [63:0] get_cc_payload (input [BAR0_SIZE-1:0] reg_addr);
    begin
        case (reg_addr)
            REG_SCRATCH     :   get_cc_payload = cq_val_REG_SCRATCH;
            REG_ID          :   get_cc_payload = cq_val_REG_ID;
            REG_INT_CTRL    :   get_cc_payload = cq_val_REG_INT_CTRL;
            REG_STATUS      :   get_cc_payload = cq_val_REG_STATUS;
            REG_DMA_ADDR    :   get_cc_payload = cq_val_REG_DMA_ADDR;
            REG_DMA_CTRL    :   get_cc_payload = cq_val_REG_DMA_CTRL;
            REG_DMA_STATUS  :   get_cc_payload = cq_val_REG_DMA_STATUS;
            REG_RT_SRC_ADDR :   get_cc_payload = cq_val_REG_RT_SRC_ADDR;
            REG_RT_DST_ADDR :   get_cc_payload = cq_val_REG_RT_DST_ADDR;
            REG_RT_CTRL     :   get_cc_payload = cq_val_REG_RT_CTRL;
            REG_RT_STATUS   :   get_cc_payload = in_val_REG_RT_STATUS;
            REG_RX_RING0_BASE:  get_cc_payload = cq_val_REG_RX_RING_BASE[0];
            REG_RX_RING0_SIZE:  get_cc_payload = cq_val_REG_RX_RING_SIZE[0];
            REG_RX_RING0_CTRL:  get_cc_payload = cq_val_REG_RX_RING_CTRL[0];
            REG_RX_RING0_CONS:  get_cc_payload = cq_val_REG_RX_RING_CONS[0];
            REG_RX_RING0_PROD:  get_cc_payload = i_rx_ring_prod_ptr[0*64 +: 64];
            REG_RX_RING0_DROP:  get_cc_payload = i_rx_ring_drop_count[0*64 +: 64];
            REG_RX_RING0_STAT:  get_cc_payload = i_rx_ring_status[0*64 +: 64];
            REG_RX_RING1_BASE:  get_cc_payload = cq_val_REG_RX_RING_BASE[1];
            REG_RX_RING1_SIZE:  get_cc_payload = cq_val_REG_RX_RING_SIZE[1];
            REG_RX_RING1_CTRL:  get_cc_payload = cq_val_REG_RX_RING_CTRL[1];
            REG_RX_RING1_CONS:  get_cc_payload = cq_val_REG_RX_RING_CONS[1];
            REG_RX_RING1_PROD:  get_cc_payload = i_rx_ring_prod_ptr[1*64 +: 64];
            REG_RX_RING1_DROP:  get_cc_payload = i_rx_ring_drop_count[1*64 +: 64];
            REG_RX_RING1_STAT:  get_cc_payload = i_rx_ring_status[1*64 +: 64];

        default: get_cc_payload = {64{1'b0}}; // Undefined registers return 0
        endcase
    end

endfunction

always @(posedge user_clk or posedge user_reset_p) begin
    if (user_reset_p) begin
        cc_valid            <= 1'b0;
        cc_requester_id     <= 16'h0;
        cc_tag              <= 8'h0;
        cc_tc               <= 3'h0;
        cc_lower_addr       <= 7'h0;
        cc_dword_count      <= 11'h0;
        cc_status           <= 3'b000; // Successful completion
        cc_payload          <= {64{1'b0}};
        cc_last             <= 1'b0;
    end else if (cc_state == CC_RESP && cc_ready) begin
                    cc_valid            <= 1'b1;
                    cc_requester_id     <= temp_cq_requester_id;
                    cc_tag              <= temp_cq_tag;
                    cc_tc               <= temp_cq_tc;
                    cc_lower_addr       <= temp_cq_lower_addr;
                    cc_dword_count      <= temp_cq_payload_dw_count;
                    cc_status           <= 3'b000; // Successful completion
                    cc_payload          <= {{64{1'b0}}, get_cc_payload(temp_cq_reg_addr)};
                    cc_last             <= 1'b1; // Single beat response
                end
    else if (cc_state == IDLE) begin
        cc_valid            <= 1'b0;
        cc_last             <= 1'b0; // Single beat response    
    end
    
end


function [10:0] get_rt_dw_count(input [63:0] ctrl);
    begin
        case (ctrl)
            RT_CTRL_SMALL: get_rt_dw_count = RT_SMALL_DW;
            RT_CTRL_LARGE: get_rt_dw_count = RT_LARGE_DW;
            RT_CTRL_LONG : get_rt_dw_count = RT_LONG_DW;
            default      : get_rt_dw_count = 11'd0;
        endcase
    end
endfunction

function [1:0] get_rt_beats(input [63:0] ctrl);
    begin
        case (ctrl)
            RT_CTRL_SMALL: get_rt_beats = RT_SMALL_BEATS;
            RT_CTRL_LARGE: get_rt_beats = RT_LARGE_BEATS;
            RT_CTRL_LONG : get_rt_beats = RT_LONG_BEATS;
            default      : get_rt_beats = 2'd0;
        endcase
    end
endfunction

function [12:0] get_rt_byte_count(input [63:0] ctrl);
    begin
        case (ctrl)
            RT_CTRL_SMALL: get_rt_byte_count = 13'd16;
            RT_CTRL_LARGE: get_rt_byte_count = 13'd48;
            RT_CTRL_LONG : get_rt_byte_count = 13'd80;
            default      : get_rt_byte_count = 13'd0;
        endcase
    end
endfunction

// RQ/RC roundtrip channel ========================================
    reg [2:0]  rt_state;

    localparam RT_IDLE       = 3'd0;
    localparam RT_SEND_READ  = 3'd1;
    localparam RT_WAIT_DATA  = 3'd2;
    localparam RT_SEND_WRITE = 3'd3;
    localparam RT_DONE       = 3'd4;
 
always @(posedge user_clk or posedge user_reset_p) begin
    if (user_reset_p) begin
        rt_state              <= RT_IDLE;
        in_val_REG_RT_STATUS  <= 64'b0;
        rt_expected_dw_count  <= 11'b0;
        rt_expected_beats     <= 2'b0;
        rt_expected_byte_count<= 13'b0;
        rc_beat_count         <= 2'b0;
        write_beat_index      <= 2'b0;
        for (i = 0; i < 3; i = i + 1) begin
            rt_buffer[i]         <= 256'b0;
            rt_buffer_dw_keep[i] <= 8'b0;
        end
    end else begin
        case (rt_state)
            RT_IDLE: begin
                in_val_REG_RT_STATUS   <= 64'b0;
                rc_beat_count          <= 2'b0;
                write_beat_index       <= 2'b0;
                if (cq_val_REG_RT_CTRL == RT_CTRL_SMALL ||
                    cq_val_REG_RT_CTRL == RT_CTRL_LARGE ||
                    cq_val_REG_RT_CTRL == RT_CTRL_LONG) begin
                    rt_expected_dw_count   <= get_rt_dw_count(cq_val_REG_RT_CTRL);
                    rt_expected_beats      <= get_rt_beats(cq_val_REG_RT_CTRL);
                    rt_expected_byte_count <= get_rt_byte_count(cq_val_REG_RT_CTRL);
                    in_val_REG_RT_STATUS   <= RT_STATUS_BUSY;
                    rt_state <= RT_SEND_READ;
                end
            end
            RT_SEND_READ: begin
                in_val_REG_RT_STATUS <= RT_STATUS_BUSY;
                if (rq_ready) begin
                    rc_beat_count <= 2'b0;
                    rt_state <= RT_WAIT_DATA;
                end
            end
            RT_WAIT_DATA: begin
                in_val_REG_RT_STATUS <= RT_STATUS_BUSY;
                if (rc_valid) begin
                    if (rc_err_code != 4'b0 || rc_posioned || rc_tag != 8'd1) begin
                        in_val_REG_RT_STATUS <= RT_STATUS_DONE | RT_STATUS_ERROR;
                        rt_state <= RT_DONE;
                    end else if (rc_beat_count >= rt_expected_beats) begin
                        in_val_REG_RT_STATUS <= RT_STATUS_DONE | RT_STATUS_ERROR;
                        rt_state <= RT_DONE;
                    end else begin
                        rt_buffer[rc_beat_count]         <= rc_payload;
                        rt_buffer_dw_keep[rc_beat_count] <= rc_payload_dw_keep;

                        if (rc_beat_count == 2'b0 &&
                            rc_payload_byte_count != rt_expected_byte_count) begin
                            in_val_REG_RT_STATUS <= RT_STATUS_DONE | RT_STATUS_ERROR;
                            rt_state <= RT_DONE;
                        end else if (rc_payload_last) begin
                            if ((rc_beat_count + 1'b1) != rt_expected_beats) begin
                                in_val_REG_RT_STATUS <= RT_STATUS_DONE | RT_STATUS_ERROR;
                                rt_state <= RT_DONE;
                            end else begin
                                write_beat_index <= 2'b0;
                                rt_state <= RT_SEND_WRITE;
                            end
                        end else begin
                            rc_beat_count <= rc_beat_count + 1'b1;
                        end
                    end
                end
            end
            RT_SEND_WRITE: begin
                in_val_REG_RT_STATUS <= RT_STATUS_BUSY;
                if (rq_ready) begin
                    if ((write_beat_index + 1'b1) == rt_expected_beats) begin
                        in_val_REG_RT_STATUS <= RT_STATUS_DONE;
                        rt_state <= RT_DONE;
                    end else
                        write_beat_index <= write_beat_index + 1'b1;
                end
            end
            RT_DONE: begin
                 if (cq_val_REG_RT_CTRL == 0)
                    rt_state <= RT_IDLE;
            end
            default: begin
                rt_state <= RT_IDLE;
            end
        endcase
    end
end

always @(posedge user_clk or posedge user_reset_p)  begin
    if (user_reset_p) begin
        rq_valid            <= 1'b0;
        rq_type             <= 4'b0;
        rq_payload_sop      <= 1'b0;
        rq_payload_last     <= 1'b0;
        rq_addr             <= 64'b0;
        rq_payload_dw_count <= 11'b0;
        rq_tag              <= 8'b0;
        rq_tc               <= 3'b0;
        rq_payload          <= 256'b0;
    end else begin
        case (rt_state)
            RT_SEND_READ: begin
                rq_valid            <= 1'b1;
                rq_type             <= TYPE_READ;
                rq_payload_sop      <= 1'b1;
                rq_payload_last     <= 1'b1;
                rq_addr             <= cq_val_REG_RT_SRC_ADDR;
                rq_payload_dw_count <= rt_expected_dw_count;
                rq_tag              <= 8'd1;
                rq_tc               <= 3'b000;
                rq_payload          <= 256'b0;
            end
            RT_SEND_WRITE: begin
                rq_valid            <= 1'b1;
                rq_type             <= TYPE_WRITE;
                rq_payload_sop      <= (write_beat_index == 2'b0);
                rq_payload_last     <= ((write_beat_index + 1'b1) == rt_expected_beats);
                rq_addr             <= cq_val_REG_RT_DST_ADDR;
                rq_payload_dw_count <= rt_expected_dw_count;
                rq_tag              <= 8'd2;
                rq_tc               <= 3'b000;
                rq_payload          <= rt_buffer[write_beat_index];
            end
            default: begin
                rq_valid            <= 1'b0;
                rq_payload_sop      <= 1'b0;
                rq_payload_last     <= 1'b0;
            end
        endcase
    end
end

endmodule
