// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vtb_order_book_parser_builder_arbiter.h for the primary calling header

#ifndef VERILATED_VTB_ORDER_BOOK_PARSER_BUILDER_ARBITER___024ROOT_H_
#define VERILATED_VTB_ORDER_BOOK_PARSER_BUILDER_ARBITER___024ROOT_H_  // guard

#include "verilated.h"
#include "verilated_timing.h"
class Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8;
class Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2;


class Vtb_order_book_parser_builder_arbiter__Syms;

class alignas(VL_CACHE_LINE_BYTES) Vtb_order_book_parser_builder_arbiter___024root final : public VerilatedModule {
  public:
    // CELLS
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst;
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst;
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst;
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst;

    // DESIGN SPECIFIC STATE
    // Anonymous structures to workaround compiler member-count bugs
    struct {
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__i_clk_156;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__i_rst;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__i_axi_rx_valid;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__i_axi_rx_last;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__i_builder_ff_pop;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__o_msg_valid;
        CData/*7:0*/ tb_order_book_parser_builder_arbiter__DOT__o_msg_type;
        CData/*7:0*/ tb_order_book_parser_builder_arbiter__DOT__o_buy_sell;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__batch_checks_en;
        CData/*7:0*/ tb_order_book_parser_builder_arbiter__DOT____Vlvbound_hea2c4470__0;
        CData/*7:0*/ tb_order_book_parser_builder_arbiter__DOT____Vlvbound_hea2c4470__1;
        CData/*7:0*/ tb_order_book_parser_builder_arbiter__DOT____Vlvbound_h07a64770__0;
        CData/*3:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__head_counter;
        CData/*6:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__buffed_bytes;
        CData/*6:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__valid_bytes;
        CData/*7:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__peeked_type;
        CData/*4:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__state;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_ff_valid;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_ff_valid;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__arb_src_not_empty;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_AAPL__i_ff_pop;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_HSBC__i_ff_pop;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_valid;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__do_push;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__qty_bid_ask;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__qty_is_add;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_valid;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_pop;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_upd_state;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_op;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT____Vlvbound_hb0763474__0;
        CData/*2:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
        CData/*2:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
        CData/*3:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_valid;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__do_push;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__qty_bid_ask;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__qty_is_add;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_valid;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_pop;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_upd_state;
        CData/*1:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_op;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT____Vlvbound_hb0763474__0;
        CData/*2:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
        CData/*2:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
        CData/*3:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__rr_ptr_q;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_c;
        CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__have_grant_c;
        CData/*3:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__head_counter;
        CData/*4:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__state;
        CData/*6:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__buffed_bytes;
        CData/*7:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__peeked_type;
        CData/*1:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_upd_state;
        CData/*1:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_upd_state;
    };
    struct {
        CData/*0:0*/ __VstlDidInit;
        CData/*0:0*/ __VstlFirstIteration;
        CData/*0:0*/ __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_clk_156__0;
        CData/*0:0*/ __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_rst__0;
        CData/*0:0*/ __VactDidInit;
        CData/*0:0*/ __VactContinue;
        SData/*15:0*/ tb_order_book_parser_builder_arbiter__DOT__o_stock_locate;
        SData/*15:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__msg_count;
        SData/*11:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_addr;
        SData/*11:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_addr;
        SData/*15:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__msg_count;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__o_shares;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__o_price;
        VlWide<9>/*273:0*/ tb_order_book_parser_builder_arbiter__DOT__o_builder_payload;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__round_idx;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__total_parser_msgs;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__total_payloads;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__aapl_payloads;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__hsbc_payloads;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__errors;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__batch_payloads;
        VlWide<16>/*511:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff;
        VlWide<16>/*511:0*/ tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff;
        VlWide<9>/*273:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload;
        VlWide<9>/*273:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload;
        VlWide<9>/*282:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_price;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_shares;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_price;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_shares;
        VlWide<5>/*130:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__qty_msg;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__qty_price;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__qty_d_shares;
        VlWide<9>/*282:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg;
        VlWide<3>/*66:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_i_data;
        VlWide<3>/*66:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_price;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_shares;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_price;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_shares;
        VlWide<5>/*130:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__qty_msg;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__qty_price;
        IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__qty_d_shares;
        VlWide<9>/*282:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg;
        VlWide<3>/*66:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_i_data;
        VlWide<3>/*66:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data;
        VlWide<9>/*282:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg;
        VlWide<3>/*66:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data;
        VlWide<9>/*282:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg;
        VlWide<3>/*66:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data;
        VlWide<16>/*511:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff;
        IData/*31:0*/ __VactIterCount;
        QData/*63:0*/ tb_order_book_parser_builder_arbiter__DOT__i_axi_rx_data;
        QData/*63:0*/ tb_order_book_parser_builder_arbiter__DOT__i_axi_rx_ingress_tick;
        QData/*63:0*/ tb_order_book_parser_builder_arbiter__DOT__o_seq_num;
        QData/*63:0*/ tb_order_book_parser_builder_arbiter__DOT__o_order_ref_num;
        QData/*63:0*/ tb_order_book_parser_builder_arbiter__DOT__o_new_order_ref_num;
        QData/*63:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__qty_seq_num;
        QData/*63:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__qty_seq_num;
        VlUnpacked<CData/*7:0*/, 359> tb_order_book_parser_builder_arbiter__DOT__frame_bytes;
        VlUnpacked<VlWide<9>/*273:0*/, 2> tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem;
        VlUnpacked<VlWide<9>/*282:0*/, 8> tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem;
        VlUnpacked<VlWide<3>/*66:0*/, 4096> tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram;
        VlUnpacked<VlWide<9>/*273:0*/, 2> tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem;
    };
    struct {
        VlUnpacked<VlWide<9>/*282:0*/, 8> tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem;
        VlUnpacked<VlWide<3>/*66:0*/, 4096> tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1;
        VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1;
        VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1;
    };
    VlDelayScheduler __VdlySched;
    VlTriggerScheduler __VtrigSched_h3dc8676a__0;
    VlTriggerVec<17> __VstlTriggered;
    VlTriggerVec<19> __VactTriggered;
    VlTriggerVec<19> __VnbaTriggered;

    // INTERNAL VARIABLES
    Vtb_order_book_parser_builder_arbiter__Syms* const vlSymsp;

    // CONSTRUCTORS
    Vtb_order_book_parser_builder_arbiter___024root(Vtb_order_book_parser_builder_arbiter__Syms* symsp, const char* v__name);
    ~Vtb_order_book_parser_builder_arbiter___024root();
    VL_UNCOPYABLE(Vtb_order_book_parser_builder_arbiter___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};


#endif  // guard
