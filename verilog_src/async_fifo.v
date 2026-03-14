
module async_fifo #(
    parameter DEPTH  = 2,
    parameter DATA_W = 274
) (
    input  wire              i_wr_clk,
    input  wire              i_wr_rst,
    input  wire              i_wr_en,
    input  wire [DATA_W-1:0] i_wr_data,
    output reg               o_wr_full,
    input  wire              i_rd_clk,
    input  wire              i_rd_rst,
    input  wire              i_rd_en,
    output reg               o_rd_empty,
    output reg               o_rd_valid,
    output reg [DATA_W-1:0]  o_rd_data
);

function integer clog2;
    input integer value;
    integer i;
    begin
        clog2 = 0;
        for (i = value - 1; i > 0; i = i >> 1)
            clog2 = clog2 + 1;
    end
endfunction

localparam ADDR_W = (DEPTH > 1) ? clog2(DEPTH) : 1;
localparam PTR_W  = ADDR_W + 1; //PTR_W is POINTER_WIDTH, the bit width of the read/write pointers.

reg [DATA_W-1:0] mem [0:DEPTH-1];

reg [PTR_W-1:0] wr_ptr_bin;  // binary version
reg [PTR_W-1:0] wr_ptr_gray; // gray code counterpart
reg [PTR_W-1:0] rd_ptr_bin;
reg [PTR_W-1:0] rd_ptr_gray;

(* ASYNC_REG = "TRUE" *) reg [PTR_W-1:0] rd_ptr_gray_sync1;
(* ASYNC_REG = "TRUE" *) reg [PTR_W-1:0] rd_ptr_gray_sync2;
(* ASYNC_REG = "TRUE" *) reg [PTR_W-1:0] wr_ptr_gray_sync1;
(* ASYNC_REG = "TRUE" *) reg [PTR_W-1:0] wr_ptr_gray_sync2;

wire wr_do = i_wr_en && !o_wr_full;
wire rd_do = i_rd_en && !o_rd_empty;

wire [PTR_W-1:0] wr_bin_next  = wr_ptr_bin + {{(PTR_W-1){1'b0}}, wr_do};
wire [PTR_W-1:0] wr_gray_next = (wr_bin_next >> 1) ^ wr_bin_next; // binary to gray code conversion: G = B ^ (B >> 1)
wire [PTR_W-1:0] rd_bin_next  = rd_ptr_bin + {{(PTR_W-1){1'b0}}, rd_do};
wire [PTR_W-1:0] rd_gray_next = (rd_bin_next >> 1) ^ rd_bin_next;


wire [PTR_W-1:0] wr_full_cmp  = {~rd_ptr_gray_sync2[PTR_W-1:PTR_W-2], rd_ptr_gray_sync2[PTR_W-3:0]};//Gray-code representation of the “full-condition pointer” 
wire             wr_full_next = (wr_gray_next == wr_full_cmp);
wire             rd_empty_next = (rd_gray_next == wr_ptr_gray_sync2);

always @(posedge i_wr_clk or posedge i_wr_rst) begin
    if (i_wr_rst) begin
        wr_ptr_bin       <= {PTR_W{1'b0}};
        wr_ptr_gray      <= {PTR_W{1'b0}};
        rd_ptr_gray_sync1 <= {PTR_W{1'b0}};
        rd_ptr_gray_sync2 <= {PTR_W{1'b0}};
        o_wr_full        <= 1'b0;
    end else begin
        rd_ptr_gray_sync1 <= rd_ptr_gray;
        rd_ptr_gray_sync2 <= rd_ptr_gray_sync1;

        if (wr_do) begin
            mem[wr_ptr_bin[ADDR_W-1:0]] <= i_wr_data;
        end
        wr_ptr_bin  <= wr_bin_next;
        wr_ptr_gray <= wr_gray_next;
        o_wr_full   <= wr_full_next;
    end
end

always @(posedge i_rd_clk or posedge i_rd_rst) begin
    if (i_rd_rst) begin
        rd_ptr_bin       <= {PTR_W{1'b0}};
        rd_ptr_gray      <= {PTR_W{1'b0}};
        wr_ptr_gray_sync1 <= {PTR_W{1'b0}};
        wr_ptr_gray_sync2 <= {PTR_W{1'b0}};
        o_rd_empty       <= 1'b1;
        o_rd_valid       <= 1'b0;
        o_rd_data        <= {DATA_W{1'b0}};
    end else begin
        wr_ptr_gray_sync1 <= wr_ptr_gray;
        wr_ptr_gray_sync2 <= wr_ptr_gray_sync1;

        if (rd_do) begin
            o_rd_data <= mem[rd_ptr_bin[ADDR_W-1:0]];
        end

        rd_ptr_bin  <= rd_bin_next; // rd_ptr_bin + 1
        rd_ptr_gray <= rd_gray_next;
        o_rd_empty  <= rd_empty_next;
        o_rd_valid  <= rd_do;
    end
end

endmodule
