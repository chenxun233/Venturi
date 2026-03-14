module RQ_gearbox256 #(
    parameter DATA_WIDTH = 256
)(
    input  wire                   clk,
    input  wire                   rst_n,

    // =========================================================================
    // User Interface
    // =========================================================================
    input  wire [127:0]              rq_descriptor,
    input  wire [255:0]              rq_payload,
    input  wire [10:0]               rq_payload_dw_count, 
    input  wire                      rq_payload_last,
    input  wire                      rq_valid,
    output wire                      rq_ready, 
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


    localparam TYPE_READ  = 4'b0000;
    localparam TYPE_WRITE = 4'b0001; // for example, you can define

    reg [127:0] payload_prev;
    reg [10:0]  remain_dw;
    wire [3:0]  type  = rq_descriptor[78:75]; // 
    wire [127:0] payload_cur = rq_payload[127:0]; // Current payload beat

    reg [2:0] state;
    localparam IDLE     = 3'b000;
    localparam SENDING  = 3'b001;
    localparam TAIL     = 3'b010;

    
    function [7:0] keep_mask(input [3:0] count);
        begin
            case (count)
                4'd0: keep_mask = 8'h00;
                4'd1: keep_mask = 8'h01;
                4'd2: keep_mask = 8'h03;
                4'd3: keep_mask = 8'h07;
                4'd4: keep_mask = 8'h0F;
                4'd5: keep_mask = 8'h1F;
                4'd6: keep_mask = 8'h3F;
                4'd7: keep_mask = 8'h7F;
                default: keep_mask = 8'hFF;
            endcase
        end
    endfunction

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            s_axis_rq_tdata   <= 256'b0;
            s_axis_rq_tvalid  <= 1'b0;
            s_axis_rq_tlast   <= 1'b0;
            s_axis_rq_tkeep   <= 8'b0;
            s_axis_rq_tuser   <= 60'b0;
            payload_prev      <= 128'b0;
            remain_dw         <= 11'b0;
            state             <= IDLE;
        end else begin
            s_axis_rq_tdata   <= 256'b0;
            s_axis_rq_tvalid  <= 1'b0;
            s_axis_rq_tlast   <= 1'b0;
            s_axis_rq_tkeep   <= 8'b0;
            s_axis_rq_tuser   <= 60'b0;

            if (s_axis_rq_tready) begin
                case (state)
                    IDLE: begin
                        if (rq_valid) begin
                            case (type)
                                TYPE_READ: begin
                                    s_axis_rq_tdata   <= {128'b0, rq_descriptor};
                                    s_axis_rq_tvalid  <= 1'b1;
                                    s_axis_rq_tlast   <= 1'b1;
                                    s_axis_rq_tkeep   <= 8'h0F;
                                    s_axis_rq_tuser   <= {52'b0, rq_descriptor[107:104], rq_descriptor[111:108]};
                                end
                                TYPE_WRITE: begin
                                    payload_prev      <= rq_payload[255:128];
                                    s_axis_rq_tdata   <= {payload_cur, rq_descriptor};
                                    s_axis_rq_tvalid  <= 1'b1;
                                    s_axis_rq_tuser   <= {52'b0, rq_descriptor[107:104], rq_descriptor[111:108]};
                                    if (rq_payload_dw_count <= 11'd4) begin
                                        s_axis_rq_tlast <= 1'b1;
                                        s_axis_rq_tkeep <= keep_mask(4'd4 + rq_payload_dw_count[3:0]);
                                        remain_dw       <= 11'd0;
                                    end else begin
                                        s_axis_rq_tlast <= 1'b0;
                                        s_axis_rq_tkeep <= 8'hFF;
                                        remain_dw       <= rq_payload_dw_count - 11'd4;
                                        if (rq_payload_last)
                                            state <= TAIL;
                                        else
                                            state <= SENDING;
                                    end
                                end
                                default: begin
                                    remain_dw <= 11'd0;
                                end
                            endcase
                        end
                    end
                    SENDING: begin
                        if (rq_valid) begin
                            s_axis_rq_tdata  <= {payload_cur, payload_prev};
                            s_axis_rq_tvalid <= 1'b1;
                            s_axis_rq_tuser  <= 60'b0;
                            payload_prev     <= rq_payload[255:128];

                            if (remain_dw <= 11'd8) begin
                                s_axis_rq_tlast <= 1'b1;
                                s_axis_rq_tkeep <= keep_mask(remain_dw[3:0]);
                                remain_dw       <= 11'd0;
                                state           <= IDLE;
                            end else begin
                                s_axis_rq_tlast <= 1'b0;
                                s_axis_rq_tkeep <= 8'hFF;
                                remain_dw       <= remain_dw - 11'd8;
                                if (rq_payload_last)
                                    state <= TAIL;
                            end
                        end
                    end
                    TAIL: begin
                        s_axis_rq_tdata   <= {128'b0, payload_prev};
                        s_axis_rq_tvalid  <= 1'b1;
                        s_axis_rq_tlast   <= 1'b1;
                        s_axis_rq_tkeep   <= keep_mask(remain_dw[3:0]);
                        s_axis_rq_tuser   <= 60'b0;
                        remain_dw         <= 11'd0;
                        state             <= IDLE;
                    end
                    default: begin
                        remain_dw         <= 11'd0;
                        state             <= IDLE;
                    end
                endcase
            end
        end
    end 
    assign rq_ready = (state != TAIL) && s_axis_rq_tready;


endmodule
