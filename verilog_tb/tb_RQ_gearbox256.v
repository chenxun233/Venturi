`timescale 1ns / 1ps

module tb_RQ_gearbox256;

    // =========================================================================
    // Parameters
    // =========================================================================
    parameter DATA_WIDTH = 256;
    parameter CLK_PERIOD = 10;

    // =========================================================================
    // DUT Signals
    // =========================================================================
    reg                     clk;
    reg                     rst_n;
    
    // User Interface
    reg  [127:0]            descriptor;
    reg  [255:0]            rq_wr_data;
    reg  [10:0]             rq_dword_count;
    reg                     rq_last;
    reg                     rq_valid;
    reg                     rq_sop;
    wire                    rq_ready;
    
    // PCIe IP Core Interface
    wire [DATA_WIDTH-1:0]   s_axis_rq_tdata;
    wire                    s_axis_rq_tvalid;
    wire [59:0]             s_axis_rq_tuser;
    wire [7:0]              s_axis_rq_tkeep;
    wire                    s_axis_rq_tlast;
    wire                    one_more_cycle;
    reg                     s_axis_rq_tready;

    // =========================================================================
    // DUT Instantiation
    // =========================================================================
    assign one_more_cycle = dut.one_more_cycle;

    RQ_gearbox256 #(
        .DATA_WIDTH(DATA_WIDTH)
    ) dut (
        .clk                (clk),
        .rst_n              (rst_n),
        .descriptor         (descriptor),
        .rq_wr_data         (rq_wr_data),
        .rq_dword_count     (rq_dword_count),
        .rq_last            (rq_last),
        .rq_valid           (rq_valid),
        .rq_sop             (rq_sop),
        .rq_ready           (rq_ready),
        .s_axis_rq_tdata    (s_axis_rq_tdata),
        .s_axis_rq_tvalid   (s_axis_rq_tvalid),
        .s_axis_rq_tuser    (s_axis_rq_tuser),
        .s_axis_rq_tkeep    (s_axis_rq_tkeep),
        .s_axis_rq_tlast    (s_axis_rq_tlast),
        .s_axis_rq_tready   (s_axis_rq_tready)
    );

    // =========================================================================
    // Clock Generation
    // =========================================================================
    initial begin
        clk = 0;
        forever #(CLK_PERIOD/2) clk = ~clk;
    end

    // =========================================================================
    // Monitor
    // =========================================================================
    always @(posedge clk) begin
        if (s_axis_rq_tvalid && s_axis_rq_tready) begin
            $display("[%0t] OUT: tdata=%h, tkeep=%b, tlast=%b",
                     $time, s_axis_rq_tdata, s_axis_rq_tkeep, s_axis_rq_tlast);
        end
    end

    // =========================================================================
    // Main Test
    // =========================================================================
    initial begin
        $display("========== RQ_gearbox256 Testbench ==========");
        
        // Initialize
        rst_n            = 0;
        descriptor       = 0;
        rq_wr_data       = 0;
        rq_dword_count   = 0;
        rq_last          = 0;
        rq_valid         = 0;
        rq_sop           = 0;
        s_axis_rq_tready = 1;
        
        // Reset
        repeat(5) @(posedge clk);
        rst_n = 1;
        repeat(2) @(posedge clk);

        // -----------------------------------------------------------------
        // Test 1: dw_count = 1 (no one_more_cycle, keep=0x1F)
        // -----------------------------------------------------------------
        $display("\n[TEST] dw_count = 1");
        @(posedge clk);
        descriptor     <= 128'hAAAA_BBBB_CCCC_DDDD_1111_2222_3333_4444;
        rq_wr_data     <= 256'h0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_DEAD_0001;
        rq_dword_count <= 11'd1;
        rq_valid       <= 1;
        rq_sop         <= 1;
        rq_last        <= 1;
        @(posedge clk);
        rq_valid <= 0; rq_sop <= 0; rq_last <= 0;
        repeat(3) @(posedge clk);

        // -----------------------------------------------------------------
        // Test 2: dw_count = 2 (no one_more_cycle, keep=0x3F)
        // -----------------------------------------------------------------
        $display("\n[TEST] dw_count = 2");
        @(posedge clk);
        descriptor     <= 128'hAAAA_BBBB_CCCC_DDDD_1111_2222_3333_4444;
        rq_wr_data     <= 256'h0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_DEAD_0002_DEAD_0001;
        rq_dword_count <= 11'd2;
        rq_valid       <= 1;
        rq_sop         <= 1;
        rq_last        <= 1;
        @(posedge clk);
        rq_valid <= 0; rq_sop <= 0; rq_last <= 0;
        repeat(3) @(posedge clk);

        // -----------------------------------------------------------------
        // Test 3: dw_count = 4 (no one_more_cycle, keep=0xFF)
        // -----------------------------------------------------------------
        $display("\n[TEST] dw_count = 4");
        @(posedge clk);
        descriptor     <= 128'hAAAA_BBBB_CCCC_DDDD_1111_2222_3333_4444;
        rq_wr_data     <= 256'h0000_0000_0000_0000_0000_0000_0000_0000_DEAD_0004_DEAD_0003_DEAD_0002_DEAD_0001;
        rq_dword_count <= 11'd4;
        rq_valid       <= 1;
        rq_sop         <= 1;
        rq_last        <= 1;
        @(posedge clk);
        rq_valid <= 0; rq_sop <= 0; rq_last <= 0;
        repeat(3) @(posedge clk);

        // -----------------------------------------------------------------
        // Test 4: dw_count = 7 (one_more_cycle, keep=0x07)
        // -----------------------------------------------------------------
        $display("\n[TEST] dw_count = 7");
        @(posedge clk);
        descriptor     <= 128'hAAAA_BBBB_CCCC_DDDD_1111_2222_3333_4444;
        rq_wr_data     <= 256'h0000_0000_DEAD_0007_DEAD_0006_DEAD_0005_DEAD_0004_DEAD_0003_DEAD_0002_DEAD_0001;
        rq_dword_count <= 11'd7;
        rq_valid       <= 1;
        rq_sop         <= 1;
        rq_last        <= 1;
        @(posedge clk);
        rq_valid <= 0; rq_sop <= 0; rq_last <= 0;
        repeat(5) @(posedge clk);  // Extra cycles for one_more_cycle

        // -----------------------------------------------------------------
        // Test 5: dw_count = 9 (2 user beats + one_more_cycle, keep=0x01)
        // -----------------------------------------------------------------
        $display("\n[TEST] dw_count = 9");
        // Beat 1 (SOP)
        @(posedge clk);
        descriptor     <= 128'hAAAA_BBBB_CCCC_DDDD_1111_2222_3333_4444;
        rq_wr_data     <= 256'hDEAD_0008_DEAD_0007_DEAD_0006_DEAD_0005_DEAD_0004_DEAD_0003_DEAD_0002_DEAD_0001;
        rq_dword_count <= 11'd9;
        rq_valid       <= 1;
        rq_sop         <= 1;
        rq_last        <= 0;
        @(posedge clk);
        // Beat 2 (LAST)
        descriptor     <= 0;
        rq_wr_data     <= 256'h0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_0000_DEAD_0009;
        rq_sop         <= 0;
        rq_last        <= 1;
        @(posedge clk);
        rq_valid <= 0; rq_sop <= 0; rq_last <= 0;
        repeat(5) @(posedge clk);

        // -----------------------------------------------------------------
        // Done
        // -----------------------------------------------------------------
        $display("\n========== All Tests Completed ==========\n");
        $finish;
    end

    // Waveform dump
    initial begin
        $dumpfile("tb_RQ_gearbox256.vcd");
        $dumpvars(0, tb_RQ_gearbox256);
    end

endmodule
