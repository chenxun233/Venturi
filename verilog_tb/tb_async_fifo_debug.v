`timescale 1ns / 1ps

module tb_async_fifo_debug;

    localparam CLK_WR_PERIOD = 10;
    localparam CLK_RD_PERIOD = 14;
    localparam DEPTH = 2;
    localparam DATA_W = 8;

    reg                 wr_clk;
    reg                 rd_clk;
    reg                 wr_rst;
    reg                 rd_rst;
    reg                 wr_en;
    reg  [DATA_W-1:0]   wr_data;
    wire                wr_full;
    wire [63:0]         wr_drop_count;
    reg                 rd_en;
    wire                rd_empty;
    wire                rd_valid;
    wire [DATA_W-1:0]   rd_data;

    integer read_count;

    async_fifo #(
        .DEPTH  (DEPTH),
        .DATA_W (DATA_W)
    ) dut (
        .i_wr_clk        (wr_clk),
        .i_wr_rst        (wr_rst),
        .i_wr_en         (wr_en),
        .i_wr_data       (wr_data),
        .o_wr_full       (wr_full),
        .o_wr_drop_count (wr_drop_count),
        .i_rd_clk        (rd_clk),
        .i_rd_rst        (rd_rst),
        .i_rd_en         (rd_en),
        .o_rd_empty      (rd_empty),
        .o_rd_valid      (rd_valid),
        .o_rd_data       (rd_data)
    );

    always #(CLK_WR_PERIOD/2) wr_clk = ~wr_clk;
    always #(CLK_RD_PERIOD/2) rd_clk = ~rd_clk;

    always @(posedge rd_clk) begin
        if (rd_valid) begin
            read_count <= read_count + 1;
        end
    end

    initial begin
        wr_clk = 1'b0;
        rd_clk = 1'b0;
        wr_rst = 1'b1;
        rd_rst = 1'b1;
        wr_en = 1'b0;
        wr_data = {DATA_W{1'b0}};
        rd_en = 1'b0;
        read_count = 0;

        repeat (4) @(posedge wr_clk);
        repeat (4) @(posedge rd_clk);
        wr_rst = 1'b0;
        rd_rst = 1'b0;

        @(posedge wr_clk);
        wr_en <= 1'b1;
        wr_data <= 8'h11;
        @(posedge wr_clk);
        wr_data <= 8'h22;
        @(posedge wr_clk);
        wr_data <= 8'h33;
        @(posedge wr_clk);
        wr_en <= 1'b0;
        wr_data <= 8'h00;

        repeat (6) @(posedge wr_clk);
        if (wr_drop_count == 64'd0) begin
            $fatal(1, "wr_drop_count should increment when writes hit full FIFO");
        end

        repeat (4) @(posedge rd_clk);
        rd_en <= 1'b1;
        repeat (4) @(posedge rd_clk);
        rd_en <= 1'b0;

        repeat (4) @(posedge rd_clk);
        if (read_count != 2) begin
            $fatal(1, "Expected to read exactly 2 retained entries, got %0d", read_count);
        end
        if (wr_drop_count != 64'd1) begin
            $fatal(1, "Expected exactly one dropped write, got %0d", wr_drop_count);
        end

        $display("PASS: async_fifo write-drop counter validated");
        $finish;
    end

endmodule
