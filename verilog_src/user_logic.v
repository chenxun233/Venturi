// =============================================================================
// user_logic.v - User Logic for FPGA PCIe Hello World Example
// =============================================================================

// =============================================================================

module user_logic #(
    parameter DATA_WIDTH = 256,
    parameter BAR0_SIZE  = 16
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
    input wire [15:0]                   pcie_requester_id
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

    reg [255:0] small_buffer            ;
    reg [7:0]   small_buffer_dw_keep  ;
    reg [12:0]  small_byte_count ;
    reg [255:0] large_buffer [1:0]      ;
    reg [7:0]   large_buffer_dw_keep [1:0] ;
    reg [12:0]  large_byte_count [1:0] ;
    reg [1:0]   beat_cnt                ;


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


// RQ channel ======================================================
    reg [2:0]  rt_state;
    reg        rt_rc_received;
    reg        write_beat_upper; 

    localparam RT_IDLE       = 3'd0;
    localparam RT_SEND_READ  = 3'd1;
    localparam RT_WAIT_DATA  = 3'd2;
    localparam RT_SEND_WRITE = 3'd3;
    localparam RT_DONE       = 3'd4;
 
always @(posedge user_clk or posedge user_reset_p) begin
    if (user_reset_p) begin
        rt_state           <= RT_IDLE;
        in_val_REG_RT_STATUS <= 64'b0;
    end else begin
        case (rt_state)
            RT_IDLE: begin
                if (cq_val_REG_RT_CTRL == 1 || cq_val_REG_RT_CTRL == 2)
                    rt_state <= RT_SEND_READ;
            end
            RT_SEND_READ: begin
                if (rq_ready)
                    rt_state <= RT_WAIT_DATA;
            end
            RT_WAIT_DATA: begin
                if (rt_rc_received)
                    rt_state <= RT_SEND_WRITE;
            end
            RT_SEND_WRITE: begin
                if (rq_ready) begin
                    if (cq_val_REG_RT_CTRL == 1) begin
                        rt_state <= RT_DONE;
                    end else if (cq_val_REG_RT_CTRL == 2) begin
                        // Only transition to DONE when second beat is sent AND accepted
                        if (write_beat_upper && rq_payload_last)
                            rt_state <= RT_DONE;
                    end
                end
            end
            RT_DONE: begin
                 in_val_REG_RT_STATUS <= 64'd2; // Set Done bit
                 if (cq_val_REG_RT_CTRL == 0)
                    rt_state <= RT_IDLE;
            end
            default: begin
                rt_state <= rt_state;
            end
        endcase

        if (rt_state == RT_IDLE)
            in_val_REG_RT_STATUS <= 64'd0;
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
        write_beat_upper    <= 1'b0;
    end else begin
        case (rt_state)
            RT_SEND_READ: begin
                rq_valid            <= 1'b1;
                rq_type             <= TYPE_READ;
                rq_payload_sop      <= 1'b1;
                rq_payload_last     <= 1'b1;
                rq_addr             <= cq_val_REG_RT_SRC_ADDR;
                rq_payload_dw_count <= (cq_val_REG_RT_CTRL == 1) ? 11'd4 : 11'd12;
                rq_tag              <= 8'd1;
            end
            RT_SEND_WRITE: begin
                rq_valid            <= 1'b1;
                rq_type             <= TYPE_WRITE;
                rq_addr             <= cq_val_REG_RT_DST_ADDR;
                rq_tag              <= 8'd2;

                if (cq_val_REG_RT_CTRL == 1) begin
                    // Small Trip: 1 Beat
                    rq_payload_sop      <= 1'b1;
                    rq_payload_last     <= 1'b1;
                    rq_payload_dw_count <= 11'd4;
                    rq_payload          <= small_buffer;
                end else begin
                    // Large Trip: 2 Beats for 48 bytes (384 bits)
                    rq_payload_dw_count <= 11'd12;
                    if (!write_beat_upper) begin
                         // First Beat (Lower 256 bits)
                         rq_payload_sop      <= 1'b1;
                         rq_payload_last     <= 1'b0;
                         rq_payload          <= large_buffer[0];
                         if (rq_ready) write_beat_upper <= 1'b1;
                    end else begin
                         // Second Beat (Upper 128 bits)
                         rq_payload_sop      <= 1'b0;
                         rq_payload_last     <= 1'b1;
                         rq_payload          <= large_buffer[1];
                         if (rq_ready) write_beat_upper <= 1'b0;
                    end
                end
            end
            RT_DONE: begin
                 write_beat_upper <= 1'b0;
                 rq_valid <= 1'b0;
            end
            default: begin
                 rq_valid <= 1'b0;
            end
        endcase
    end
end

// RC channel ======================================================
// RC channel just do waiting and saving.
always @(posedge user_clk or posedge user_reset_p)  begin
    if (user_reset_p) begin
        small_buffer    <= 256'b0;
        small_buffer_dw_keep <= 8'b0;
        small_byte_count <= 13'b0;
        large_buffer[0] <= 256'b0;
        large_buffer[1] <= 256'b0;
        large_buffer_dw_keep[0] <= 8'b0;
        large_buffer_dw_keep[1] <= 8'b0;
        large_byte_count[0] <= 13'b0;
        large_byte_count[1] <= 13'b0;
        beat_cnt        <= 2'b0;
        rt_rc_received  <= 1'b0;
    end else begin
        // Reset flag when starting a new operation or explicitly in IDLE
        if (rt_state == RT_IDLE) 
            rt_rc_received <= 1'b0;

        if (rc_valid) begin
            if (cq_val_REG_RT_CTRL == 1) begin
                small_buffer <= rc_payload;
                small_buffer_dw_keep <= rc_payload_dw_keep;
                small_byte_count <= rc_payload_byte_count;
                if (rc_payload_last) rt_rc_received <= 1'b1;

            end else if (cq_val_REG_RT_CTRL == 2) begin
                large_buffer[beat_cnt] <= rc_payload;
                large_buffer_dw_keep[beat_cnt] <= rc_payload_dw_keep;
                large_byte_count[beat_cnt] <= rc_payload_byte_count;
                
                if (rc_payload_last) begin
                    beat_cnt <= 2'b0;
                    rt_rc_received <= 1'b1;
                end else
                    beat_cnt <= beat_cnt + 1'b1;
            end
        end
    end
end



endmodule
