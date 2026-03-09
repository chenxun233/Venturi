module fifo #(
    parameter DEPTH = 8,              // must be power-of-2 (2,4,8,16,32...)
    parameter DATA_W = 393
)(
    input  wire                 i_clk,
    input  wire                 i_rst,        // active high, synchronous

    // push side
    input  wire                 i_do_push,
    output wire                 o_push_ready,
    input  wire [DATA_W-1:0]    i_data,

    // pop side
    input  wire                 i_do_pop,
    output wire                 o_not_empty,
    output reg                  o_valid, // indicating the pop data is valid (not garbage due to empty)
    output reg [DATA_W-1:0]     o_data
);

    // ----------------------------
    // derived parameters
    // ----------------------------
    function integer clog2;
        input integer value;
        integer i;
        begin
            clog2 = 0;
            for (i = value-1; i > 0; i = i >> 1)
                clog2 = clog2 + 1;
        end
    endfunction

    localparam ADDR  = clog2(DEPTH);
    localparam CNT_W = clog2(DEPTH + 1);

    // ----------------------------
    // storage
    // ----------------------------
    reg [DATA_W-1:0] mem [0:DEPTH-1];
    reg [ADDR-1:0]        wr_ptr;
    reg [ADDR-1:0]        rd_ptr;
    reg [CNT_W-1:0]       count;

    wire full  = (count == DEPTH[CNT_W-1:0]);

    assign o_push_ready = ~full;
    assign o_not_empty = ~(count == {CNT_W{1'b0}});

    // combinational read (fine for small DEPTH; infers LUTRAM typically)

    wire do_push = i_do_push & o_push_ready;
    

    always @(posedge i_clk) begin
        if (i_rst) begin
            wr_ptr      <= {ADDR{1'b0}};
            rd_ptr      <= {ADDR{1'b0}};
            count       <= {CNT_W{1'b0}};
            o_valid <= 1'b0;
        end else begin
            // write
            if (do_push) begin
                mem[wr_ptr] <= i_data;
                wr_ptr <= wr_ptr + 1;
            end
            // advance read pointer
            if (i_do_pop) begin
                o_data  <= mem[rd_ptr];
                rd_ptr      <= rd_ptr + 1;
                o_valid <= 1'b1;
            end else begin
                o_data  <= o_data;
                o_valid <= 1'b0;
            end
            // count
            case ({do_push, i_do_pop})
                2'b10: count    <= count + 1;
                2'b01: count    <= count - 1;
                default: count  <= count;
            endcase
        end
    end

endmodule
