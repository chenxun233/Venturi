`timescale 1ns / 1ps

module async_fifo #(
    parameter DEPTH  = 2,
    parameter DATA_W = 274
) (
    input  wire              i_wr_clk,
    input  wire              i_wr_rst,
    input  wire              i_wr_en,
    input  wire [DATA_W-1:0] i_wr_data,
    input  wire              i_rd_clk,
    input  wire              i_rd_rst,
    input  wire              i_rd_en,
    output wire              o_wr_full,
    output wire              o_rd_empty,
    output reg               o_rd_valid,
    output reg  [DATA_W-1:0] o_rd_data
);

function integer clog2 (input integer value);
    integer i;
    begin
        clog2 = 0;
        for (i = value - 1; i > 0; i = i >> 1)
            clog2 = clog2 + 1;
    end
endfunction

function [PTR_W-1:0] bin_2_gray (input [PTR_W-1:0] bin);
    begin
        bin_2_gray = bin ^ (bin >> 1);
    end
endfunction

function [0:0] is_full (
    input [PTR_W-1:0] rd_gray,
    input [PTR_W-1:0] wr_gray
);
    begin
        if (PTR_W > 2)
            is_full = (wr_gray == {~rd_gray[PTR_W-1:PTR_W-2], rd_gray[PTR_W-3:0]});
        else
            is_full = (wr_gray == ~rd_gray);
    end
endfunction

localparam ADDR_W = (DEPTH > 1) ? clog2(DEPTH) : 1;
localparam PTR_W  = ADDR_W + 1;

reg [DATA_W-1:0] mem [0:DEPTH-1];

reg  [PTR_W-1:0] wr_ptr_bin;
reg  [PTR_W-1:0] rd_ptr_bin;



wire  wr_do;
wire  rd_do;


assign wr_do = i_wr_en && ! o_wr_full;
assign rd_do = i_rd_en && ! o_rd_empty;


// the follow two are used because they are the first stage crossing time, they should be stable.
reg  [PTR_W-1:0] wr_ptr_gray;
reg  [PTR_W-1:0] rd_ptr_gray;
// When above have crossed the time domain, they become these:
wire [PTR_W-1:0] wr_ptr_gray_rd;
wire [PTR_W-1:0] rd_ptr_gray_wr;

assign o_wr_full  = is_full(rd_ptr_gray_wr, wr_ptr_gray);
assign o_rd_empty = (wr_ptr_gray_rd == rd_ptr_gray);



always @(posedge i_wr_clk or posedge i_wr_rst) begin
    if (i_wr_rst) begin
        wr_ptr_bin                          <= 0;
        wr_ptr_gray                         <= 0;
    end else begin
        if (wr_do) begin
            mem[wr_ptr_bin[ADDR_W-1:0]]     <= i_wr_data;
            wr_ptr_bin                      <= wr_ptr_bin+1;
            wr_ptr_gray                     <= bin_2_gray(wr_ptr_bin+1);
        end
    end
end

always @(posedge i_rd_clk or posedge i_rd_rst) begin
    if (i_rd_rst) begin
        o_rd_valid      <= 0;
        rd_ptr_bin      <= 0;
        rd_ptr_gray     <= 0;
        o_rd_data       <= 0;
    end else begin
        o_rd_valid <= rd_do;               // Bug 1 fix: deasserts when no read
        if (rd_do) begin
            o_rd_data   <= mem[rd_ptr_bin[ADDR_W-1:0]];
            rd_ptr_bin  <= rd_ptr_bin + 1;
            rd_ptr_gray <= bin_2_gray(rd_ptr_bin + 1);
        end
    end
end


ff_sync #(
    .PTR_W (PTR_W)
) wr_2_rd (
    .i_clk (i_rd_clk),
    .i_rst (i_rd_rst),
    .i_ptr (wr_ptr_gray),
    .o_ptr (wr_ptr_gray_rd)
);

ff_sync #(
    .PTR_W (PTR_W)
) rd_2_wr (
    .i_clk (i_wr_clk),
    .i_rst (i_wr_rst),
    .i_ptr (rd_ptr_gray),
    .o_ptr (rd_ptr_gray_wr)
);


endmodule

module ff_sync #(
    parameter PTR_W = 2
) (
    input  wire             i_clk,
    input  wire             i_rst,
    input  wire [PTR_W-1:0] i_ptr,
    output wire [PTR_W-1:0] o_ptr
);

(* ASYNC_REG = "TRUE" *) reg [PTR_W-1:0] sync_reg_1;
(* ASYNC_REG = "TRUE" *) reg [PTR_W-1:0] sync_reg_2;
always @(posedge i_clk or posedge i_rst) begin
    if (i_rst) begin
        sync_reg_1 <= {PTR_W{1'b0}};
        sync_reg_2 <= {PTR_W{1'b0}};
    end else begin
        sync_reg_1 <= i_ptr;
        sync_reg_2 <= sync_reg_1;
    end
end

assign o_ptr = sync_reg_2;

endmodule
