// =============================================================================
// pcie_wrapper.v - PCIe Interface Layer for HFT NIC
// =============================================================================
// Top-level wrapper connecting Xilinx UltraScale PCIe Gen3 IP core to user logic
// via CQ_parser, CC_formatter, RQ_formatter, and RC_parser modules.
//
// Channel Summary:
//   CQ (Completer Request)   - Host → FPGA MMIO requests (reads/writes)
//   CC (Completer Completion) - FPGA → Host read data responses
//   RQ (Requester Request)    - FPGA → Host DMA requests (reads/writes)
//   RC (Requester Completion) - Host → FPGA DMA read completions
// =============================================================================

module pcie_wrapper #(
    parameter DATA_WIDTH = 256,
    parameter BAR0_SIZE  = 16                // 2^16 = 64 KB BAR
)(
    // =========================================================================
    // Top Interface
    // =========================================================================
    output wire [7:0]                   o_pci_exp_txn,
    output wire [7:0]                   o_pci_exp_txp,
    input  wire [7:0]                   i_pci_exp_rxn,
    input  wire [7:0]                   i_pci_exp_rxp,
    input  wire                         pcie_clk_50,
    input  wire                         pcie_clk_100,
    input  wire                         i_pcie_rst_n,
    // =========================================================================
    // Logic Interface
    // =========================================================================
    output wire                         o_user_clk_250,
    output wire                         o_user_reset_p, //active high
    // CQ Parser Outputs (Host → FPGA MMIO Requests)
    output wire                         o_cq_valid,
    output wire [3:0]                   o_cq_type,
    output wire [BAR0_SIZE-1:0]         o_cq_reg_addr,
    output wire [63:0]                  o_cq_payload,
    output wire [2:0]                   o_cq_bar_id,
    output wire [15:0]                  o_cq_requester_id,
    output wire [7:0]                   o_cq_tag,
    output wire [2:0]                   o_cq_tc,
    output wire [6:0]                   o_cq_lower_addr,
    output wire [10:0]                  o_cq_payload_dw_count,
    output wire                         o_cq_last,
    // CC Formatter Inputs (FPGA → Host Read Responses)s
    output wire                         o_cc_ready,
    input  wire                         i_cc_valid,
    input  wire [15:0]                  i_cc_requester_id,
    input  wire [7:0]                   i_cc_tag,
    input  wire [2:0]                   i_cc_tc,
    input  wire [6:0]                   i_cc_lower_addr,
    input  wire [10:0]                  i_cc_dword_count,
    input  wire [2:0]                   i_cc_status,
    input  wire [DATA_WIDTH/2-1:0]      i_cc_payload,
    input  wire                         i_cc_last,
    // RQ Formatter Inputs (FPGA → Host DMA Requests)
    output wire                         o_rq_ready,
    input  wire                         i_rq_valid,
    input  wire [3:0]                   i_rq_type,
    input  wire                         i_rq_payload_sop,
    input  wire                         i_rq_payload_last,        // the last cycle of data sending
    input  wire [63:0]                  i_rq_addr,
    input  wire [10:0]                  i_rq_payload_dw_count, // Total Data DWords (1-1024) in this burst. Does not include header.
    input  wire [7:0]                   i_rq_tag,
    input  wire [2:0]                   i_rq_tc,
    input  wire [255:0]                 i_rq_payload,
    // RC Parser Outputs (Host → FPGA DMA Read Completions) - Realigned by gearbox
    output wire  [11:0]                 o_rc_lower_addr,
    output wire  [3:0]                  o_rc_err_code,
    output wire  [12:0]                 o_rc_payload_byte_count,
    output wire                         o_rc_request_completed,
    output wire  [15:0]                 o_rc_requester_id,
    output wire  [7:0]                  o_rc_tag,
    output wire                         o_rc_valid,               // valid for all
    output wire                         o_rc_payload_last,         // End of packet
    output wire [255:0]                 o_rc_payload,             // Realigned 256-bit payload
    output wire [DATA_WIDTH / 32-1:0]   o_rc_payload_dw_keep,         // Byte enables
    output wire                         o_rc_posioned,
    output wire [12:0]                  o_rc_payload_dw_count,
    // Configuration Outputs
    output wire [2:0]                   o_cfg_max_payload,
    output wire [2:0]                   o_cfg_max_read_req,
    output wire [15:0]                  o_pcie_requester_id
);

    // =========================================================================
    // Internal Wires - CQ Channel (from PCIe IP to CQ_parser)
    // =========================================================================
    wire [DATA_WIDTH-1:0]           m_axis_cq_tdata;
    wire                            m_axis_cq_tvalid;
    wire [84:0]                     m_axis_cq_tuser;
    wire [DATA_WIDTH / 32-1:0]      m_axis_cq_tkeep;
    wire                            m_axis_cq_tlast;
    wire                            m_axis_cq_tready;

    // =========================================================================
    // Internal Wires - CC Channel (from CC_formatter to PCIe IP)
    // =========================================================================
    wire [DATA_WIDTH-1:0]           s_axis_cc_tdata;
    wire                            s_axis_cc_tvalid;
    wire [32:0]                     s_axis_cc_tuser;
    wire [DATA_WIDTH / 32-1:0]      s_axis_cc_tkeep;
    wire                            s_axis_cc_tlast;
    wire [3:0]                      s_axis_cc_tready;

    // =========================================================================
    // Internal Wires - RQ Channel (from RQ_formatter to PCIe IP)
    // =========================================================================
    wire [DATA_WIDTH-1:0]           s_axis_rq_tdata;
    wire                            s_axis_rq_tvalid;
    wire [59:0]                     s_axis_rq_tuser;
    wire [DATA_WIDTH / 32-1:0]      s_axis_rq_tkeep;
    wire                            s_axis_rq_tlast;
    wire [3:0]                      s_axis_rq_tready;

    // =========================================================================
    // Internal Wires - RC Channel (from PCIe IP to RC_parser)
    // =========================================================================
    wire [DATA_WIDTH-1:0]           m_axis_rc_tdata;
    wire                            m_axis_rc_tvalid;
    wire [74:0]                     m_axis_rc_tuser;
    wire [DATA_WIDTH / 32-1:0]      m_axis_rc_tkeep;
    wire                            m_axis_rc_tlast;
    wire                            m_axis_rc_tready;

    // =========================================================================
    // Internal Wires - Configuration Management
    // =========================================================================
    wire [18:0]                     cfg_mgmt_addr;
    wire                            cfg_mgmt_write;
    wire [31:0]                     cfg_mgmt_write_data;
    wire [3:0]                      cfg_mgmt_byte_enable;
    wire                            cfg_mgmt_read;
    wire [31:0]                     cfg_mgmt_read_data;
    wire                            cfg_mgmt_read_write_done;

    // =========================================================================
    // Internal Wires - Flow Control
    // =========================================================================
    wire [7:0]                      cfg_fc_ph;
    wire [11:0]                     cfg_fc_pd;
    wire [7:0]                      cfg_fc_nph;
    wire [11:0]                     cfg_fc_npd;
    wire [7:0]                      cfg_fc_cplh;
    wire [11:0]                     cfg_fc_cpld;
    wire [2:0]                      cfg_fc_sel;

    // =========================================================================
    // Internal Wires - Status and Error
    // =========================================================================
    wire                            cfg_rcb_status;
    wire                            status_error_cor;
    wire                            status_error_uncor;

    // =========================================================================
    // Internal Wires - Interrupt
    // =========================================================================
    wire [3:0]                      cfg_interrupt_msix_enable;
    wire [3:0]                      cfg_interrupt_msix_mask;
    wire [7:0]                      cfg_interrupt_msix_vf_enable;
    wire [7:0]                      cfg_interrupt_msix_vf_mask;
    wire [63:0]                     cfg_interrupt_msix_address;
    wire [31:0]                     cfg_interrupt_msix_data;
    wire                            cfg_interrupt_msix_int;
    wire                            cfg_interrupt_msix_sent;
    wire                            cfg_interrupt_msix_fail;
    wire [3:0]                      cfg_interrupt_msi_function_number;
    wire [15:0]                     cfg_function_status; // Bus[15:8], Dev[7:3], Func[2:0]

    // =========================================================================
    // Internal Wires - RQ Sequence
    // =========================================================================
    wire [1:0]                      pcie_tfc_nph_av;
    wire [1:0]                      pcie_tfc_npd_av;

    // =========================================================================
    // Tie-off unused configuration management signals
    // =========================================================================
    assign cfg_mgmt_addr        = 19'h0;
    assign cfg_mgmt_write       = 1'b0;
    assign cfg_mgmt_write_data  = 32'h0;
    assign cfg_mgmt_byte_enable = 4'h0;
    assign cfg_mgmt_read        = 1'b0;
    assign cfg_fc_sel           = 3'h0;
    assign status_error_cor     = 1'b0;
    assign status_error_uncor   = 1'b0;

    // =========================================================================
    // Tie-off unused interrupt signals
    // =========================================================================
    assign cfg_interrupt_msix_address         = 64'h0;
    assign cfg_interrupt_msix_data            = 32'h0;
    assign cfg_interrupt_msix_int             = 1'b0;
    assign cfg_interrupt_msi_function_number  = 4'h0;

    // =========================================================================
    // CQ_parser Instantiation
    // =========================================================================
    CQ_parser #(
        .DATA_WIDTH                 (DATA_WIDTH),
        .BAR0_SIZE                  (BAR0_SIZE)
    ) cq_parser_inst (
        .m_axis_cq_tdata            (m_axis_cq_tdata),
        .m_axis_cq_tvalid           (m_axis_cq_tvalid),
        .m_axis_cq_tuser            (m_axis_cq_tuser),
        .m_axis_cq_tkeep            (m_axis_cq_tkeep),
        .m_axis_cq_tlast            (m_axis_cq_tlast),
        .m_axis_cq_tready           (m_axis_cq_tready),
        .cq_valid                   (o_cq_valid),
        .cq_type                    (o_cq_type),
        .cq_reg_addr                (o_cq_reg_addr),
        .cq_payload                 (o_cq_payload),
        .cq_bar_id                  (o_cq_bar_id),
        .cq_requester_id            (o_cq_requester_id),
        .cq_tag                     (o_cq_tag),
        .cq_tc                      (o_cq_tc),
        .cq_lower_addr              (o_cq_lower_addr),
        .cq_payload_dw_count        (o_cq_payload_dw_count),
        .cq_last                    (o_cq_last)
    );

    // =========================================================================
    // CC_formatter Instantiation
    // =========================================================================
    CC_formatter #(
        .DATA_WIDTH                 (DATA_WIDTH)
    ) cc_formatter_inst (
        .cc_ready                   (o_cc_ready),
        .cc_valid                   (i_cc_valid),
        .cc_requester_id            (i_cc_requester_id),
        .cc_tag                     (i_cc_tag),
        .cc_tc                      (i_cc_tc),
        .cc_lower_addr              (i_cc_lower_addr),
        .cc_dword_count             (i_cc_dword_count),
        .cc_status                  (i_cc_status),
        .cc_payload                 (i_cc_payload),
        .cc_last                    (i_cc_last),
        .s_axis_cc_tdata            (s_axis_cc_tdata),
        .s_axis_cc_tvalid           (s_axis_cc_tvalid),
        .s_axis_cc_tuser            (s_axis_cc_tuser),
        .s_axis_cc_tkeep            (s_axis_cc_tkeep),
        .s_axis_cc_tlast            (s_axis_cc_tlast),
        .s_axis_cc_tready           (s_axis_cc_tready)
    );

    // =========================================================================
    // RQ_formatter Instantiation
    // =========================================================================
    RQ_formatter #(
        .DATA_WIDTH                 (DATA_WIDTH)
    ) rq_formatter_inst (
        .clk                        (o_user_clk_250),
        .rst_n                      (~o_user_reset_p),
        .rq_valid                   (i_rq_valid),
        .rq_ready                   (o_rq_ready),
        .rq_type                    (i_rq_type),
        .rq_payload_sop             (i_rq_payload_sop),
        .rq_payload_last            (i_rq_payload_last),
        .rq_addr                    (i_rq_addr),
        .rq_payload_dw_count        (i_rq_payload_dw_count),
        .rq_tag                     (i_rq_tag),
        .rq_tc                      (i_rq_tc),
        .rq_payload                 (i_rq_payload),
        .s_axis_rq_tdata            (s_axis_rq_tdata),
        .s_axis_rq_tvalid           (s_axis_rq_tvalid),
        .s_axis_rq_tuser            (s_axis_rq_tuser),
        .s_axis_rq_tkeep            (s_axis_rq_tkeep),
        .s_axis_rq_tlast            (s_axis_rq_tlast),
        .s_axis_rq_tready           (s_axis_rq_tready)
    );

    // =========================================================================
    // RC_parser Instantiation (includes RC_gearbox256 for data realignment)
    // =========================================================================



    RC_parser #(
        .DATA_WIDTH                 (DATA_WIDTH)
    ) rc_parser_inst (
        .clk                        (o_user_clk_250),
        .rst_n                      (~o_user_reset_p),
        .m_axis_rc_tdata            (m_axis_rc_tdata),
        .m_axis_rc_tvalid           (m_axis_rc_tvalid),
        .m_axis_rc_tuser            (m_axis_rc_tuser),
        .m_axis_rc_tkeep            (m_axis_rc_tkeep),
        .m_axis_rc_tlast            (m_axis_rc_tlast),
        .m_axis_rc_tready           (m_axis_rc_tready),
        .rc_lower_addr              (o_rc_lower_addr),
        .rc_err_code                (o_rc_err_code),
        .rc_payload_byte_count      (o_rc_payload_byte_count),
        .rc_request_completed       (o_rc_request_completed),
        .rc_requester_id            (o_rc_requester_id),
        .rc_completer_id            (),
        .rc_tag                     (o_rc_tag),
        .rc_valid                   (o_rc_valid),
        .rc_payload_last            (o_rc_payload_last),
        .rc_payload                 (o_rc_payload),
        .rc_payload_dw_keep         (o_rc_payload_dw_keep),
        .rc_posioned                (o_rc_posioned),
        .rc_payload_dw_count        (o_rc_payload_dw_count)
    );

    // =========================================================================
    // PCIe IP Core Instantiation
    // =========================================================================
    pcie3_ultrascale_0 pcie3_ultrascale_inst (
        // PCIe Physical Pins
        .pci_exp_txn                (o_pci_exp_txn),
        .pci_exp_txp                (o_pci_exp_txp),
        .pci_exp_rxn                (i_pci_exp_rxn),
        .pci_exp_rxp                (i_pci_exp_rxp),

        // System Interface
        .sys_clk                   (pcie_clk_50),
        .sys_clk_gt                (pcie_clk_100),
        .sys_reset                   (i_pcie_rst_n),
        .pcie_perstn1_in            (1'b0),
        .pcie_perstn0_out           (),
        .pcie_perstn1_out           (),

        // User Clock and Reset
        .user_clk                   (o_user_clk_250),
        .user_reset                 (o_user_reset_p), //active high

        // CQ Channel (Host → FPGA MMIO Requests)
        .m_axis_cq_tdata            (m_axis_cq_tdata),
        .m_axis_cq_tvalid           (m_axis_cq_tvalid),
        .m_axis_cq_tuser            (m_axis_cq_tuser),
        .m_axis_cq_tkeep            (m_axis_cq_tkeep),
        .m_axis_cq_tlast            (m_axis_cq_tlast),
        .m_axis_cq_tready           (m_axis_cq_tready),

        // CC Channel (FPGA → Host Read Responses)
        .s_axis_cc_tdata            (s_axis_cc_tdata),
        .s_axis_cc_tvalid           (s_axis_cc_tvalid),
        .s_axis_cc_tuser            (s_axis_cc_tuser),
        .s_axis_cc_tkeep            (s_axis_cc_tkeep),
        .s_axis_cc_tlast            (s_axis_cc_tlast),
        .s_axis_cc_tready           (s_axis_cc_tready),

        // RQ Channel (FPGA → Host DMA Requests)
        .s_axis_rq_tdata            (s_axis_rq_tdata),
        .s_axis_rq_tvalid           (s_axis_rq_tvalid),
        .s_axis_rq_tuser            (s_axis_rq_tuser),
        .s_axis_rq_tkeep            (s_axis_rq_tkeep),
        .s_axis_rq_tlast            (s_axis_rq_tlast),
        .s_axis_rq_tready           (s_axis_rq_tready),

        // RC Channel (Host → FPGA DMA Read Completions)
        .m_axis_rc_tdata            (m_axis_rc_tdata),
        .m_axis_rc_tvalid           (m_axis_rc_tvalid),
        .m_axis_rc_tuser            (m_axis_rc_tuser),
        .m_axis_rc_tkeep            (m_axis_rc_tkeep),
        .m_axis_rc_tlast            (m_axis_rc_tlast),
        .m_axis_rc_tready           (m_axis_rc_tready),

        // RQ Sequence and Flow Control
        .pcie_rq_seq_num            (), 
        .pcie_rq_seq_num_vld        (),
        .pcie_rq_tag                (), //o. The IP core gives you a tag if you didn't provide one
        .pcie_rq_tag_av             (), //o
        .pcie_rq_tag_vld            (),
        .pcie_tfc_nph_av            (pcie_tfc_nph_av),
        .pcie_tfc_npd_av            (pcie_tfc_npd_av),
        .pcie_cq_np_req             (1'b1),
        .pcie_cq_np_req_count       (),

        // Configuration Status
        .cfg_phy_link_down          (),
        .cfg_phy_link_status        (),
        .cfg_negotiated_width       (),
        .cfg_current_speed          (),
        .cfg_max_payload            (o_cfg_max_payload),
        .cfg_max_read_req           (o_cfg_max_read_req),
        .cfg_function_status        (cfg_function_status), // Capturing BDF
        .cfg_function_power_state   (),
        .cfg_vf_status              (),
        .cfg_vf_power_state         (),
        .cfg_link_power_state       (),

        // Configuration Management
        .cfg_mgmt_addr              (cfg_mgmt_addr),
        .cfg_mgmt_write             (cfg_mgmt_write),
        .cfg_mgmt_write_data        (cfg_mgmt_write_data),
        .cfg_mgmt_byte_enable       (cfg_mgmt_byte_enable),
        .cfg_mgmt_read              (cfg_mgmt_read),
        .cfg_mgmt_read_data         (cfg_mgmt_read_data),
        .cfg_mgmt_read_write_done   (cfg_mgmt_read_write_done),
        .cfg_mgmt_type1_cfg_reg_access (1'b0),

        // Error Reporting
        .cfg_err_cor_out            (),
        .cfg_err_nonfatal_out       (),
        .cfg_err_fatal_out          (),
        .cfg_local_error            (),
        .cfg_ltr_enable             (),
        .cfg_ltssm_state            (),
        .cfg_rcb_status             (cfg_rcb_status),
        .cfg_dpa_substate_change    (),
        .cfg_obff_enable            (),
        .cfg_pl_status_change       (),
        .cfg_tph_requester_enable   (),
        .cfg_tph_st_mode            (),
        .cfg_vf_tph_requester_enable (),
        .cfg_vf_tph_st_mode         (),

        // Message Interface
        .cfg_msg_received           (),
        .cfg_msg_received_data      (),
        .cfg_msg_received_type      (),
        .cfg_msg_transmit           (1'b0),
        .cfg_msg_transmit_type      (3'd0),
        .cfg_msg_transmit_data      (32'd0),
        .cfg_msg_transmit_done      (),

        // Flow Control
        .cfg_fc_ph                  (cfg_fc_ph),
        .cfg_fc_pd                  (cfg_fc_pd),
        .cfg_fc_nph                 (cfg_fc_nph),
        .cfg_fc_npd                 (cfg_fc_npd),
        .cfg_fc_cplh                (cfg_fc_cplh),
        .cfg_fc_cpld                (cfg_fc_cpld),
        .cfg_fc_sel                 (cfg_fc_sel),

        // Per-Function Status
        .cfg_per_func_status_control (3'd0),
        .cfg_per_func_status_data   (),
        .cfg_per_function_number    (4'd0),
        .cfg_per_function_output_request (1'b0),
        .cfg_per_function_update_done (),

        // Device Serial Number
        .cfg_dsn                    (64'd0),

        // Power Management
        .cfg_power_state_change_ack (1'b1),
        .cfg_power_state_change_interrupt (),

        // Error Injection
        .cfg_err_cor_in             (status_error_cor),
        .cfg_err_uncor_in           (status_error_uncor),
        .cfg_flr_in_process         (),
        .cfg_flr_done               (4'd0),
        .cfg_vf_flr_in_process      (),
        .cfg_vf_flr_done            (8'd0),

        // Link Training
        .cfg_link_training_enable   (1'b1),

        // Legacy Interrupt
        .cfg_interrupt_int          (4'd0),
        .cfg_interrupt_pending      (4'd0),
        .cfg_interrupt_sent         (),

        // MSI-X Interrupt
        .cfg_interrupt_msix_enable  (cfg_interrupt_msix_enable),
        .cfg_interrupt_msix_mask    (cfg_interrupt_msix_mask),
        .cfg_interrupt_msix_vf_enable (cfg_interrupt_msix_vf_enable),
        .cfg_interrupt_msix_vf_mask (cfg_interrupt_msix_vf_mask),
        .cfg_interrupt_msix_address (cfg_interrupt_msix_address),
        .cfg_interrupt_msix_data    (cfg_interrupt_msix_data),
        .cfg_interrupt_msix_int     (cfg_interrupt_msix_int),
        .cfg_interrupt_msix_sent    (cfg_interrupt_msix_sent),
        .cfg_interrupt_msix_fail    (cfg_interrupt_msix_fail),
        .cfg_interrupt_msi_function_number (cfg_interrupt_msi_function_number),

        // Hot Reset
        .cfg_hot_reset_out          (),
        .cfg_config_space_enable    (1'b1),
        .cfg_req_pm_transition_l23_ready (1'b0),
        .cfg_hot_reset_in           (1'b0),

        // Downstream Port
        .cfg_ds_port_number         (8'd0),
        .cfg_ds_bus_number          (8'd0),
        .cfg_ds_device_number       (5'd0),
        .cfg_ds_function_number     (3'd0),

        // Subsystem Vendor ID
        .cfg_subsys_vend_id         (16'h0),

        // GT Clocking
        .int_qpll1lock_out          (),
        .int_qpll1outrefclk_out     (),
        .int_qpll1outclk_out        (),
        .phy_rdy_out                ()
    );

    assign o_pcie_requester_id = cfg_function_status;

endmodule
