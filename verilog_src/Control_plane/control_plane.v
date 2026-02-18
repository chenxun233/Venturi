module control_plane #(
    parameter DATA_WIDTH    = 64,
    parameter BAR0_SIZE     = 16
)(
// pcie_wrapper interface
input wire                          i_user_clk_250,
input wire                          i_user_reset_p, //active high
input wire                          i_cq_valid,
input wire [3:0]                    i_cq_type,
input wire [BAR0_SIZE-1:0]          i_cq_reg_addr,
input wire [63:0]                   i_cq_payload,
input wire [2:0]                    i_cq_bar_id,
input wire [15:0]                   i_cq_requester_id,
input wire [7:0]                    i_cq_tag,
input wire [2:0]                    i_cq_tc,
input wire [6:0]                    i_cq_lower_addr,
input wire [10:0]                   i_cq_payload_dw_count,
input wire                          i_cq_last,
input wire                          i_cc_ready,
output  wire                        o_cc_valid,
output  wire [15:0]                 o_cc_requester_id,
output  wire [7:0]                  o_cc_tag,
output  wire [2:0]                  o_cc_tc,
output  wire [6:0]                  o_cc_lower_addr,
output  wire [10:0]                 o_cc_dword_count,
output  wire [2:0]                  o_cc_status,
output  wire [DATA_WIDTH/2-1:0]     o_cc_payload,
output  wire                        o_cc_last,
// ethernet_controller interface
input   wire                        i_clk_156   ,
output  wire                        o_sync_fire ,
output  reg [47:0]                  o_ctl_mac_addr  ,
output  reg [31:0]                  o_ctl_ip_addr   ,
output  reg [15:0]                  o_ctl_port
);

reg         [0:0]           async_fire    ;

localparam  [BAR0_SIZE-1:0] REG_ETH_FIRE    = 16'h00; // address of mac_addr register
localparam  [BAR0_SIZE-1:0] REG_MAC         = 16'h04; // update Ethernet settings (e.g., MAC/IP address) when write to this register
localparam  [BAR0_SIZE-1:0] REG_IP          = 16'h08; // update IP address when write to this register
localparam  [BAR0_SIZE-1:0] REG_PORT        = 16'h0C; // update port when write to this register


// === cq===
always @(posedge i_user_clk_250) begin
    if (i_user_reset_p) begin
        o_ctl_mac_addr  <= 48'd0;
        o_ctl_ip_addr   <= 32'd0;
        o_ctl_port      <= 16'd0;
        async_fire  <= 1'b0;
    end else begin
        if (i_cq_valid && i_cq_type == 4'b0001) begin // process write requests
            case (i_cq_reg_addr)
                REG_ETH_FIRE: begin
                    async_fire <= 1'b1;
                end
                REG_MAC: begin
                    o_ctl_mac_addr <= i_cq_payload[47:0];
                end
                REG_IP: begin
                    o_ctl_ip_addr <= i_cq_payload[31:0];
                end
                REG_PORT: begin
                    o_ctl_port    <= i_cq_payload[15:0];
                end
                default: begin
                    // do nothing for other addresses
                end
            endcase
        end
    end
    if (async_fire) begin
        async_fire <= 1'b0; // reset the fire signal after one cycle
    end
end
bit_synchronizer #(
)
rst_sync_inst
(
  .i_clk(i_clk_156),
  .i_in(async_fire),
  .o_out(o_sync_fire)
);

endmodule