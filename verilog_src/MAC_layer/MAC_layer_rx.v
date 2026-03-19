// MAC RX: XGMII -> AXI Stream (frame delineation, tdata/tkeep/tvalid/tlast).
// it strips preamble/SFD, but does not check CRC, and keep crc.
// it only involves two-cycle delay. One is for preamble/SFD stripping (valid_d1), the other is for data alignment in SOF_4 case (data_saver).
`timescale 1ns / 1ps

module MAC_layer_rx #(
    parameter DATA_WIDTH    = 64,
    parameter CTRL_WIDTH    = 8
)(
    input  wire                     i_xgmii_rx_clk   ,
    input  wire                     i_xgmii_rx_rst   , // active high
    input  wire [DATA_WIDTH-1:0]    i_xgmii_rxd      ,
    input  wire [CTRL_WIDTH-1:0]    i_xgmii_rxc      ,
    input  wire                     i_rx_status      ,

    output wire [DATA_WIDTH-1:0]    o_axi_rx_data    ,
    output reg                      o_axi_rx_valid   ,
    output reg [CTRL_WIDTH-1:0]     o_axi_rx_keep    ,
    output reg                      o_axi_rx_last    ,
    output wire                     o_frame_started  ,
    output wire [47:0]              o_frame_ts       ,
    output wire [47:0]              o_dma_ts_gray    ,
    input  wire                     i_axi_rx_ready
);
    

//====  start and end detection ====
    localparam XGMII_START = 8'hFB;
    localparam XGMII_TERM  = 8'hFD;    
    wire [1:0]   sof_location;
    wire [7:0]   eof_location;
    
    genvar      idx;
    generate
        for (idx = 0; idx < 2; idx = idx + 1 ) begin:sof_check
            assign sof_location [idx] = i_xgmii_rxc[idx*4] && (i_xgmii_rxd[idx*32 +:8] == XGMII_START);
        end
    endgenerate
    generate
        for (idx = 0; idx < 8; idx = idx + 1 ) begin:eof_check
            assign eof_location [idx] = i_xgmii_rxc[idx] && (i_xgmii_rxd[idx*8 +:8] == XGMII_TERM);
        end
    endgenerate

wire frame_started = (cur_state == IDLE) && (|sof_location) && i_rx_status && i_axi_rx_ready;

assign o_frame_started = frame_started;

timestamper #(
    .COUNTER_WIDTH (48)
) timestamp_inst (
    .i_clk               (i_xgmii_rx_clk),
    .i_rst               (i_xgmii_rx_rst),
    .i_event             (frame_started),
    .o_mac_timestamp     (o_frame_ts    ),
    .o_dma_timestamp_gray(o_dma_ts_gray)
);



// ==== cur_state machine (placeholder: drive valid/keep/last) ===
    reg [63:0]  i_xgmii_rxd_1         ; // one cycle delay for data alignment in SOF_1 case
    reg [31:0]  data_saver            ; // one cycle delay for data alignment in SOF_4 case
    reg         valid_d1              ; // one cycle delay for valid signal
    localparam  IDLE        = 3'b001  ;
    localparam  SOF         = 3'b010  ;
    localparam  TERM        = 3'b100  ;
    reg [2:0]   cur_state             ;
    reg [7:0]   eof_reg               ; // only valid when cur_state=TERM, used to save eof_location.
    reg [1:0]   sof_reg               ; // only valid when cur_state=SOF_1/SOF_4, used to save sof_location.

// one stage for i_xgmii_rxd
    always @(posedge i_xgmii_rx_clk) begin
        // XGMII has no backpressure; when downstream is not ready, drop/flush current frame.
        if (i_xgmii_rx_rst || !i_rx_status || !i_axi_rx_ready) begin
            i_xgmii_rxd_1   <=  64'd0;
            data_saver      <=  32'd0;
            valid_d1         <=  1'b0;
            cur_state       <=  IDLE;
            sof_reg         <=  2'd0;
            eof_reg         <=  8'd0;
        end else begin
            data_saver      <=  i_xgmii_rxd [63:32];
            i_xgmii_rxd_1   <=  i_xgmii_rxd;
            case (cur_state)
                IDLE: begin
                    if (| sof_location) begin
                        cur_state           <=  SOF;
                        sof_reg             <=  sof_location;
                    end 
                    else begin
                        cur_state           <=  IDLE;
                        valid_d1             <=  1'b0;
                    end
                end
                SOF: begin
                    valid_d1                 <=  1'b1;
                    if (|eof_location) begin
                        cur_state           <=  TERM;
                        eof_reg             <=  eof_location; 
                    end 
                end
                TERM: begin
                    valid_d1         <=  1'b0;
                    cur_state       <=  IDLE;
                end
                default: begin
                    cur_state <= IDLE;
                    valid_d1         <=  1'b0;
                end
            endcase
        end
    end

    //== data mux ==
    reg [63:0] data_before_rev;
    always @* 
    case (sof_reg)
        2'd0:  data_before_rev = 64'd0;
        2'b01: data_before_rev = i_xgmii_rxd_1;
        2'b10: data_before_rev = {i_xgmii_rxd[31:0],data_saver};
        default:data_before_rev = 64'd0;
    endcase
    // == valid mux ==
    always @* 
    case (cur_state)
        IDLE:   o_axi_rx_valid = 1'b0;
        SOF:    o_axi_rx_valid = valid_d1; // ditch preamble/SFD word
        TERM:   case (sof_reg)
                    2'b01: 
                        if(| (eof_reg >> 1))   o_axi_rx_valid = 1'b1;
                        else o_axi_rx_valid = 1'b0;
                    2'b10: 
                        if(| (eof_reg >> 5))   o_axi_rx_valid = 1'b1; 
                        else o_axi_rx_valid = 1'b0;
                    default:o_axi_rx_valid = 1'b0; 
                endcase
        default:o_axi_rx_valid = 1'b0;
    endcase

    // == last mux ==
    always @* 
    case (cur_state)
        IDLE:   o_axi_rx_last = 1'b0;
        SOF:    case (sof_reg)
            2'b01: 
                if(| (eof_location << 7))   o_axi_rx_last = 1'b1; 
                else o_axi_rx_last = 1'b0;
            2'b10: 
                if(| (eof_location << 3))   o_axi_rx_last = 1'b1; 
                else o_axi_rx_last = 1'b0;
            default:o_axi_rx_last = 1'b0; 
            endcase  
        TERM:   case (sof_reg)
                2'b01: 
                    if(| (eof_reg >> 1))   o_axi_rx_last = 1'b1; 
                    else o_axi_rx_last = 1'b0;
                2'b10: 
                    if(| (eof_reg >> 5))   o_axi_rx_last = 1'b1; 
                    else o_axi_rx_last = 1'b0;
                default:o_axi_rx_last = 1'b0; 
                endcase
        default:o_axi_rx_last = 1'b0;
    endcase

    // == keep mux ==
    always @*
    case (cur_state)
        IDLE:   o_axi_rx_keep = 8'b0000_0000;
        SOF:    
                case (sof_reg)
                2'b01: o_axi_rx_keep = 8'b1111_1111;
                2'b10: 
                    case(eof_location)
                        8'b0000_0001: o_axi_rx_keep = 8'b1111_0000; //
                        8'b0000_0010: o_axi_rx_keep = 8'b1111_1000; //
                        8'b0000_0100: o_axi_rx_keep = 8'b1111_1100; //
                        8'b0000_1000: o_axi_rx_keep = 8'b1111_1110; //
                        8'b0001_0000: o_axi_rx_keep = 8'b1111_1111; //
                        default:      o_axi_rx_keep = 8'b1111_1111;
                    endcase
                endcase
        TERM:   case (sof_reg)
                    2'b01:
                        case(eof_reg)
                            8'b0000_0010: o_axi_rx_keep = 8'b1000_0000; 
                            8'b0000_0100: o_axi_rx_keep = 8'b1100_0000; 
                            8'b0000_1000: o_axi_rx_keep = 8'b1110_0000; 
                            8'b0001_0000: o_axi_rx_keep = 8'b1111_0000; 
                            8'b0010_0000: o_axi_rx_keep = 8'b1111_1000; 
                            8'b0100_0000: o_axi_rx_keep = 8'b1111_1100; 
                            8'b1000_0000: o_axi_rx_keep = 8'b1111_1110; 
                            default:      o_axi_rx_keep = 8'b0000_0000;
                        endcase
                    2'b10: 
                        case(eof_reg)
                            8'b0010_0000: o_axi_rx_keep = 8'b1000_0000; //
                            8'b0100_0000: o_axi_rx_keep = 8'b1100_0000; //
                            8'b1000_0000: o_axi_rx_keep = 8'b1110_0000; //
                            default:      o_axi_rx_keep = 8'b0000_0000;
                        endcase
                    default:o_axi_rx_keep = 8'b0000_0000; 
                endcase
        default:o_axi_rx_keep = 8'b0000_0000;
    endcase    

    // Byte reverse: first byte of frame at [63:56] (big-endian)
    generate
        for (idx = 0; idx < 8; idx = idx + 1) begin : reverse_data
            assign o_axi_rx_data[idx*8 +: 8] = data_before_rev[56 - idx*8 +: 8];
        end
    endgenerate
endmodule
