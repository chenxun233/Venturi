module RQ_gearbox256 #(
    parameter DATA_WIDTH = 256
)(
    input  wire                   clk,
    input  wire                   rst_n,

    // =========================================================================
    // User Interface
    // =========================================================================
    input  wire [127:0]           descriptor,
    input  wire [255:0]           rq_wr_data,
    input  wire [10:0]            rq_dword_count,
    input  wire                   rq_last,
    input  wire                   rq_valid,
    input  wire                   rq_sop,
    output wire                   rq_ready, 
    // =========================================================================
    // PCIe IP Core Interface
    // =========================================================================
    output reg  [DATA_WIDTH-1:0]      s_axis_rq_tdata,
    output reg                        s_axis_rq_tvalid,
    output reg  [59:0]                s_axis_rq_tuser,
    output reg  [7:0]                 s_axis_rq_tkeep,
    output reg                        s_axis_rq_tlast,
    input  wire                       s_axis_rq_tready
);

    reg [127:0] data_saver;
    reg [0:0]   one_more_cycle;
    
    // Helper for Tail Keep (Your logic was mostly correct, just cleaned up)
    function [7:0] calc_tail_keep(input [10:0] count);
        case (count & 11'd7) // all data
            11'd1: calc_tail_keep = 8'b0001_1111;
            11'd2: calc_tail_keep = 8'b0011_1111;
            11'd3: calc_tail_keep = 8'b0111_1111;
            11'd4: calc_tail_keep = 8'b1111_1111; // above can be sent in Tn
            11'd5: calc_tail_keep = 8'b0000_0001; // Below has to be sent in Tn+1
            11'd6: calc_tail_keep = 8'b0000_0011;
            11'd7: calc_tail_keep = 8'b0000_0111;
            11'd0: calc_tail_keep = 8'b0000_1111;
            default: calc_tail_keep = 8'hFF; // should not happen
        endcase
    endfunction

    // this function checks if one more cycle is needed after last beat
    function [0:0] one_more (input [10:0] count);
        if ((count & 11'd7) <= 11'd4)
            one_more = 1'b0;
        else
            one_more = 1'b1;
    endfunction

    // =========================================================================
    // Main Logic
    // =========================================================================
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            s_axis_rq_tdata   <= 256'b0;
            s_axis_rq_tvalid  <= 1'b0;
            s_axis_rq_tlast   <= 1'b0;
            s_axis_rq_tkeep   <= 8'b0;
            s_axis_rq_tuser   <= 60'b0;
            data_saver        <= 128'b0;
            one_more_cycle    <= 1'b0;
        end 
        else if (s_axis_rq_tready) begin
            // -------------------------------------------------------------
            // Priority 2: NORMAL PROCESSING
            // -------------------------------------------------------------
            if (rq_valid) begin
                one_more_cycle <= one_more(rq_dword_count);
                // Case A: Small Packet (<= 4 DW) - No Remnant created
                if (rq_sop && !one_more(rq_dword_count) && rq_dword_count <= 11'd4) begin
                    s_axis_rq_tdata   <= {rq_wr_data[127:0], descriptor};
                    s_axis_rq_tvalid  <= 1'b1;
                    s_axis_rq_tlast   <= 1'b1;
                    // Calculate keep for small packet (Header(4) + Payload)
                    // If count=1(Total5)->0x1F. If count=2(Total6)->0x3F.
                    s_axis_rq_tkeep   <= calc_tail_keep(rq_dword_count);
                    s_axis_rq_tuser   <= {52'b0, descriptor[107:104], descriptor[111:108]};
                end
                
                // Case B: Large Packet 1. The SOP
                else if (rq_sop) begin
                    s_axis_rq_tdata   <= {rq_wr_data[127:0], descriptor};
                    data_saver        <= rq_wr_data[255:128]; // Save Upper
                    s_axis_rq_tuser   <= {52'b0, descriptor[107:104], descriptor[111:108]};
                    s_axis_rq_tvalid  <= 1'b1;
                    s_axis_rq_tlast   <= 1'b0;
                    s_axis_rq_tkeep   <= 8'hFF;
                end
                
                // Case B: Large Packet 2. The body
                else if (!rq_last) begin
                    s_axis_rq_tdata   <= {rq_wr_data[127:0], data_saver};
                    data_saver        <= rq_wr_data[255:128]; // Save New Upper
                    s_axis_rq_tuser   <= 60'b0;
                    s_axis_rq_tlast   <= 1'b0;
                    s_axis_rq_tvalid  <= 1'b1;
                    s_axis_rq_tkeep   <= 8'hFF;
                end
                // Case B: Large Packet 3. End, but no one more cycle needed
                else if (rq_last && ! one_more(rq_dword_count))begin
                    s_axis_rq_tdata   <= {rq_wr_data[127:0], data_saver};
                    data_saver        <= 0; //  not needed anymore
                    s_axis_rq_tuser   <= 60'b0;
                    s_axis_rq_tlast   <= 1'b1;
                    s_axis_rq_tvalid  <= 1'b1;
                    s_axis_rq_tkeep   <= calc_tail_keep(rq_dword_count);
                end
                // Case B: Large Packet 3. End, one more cycle needed
                else if (rq_last && one_more(rq_dword_count))begin
                    s_axis_rq_tdata   <= {rq_wr_data[127:0], data_saver};
                    data_saver        <= rq_wr_data[255:128]; // Save New Upper
                    s_axis_rq_tuser   <= 60'b0;
                    s_axis_rq_tlast   <= 1'b0; //hold, the last is in the one more cycle.
                    s_axis_rq_tvalid  <= 1'b1;
                    s_axis_rq_tkeep   <= 8'hFF;
                end
            end
            // even if rq_valid is low, we may have one more cycle to send
            else if (one_more_cycle) begin
                // Final Cycle for Large Packet
                s_axis_rq_tdata   <= {data_saver, 128'b0};
                s_axis_rq_tvalid  <= 1'b1;  // still valid to the PCIe core
                s_axis_rq_tlast   <= 1'b1; // last for one more cycle
                s_axis_rq_tkeep   <= calc_tail_keep(rq_dword_count);
                s_axis_rq_tuser   <= 60'b0;
                one_more_cycle    <= 1'b0;
            end
            // -------------------------------------------------------------
            // Priority 3: IDLE
            // -------------------------------------------------------------
            else begin
                s_axis_rq_tdata   <= 256'b0;
                s_axis_rq_tvalid  <= 1'b0;
                s_axis_rq_tlast   <= 1'b0;
                s_axis_rq_tkeep   <= 8'b0;
                s_axis_rq_tuser   <= 60'b0;
                data_saver        <= 128'b0;
                one_more_cycle    <= 1'b0;

            end
        end
    end
    assign rq_ready = s_axis_rq_tready && !one_more_cycle;


endmodule