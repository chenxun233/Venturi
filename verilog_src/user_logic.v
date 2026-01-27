// =============================================================================
// user_logic.v - User Logic for FPGA PCIe Hello World Example
// =============================================================================
// Implements register read/write and DMA control using the clean interface
// provided by pcie_interface.v (CQ_parser, CC_formatter, RQ_formatter).
//
// Register Map (BAR0):
//   0x00: Scratch Register (R/W)     - 64-bit scratch pad for testing
//   0x08: ID Register (RO)           - Returns 0xDEADBEEF_CAFEBABE
//   0x10: Interrupt Control (W)      - Write any value to trigger MSI interrupt
//   0x18: Status Register (RO)       - Bit 0: Link Up, Bits [31:16]: Int count
//   0x20: DMA Target Addr Low (W)    - Lower 32 bits of host memory IOVA
//   0x28: DMA Target Addr High (W)   - Upper 32 bits of host memory IOVA
//   0x30: DMA Control (W)            - Write 1 to trigger DMA write
//   0x38: DMA Status (RO)            - Bit 0: Busy, Bit 1: Done
// =============================================================================

module user_logic #(
    parameter DATA_WIDTH = 256,
    parameter BAR0_SIZE  = 16
)(
    input  wire                     clk,
    input  wire                     rst,
    // =========================================================================
    // CQ Parser Interface (Host → FPGA MMIO Requests)
    // =========================================================================
    (* MARK_DEBUG = "TRUE" *)   input  wire                     cq_valid,
    input  wire                     cq_is_write,
    (* MARK_DEBUG = "TRUE" *)   input  wire                     cq_is_read,
    (* MARK_DEBUG = "TRUE" *)   input  wire [BAR0_SIZE-1:0]     cq_reg_addr,
    input  wire [63:0]              cq_wr_data,
    input  wire [2:0]               cq_bar_id,
    (* MARK_DEBUG = "TRUE" *)   input  wire [15:0]              cq_requester_id,
    input  wire [7:0]               cq_tag,
    input  wire [2:0]               cq_tc,
    (* MARK_DEBUG = "TRUE" *)   input  wire [6:0]               cq_lower_addr,
    (* MARK_DEBUG = "TRUE" *)   input  wire [10:0]              cq_dword_count,

    // =========================================================================
    // CC Formatter Interface (FPGA → Host Read Responses)
    // =========================================================================
    input  wire                     cc_ready,
    output reg                      cc_valid,
    output reg  [15:0]              cc_requester_id,
    output reg  [7:0]               cc_tag,
    output reg  [2:0]               cc_tc,
    output reg  [6:0]               cc_lower_addr,
    output reg  [10:0]              cc_dword_count,
    output reg  [2:0]               cc_status,
    output reg  [DATA_WIDTH/2-1:0]  cc_data,
    output reg                      cc_last,

    // =========================================================================
    // RQ Formatter Interface (FPGA → Host DMA Requests)
    // =========================================================================
    input  wire                     rq_ready,
    output reg                      rq_valid,
    output reg                      rq_is_write,
    output reg                      rq_is_read,
    output reg                      rq_sop,
    output reg                      rq_last,
    output reg  [63:0]              rq_addr,
    output reg  [10:0]              rq_dword_count,
    output reg  [7:0]               rq_tag,
    output reg  [15:0]              rq_requester_id,
    output reg  [2:0]               rq_tc,
    output reg  [2:0]               rq_attr,
    output reg  [DATA_WIDTH-1:0]    rq_payload,
    output reg  [DATA_WIDTH / 32-1:0]    rq_payload_keep,

    // =========================================================================
    // RC Parser Interface (Host → FPGA DMA Read Completions)
    // =========================================================================
    input  wire                     rc_desc_valid,
    input  wire [7:0]               rc_tag,
    input  wire [2:0]               rc_status,
    input  wire [10:0]              rc_dword_count,
    input  wire [12:0]              rc_byte_count,
    input  wire [11:0]              rc_lower_addr,
    input  wire                     rc_request_completed,
    input  wire [3:0]               rc_error_code,
    input  wire                     rc_data_valid,
    input  wire                     rc_data_sop,
    input  wire                     rc_data_eop,
    input  wire [DATA_WIDTH-1:0]    rc_payload,
    input  wire [DATA_WIDTH / 32-1:0]    rc_payload_keep,

    // =========================================================================
    // Status Signals
    // =========================================================================
    input  wire                     user_lnk_up,
    output wire                     interrupt_out,
    output wire                     dma_busy_out
);


    // =========================================================================
    // Local Parameters - Register Addresses (DWord index)
    // =========================================================================
    localparam REG_SCRATCH      = 8'h00;  
    localparam REG_ID           = 8'h04; 
    localparam REG_INT_CTRL     = 8'h08; 
    localparam REG_STATUS       = 8'h0C; 
    localparam REG_DMA_ADDR_LO  = 8'h10; 
    localparam REG_DMA_ADDR_HI  = 8'h14; 
    localparam REG_DMA_CTRL     = 8'h18; 
    localparam REG_DMA_STATUS   = 8'h1C; 

    localparam MAGIC_ID         = 64'hDEADBEEF_CAFEBABE;

    // =========================================================================
    // State Machine
    // =========================================================================
    localparam ST_IDLE     = 2'b00;
    localparam ST_COMPLETE = 2'b01;
    localparam ST_DMA      = 2'b10;

    (* MARK_DEBUG = "TRUE" *)   reg [1:0]  state;

    // =========================================================================
    // User Registers
    // =========================================================================
    reg [63:0] scratch_reg;
    reg [15:0] interrupt_counter;
    reg        interrupt_pending;

    // DMA Registers
    reg [31:0] dma_addr_lo;
    reg [31:0] dma_addr_hi;
    reg        dma_busy;
    reg        dma_done;

    wire [63:0] dma_target_addr = {dma_addr_hi, dma_addr_lo};

    // Captured completer ID (our Bus:Dev:Func)

    // Saved descriptor fields for completion
    (* MARK_DEBUG = "TRUE" *)   reg [63:0] read_data;
    reg [15:0] saved_requester_id;
    reg [7:0]  saved_tag;
    reg [2:0]  saved_tc;
    reg [6:0]  saved_lower_addr;
    reg [10:0] saved_cq_dword_count;

    // =========================================================================
    // Register Address Decode
    // =========================================================================
    (* MARK_DEBUG = "TRUE" *)   wire [7:0] reg_addr = cq_reg_addr[7:0];

    // =========================================================================
    // Main State Machine
    // =========================================================================
    always @(posedge clk) begin
        if (rst) begin
            state <= ST_IDLE;
            cc_valid <= 1'b0;
            cc_requester_id <= 16'h0;
            cc_tag <= 8'h0;
            cc_tc <= 3'h0;
            cc_lower_addr <= 7'h0;
            cc_dword_count <= 11'h0;
            cc_status <= 3'h0;
            cc_data <= {(DATA_WIDTH/2){1'b0}};
            cc_last <= 1'b0;

            rq_valid <= 1'b0;
            rq_is_write <= 1'b0;
            rq_is_read <= 1'b0;
            rq_sop <= 1'b0;
            rq_last <= 1'b0;
            rq_addr <= 64'h0;
            rq_dword_count <= 11'h0;
            rq_tag <= 8'h0;
            rq_requester_id <= 16'h0;
            rq_tc <= 3'h0;
            rq_attr <= 3'h0;
            rq_payload <= {DATA_WIDTH{1'b0}};
            rq_payload_keep <= {DATA_WIDTH / 32{1'b0}};

            scratch_reg <= 64'h0;
            interrupt_counter <= 16'h0;
            interrupt_pending <= 1'b0;
            dma_addr_lo <= 32'h0;
            dma_addr_hi <= 32'h0;
            dma_busy <= 1'b0;
            dma_done <= 1'b0;

            read_data <= 64'h0;
            saved_requester_id <= 16'h0;
            saved_tag <= 8'h0;
            saved_tc <= 3'h0;
            saved_lower_addr <= 7'h0;
            saved_cq_dword_count<=0;
        end else begin
            // Default: clear single-cycle signals
            cc_valid <= 1'b0;
            rq_valid <= 1'b0;

            case (state)
                // =============================================================
                // IDLE: Wait for CQ request
                // =============================================================
                ST_IDLE: begin
                    if (cq_valid) begin
                        // Capture our completer ID from CQ tuser
                        // (This comes through cq_requester_id for the host,
                        //  but we need a separate signal for our ID -
                        //  for now use a fixed value or add a port)

                        if (cq_is_write) begin
                            // -------------------------------------------------
                            // Memory Write - Update registers
                            // -------------------------------------------------
                            case (reg_addr)
                                REG_SCRATCH: begin
                                    scratch_reg <= cq_wr_data;
                                end
                                REG_INT_CTRL: begin
                                    interrupt_pending <= 1'b1;
                                    interrupt_counter <= interrupt_counter + 1'b1;
                                end
                                REG_DMA_ADDR_LO: begin
                                    dma_addr_lo <= cq_wr_data[31:0];
                                end
                                REG_DMA_ADDR_HI: begin
                                    dma_addr_hi <= cq_wr_data[31:0];
                                end
                                REG_DMA_CTRL: begin
                                    if (cq_wr_data[0] && !dma_busy) begin
                                        dma_busy <= 1'b1;
                                        dma_done <= 1'b0;
                                        state <= ST_DMA;
                                    end
                                end
                                default: begin
                                    // Ignore writes to other addresses
                                end
                            endcase
                            // Writes don't need completion (posted)
                        end else if (cq_is_read) begin
                            // -------------------------------------------------
                            // Memory Read - Prepare completion
                            // -------------------------------------------------
                            saved_requester_id <= cq_requester_id;
                            saved_tag <= cq_tag;
                            saved_tc <= cq_tc;
                            saved_lower_addr <= cq_lower_addr;
                            saved_cq_dword_count<= cq_dword_count;
                            case (reg_addr)
                                REG_SCRATCH: begin
                                    read_data <= scratch_reg;
                                end
                                REG_ID: begin
                                    read_data <= MAGIC_ID;
                                end
                                REG_STATUS: begin
                                    read_data <= {32'h0, interrupt_counter, 15'h0, user_lnk_up};
                                end
                                REG_DMA_STATUS: begin
                                    read_data <= {62'h0, dma_done, dma_busy};
                                end
                                default: begin
                                    read_data <= 64'hDEAD_DEAD_DEAD_DEAD;
                                end
                            endcase
                            state <= ST_COMPLETE;
                        end
                    end

                    // Clear interrupt pending after some cycles (simplified)
                    if (interrupt_pending) begin
                        interrupt_pending <= 1'b0;
                    end
                end

                // =============================================================
                // COMPLETE: Send read completion via CC
                // =============================================================
                ST_COMPLETE: begin
                    if (cc_ready) begin
                        cc_valid <= 1'b1;
                        cc_requester_id <= saved_requester_id;
                        cc_tag <= saved_tag;
                        cc_tc <= saved_tc;
                        cc_lower_addr <= saved_lower_addr;
                        cc_dword_count <= saved_cq_dword_count;        // 2 DWords = 8 bytes
                        cc_status <= 3'b000;            // Successful completion
                        cc_data <= {{(DATA_WIDTH/2-64){1'b0}}, read_data};
                        cc_last <= 1'b1;

                        state <= ST_IDLE;
                    end
                end

                // =============================================================
                // DMA: Send DMA write request via RQ
                // =============================================================
                ST_DMA: begin
                    if (rq_ready && dma_busy) begin
                        rq_valid <= 1'b1;
                        rq_is_write <= 1'b1;
                        rq_is_read <= 1'b0;
                        rq_sop <= 1'b1;
                        rq_last <= 1'b1;                // Single beat DMA
                        rq_addr <= dma_target_addr;
                        rq_dword_count <= 11'd4;        // 4 DWords = 16 bytes
                        rq_tag <= 8'h42;                // Fixed tag for DMA
                        rq_tc <= 3'b0;
                        rq_attr <= 3'b0;
                        // Payload: Test pattern
                        rq_payload <= {128'h0, 64'hCAFEBABE_12345678, 64'hDEADBEEF_AABBCCDD};
                        rq_payload_keep <= 8'hFF;

                        dma_busy <= 1'b0;
                        dma_done <= 1'b1;
                        state <= ST_IDLE;
                    end
                end

                default: state <= ST_IDLE;
            endcase
        end
    end

    // =========================================================================
    // Status Outputs
    // =========================================================================
    assign interrupt_out = interrupt_pending;
    assign dma_busy_out = dma_busy;

endmodule
