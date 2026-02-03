`timescale 1ns / 1ps

module tb_RC_gearbox256;

    // =========================================================================
    // Parameters
    // =========================================================================
    parameter DATA_WIDTH = 256;
    parameter CLK_PERIOD = 10;

    // =========================================================================
    // DUT Signals
    // =========================================================================
    reg                         clk;
    reg                         rst_n;
    // PCIe IP Core Interface (Input)
    reg  [DATA_WIDTH-1:0]       m_axis_rc_tdata;
    reg                         m_axis_rc_tvalid;
    reg  [74:0]                 m_axis_rc_tuser; // Bit 32 is is_sof_0
    reg  [DATA_WIDTH/32-1:0]    m_axis_rc_tkeep;
    reg                         m_axis_rc_tlast;
    wire                        m_axis_rc_tready;

    // User Interface (Output)
    wire                        rc_valid;
    wire                        rc_payload_last;
    wire [255:0]                rc_payload;
    wire [7:0]                  rc_payload_dw_keep;
    wire [95:0]                 rc_descriptor;

    wire            sop             = dut.sop;
    wire [12:0]     byte_count      = dut.byte_count;
    // =========================================================================
    // DUT Instantiation
    // =========================================================================
    // Note: Ensure your RC_gearbox256 module ports match these names
    RC_gearbox256 #(
        .DATA_WIDTH(DATA_WIDTH)
    ) dut (
        .clk                    (clk),
        .rst_n                  (rst_n),
        .m_axis_rc_tdata        (m_axis_rc_tdata),
        .m_axis_rc_tvalid       (m_axis_rc_tvalid),
        .m_axis_rc_tuser        (m_axis_rc_tuser),
        .m_axis_rc_tkeep        (m_axis_rc_tkeep),
        .m_axis_rc_tlast        (m_axis_rc_tlast),
        .m_axis_rc_tready       (m_axis_rc_tready),
        
        .rc_valid               (rc_valid),
        .rc_payload_last        (rc_payload_last),
        .rc_payload             (rc_payload),
        .rc_payload_dw_keep     (rc_payload_dw_keep),
        .rc_descriptor          (rc_descriptor)
    );

    // =========================================================================
    // Clock Generation
    // =========================================================================
    initial begin
        clk = 1;
        forever #(CLK_PERIOD/2) clk = ~clk;
    end

    // =========================================================================
    // Helper Tasks
    // =========================================================================

    // Task: Reset
    task apply_reset;
        begin
            m_axis_rc_tdata  = 0;
            m_axis_rc_tvalid = 0;
            m_axis_rc_tuser  = 0;
            m_axis_rc_tkeep  = 0;
            m_axis_rc_tlast  = 0;
        end
    endtask


    wire [95:0]     descriptor_1 = {{67{1'b1}},13'b00000_0001_0100,{16{1'b1}}};
    wire [95:0]     descriptor_2 = {{67{1'b1}},13'b00000_0011_0100,{16{1'b1}}};
    wire [95:0]     descriptor_3 = {{67{1'b1}},13'b00000_0101_0100,{16{1'b1}}};
    wire [159:0]    payload_1   = 159'hAA10_AAA9_AAA8_AAA7_AAA6_AAA5_AAA4_AAA3__AAA2_AAA1;
    wire [255:0]    payload_2   = 256'hBB16_BB15_BB14_BB13_BB12_BB11_BB10_BBB9_BBB8_BBB7_BBB6_BBB5_BBB4_BBB3_BBB2_BBB1;
    wire [255:0]    payload_3   = 256'hCC16_CC15_CC14_CC13_CC12_CC11_CC10_CCC9_CCC8_CCC7_CCC6_CCC5_CCC4_CCC3_CCC2_CCC1;


    // =========================================================================
    // Main Test Sequence
    // =========================================================================
    initial begin
        rst_n = 0;
        #CLK_PERIOD;
        rst_n = 1;
        #CLK_PERIOD;

        // =====================================================================
        // Burst 1: Single-cycle transfer (256 bits = payload_1 + descriptor_1)
        // descriptor_1[28:16] = 20 bytes (payload_1 is 160 bits = 20 bytes)
        // tvalid=1, tlast=1 at time 1
        // =====================================================================
        m_axis_rc_tdata  = {payload_1, descriptor_1};
        m_axis_rc_tvalid = 1;
        m_axis_rc_tuser  = {{42{1'b0}}, 1'b1, {32{1'b0}}};  // tuser[32] = 1 (SOP)
        m_axis_rc_tkeep  = 8'hFF;
        m_axis_rc_tlast  = 1;
        #CLK_PERIOD;
        apply_reset();
        #CLK_PERIOD;

        // =====================================================================
        // Burst 2: Two-cycle transfer
        // descriptor_2[28:16] = 52 bytes (payload_1 + payload_2 = 20 + 32 bytes)
        // Time 1: payload_1 + descriptor_2, tlast=0
        // Time 2: payload_2, tlast=1
        // =====================================================================
        m_axis_rc_tdata  = {payload_1, descriptor_2};
        m_axis_rc_tvalid = 1;
        m_axis_rc_tuser  = {{42{1'b0}}, 1'b1, {32{1'b0}}};  // tuser[32] = 1 (SOP)
        m_axis_rc_tkeep  = 8'hFF;
        m_axis_rc_tlast  = 0;
        #CLK_PERIOD;
        m_axis_rc_tdata  = payload_2;
        m_axis_rc_tvalid = 1;
        m_axis_rc_tuser  = 0;  // tuser[32] = 0 (not SOP)
        m_axis_rc_tkeep  = 8'hFF;
        m_axis_rc_tlast  = 1;
        #CLK_PERIOD;
        apply_reset();
        #CLK_PERIOD;

        // =====================================================================
        // Burst 3: Three-cycle transfer
        // descriptor_3[28:16] = 84 bytes (payload_1 + payload_2 + payload_3 = 20 + 32 + 32 bytes)
        // Time 1: payload_1 + descriptor_3, tlast=0
        // Time 2: payload_2, tlast=0
        // Time 3: payload_3, tlast=1
        // =====================================================================
        m_axis_rc_tdata  = {payload_1, descriptor_3};
        m_axis_rc_tvalid = 1;
        m_axis_rc_tuser  = {{42{1'b0}}, 1'b1, {32{1'b0}}};  // tuser[32] = 1 (SOP)
        m_axis_rc_tkeep  = 8'hFF;
        m_axis_rc_tlast  = 0;
        #CLK_PERIOD;
        m_axis_rc_tdata  = payload_2;
        m_axis_rc_tvalid = 1;
        m_axis_rc_tuser  = 0;  // tuser[32] = 0 (not SOP)
        m_axis_rc_tkeep  = 8'hFF;
        m_axis_rc_tlast  = 0;
        #CLK_PERIOD;
        m_axis_rc_tdata  = payload_3;
        m_axis_rc_tvalid = 1;
        m_axis_rc_tuser  = 0;  // tuser[32] = 0 (not SOP)
        m_axis_rc_tkeep  = 8'hFF;
        m_axis_rc_tlast  = 1;
        #CLK_PERIOD;
        apply_reset();
        #(CLK_PERIOD*5);
        $finish;
    end
    
    // Waveform Setup
    initial begin
        // $dumpfile("tb_RC_gearbox256.vcd");
        $dumpvars(0, tb_RC_gearbox256);
    end

endmodule