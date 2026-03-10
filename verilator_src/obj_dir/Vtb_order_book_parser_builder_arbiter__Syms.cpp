// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table implementation internals

#include "Vtb_order_book_parser_builder_arbiter__pch.h"
#include "Vtb_order_book_parser_builder_arbiter.h"
#include "Vtb_order_book_parser_builder_arbiter___024root.h"
#include "Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2.h"
#include "Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8.h"

// FUNCTIONS
Vtb_order_book_parser_builder_arbiter__Syms::~Vtb_order_book_parser_builder_arbiter__Syms()
{
}

Vtb_order_book_parser_builder_arbiter__Syms::Vtb_order_book_parser_builder_arbiter__Syms(VerilatedContext* contextp, const char* namep, Vtb_order_book_parser_builder_arbiter* modelp)
    : VerilatedSyms{contextp}
    // Setup internal state of the Syms class
    , __Vm_modelp{modelp}
    // Setup module instances
    , TOP{this, namep}
    , TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst{this, Verilated::catName(namep, "tb_order_book_parser_builder_arbiter.builder_dut.symbol_book_AAPL.ask_wrapper_inst")}
    , TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst{this, Verilated::catName(namep, "tb_order_book_parser_builder_arbiter.builder_dut.symbol_book_AAPL.bid_wrapper_inst")}
    , TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst{this, Verilated::catName(namep, "tb_order_book_parser_builder_arbiter.builder_dut.symbol_book_HSBC.ask_wrapper_inst")}
    , TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst{this, Verilated::catName(namep, "tb_order_book_parser_builder_arbiter.builder_dut.symbol_book_HSBC.bid_wrapper_inst")}
{
    // Configure time unit / time precision
    _vm_contextp__->timeunit(-9);
    _vm_contextp__->timeprecision(-12);
    // Setup each module's pointers to their submodules
    TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst = &TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst;
    TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst = &TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst;
    TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst = &TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst;
    TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst = &TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst;
    // Setup each module's pointer back to symbol table (for public functions)
    TOP.__Vconfigure(true);
    TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__Vconfigure(true);
    TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__Vconfigure(true);
    TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__Vconfigure(false);
    TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__Vconfigure(false);
    // Setup scopes
    __Vscope_tb_order_book_parser_builder_arbiter.configure(this, name(), "tb_order_book_parser_builder_arbiter", "tb_order_book_parser_builder_arbiter", -9, VerilatedScope::SCOPE_OTHER);
}
