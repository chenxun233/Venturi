// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "Vtb_order_book_parser_builder_arbiter__pch.h"

//============================================================
// Constructors

Vtb_order_book_parser_builder_arbiter::Vtb_order_book_parser_builder_arbiter(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new Vtb_order_book_parser_builder_arbiter__Syms(contextp(), _vcname__, this)}
    , __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst{vlSymsp->TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst}
    , __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst{vlSymsp->TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst}
    , __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst{vlSymsp->TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst}
    , __PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst{vlSymsp->TOP.__PVT__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

Vtb_order_book_parser_builder_arbiter::Vtb_order_book_parser_builder_arbiter(const char* _vcname__)
    : Vtb_order_book_parser_builder_arbiter(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

Vtb_order_book_parser_builder_arbiter::~Vtb_order_book_parser_builder_arbiter() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void Vtb_order_book_parser_builder_arbiter___024root___eval_debug_assertions(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
#endif  // VL_DEBUG
void Vtb_order_book_parser_builder_arbiter___024root___eval_static(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter___024root___eval_initial(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter___024root___eval_settle(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter___024root___eval(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);

void Vtb_order_book_parser_builder_arbiter::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vtb_order_book_parser_builder_arbiter::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    Vtb_order_book_parser_builder_arbiter___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        Vtb_order_book_parser_builder_arbiter___024root___eval_static(&(vlSymsp->TOP));
        Vtb_order_book_parser_builder_arbiter___024root___eval_initial(&(vlSymsp->TOP));
        Vtb_order_book_parser_builder_arbiter___024root___eval_settle(&(vlSymsp->TOP));
    }
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    Vtb_order_book_parser_builder_arbiter___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool Vtb_order_book_parser_builder_arbiter::eventsPending() { return !vlSymsp->TOP.__VdlySched.empty(); }

uint64_t Vtb_order_book_parser_builder_arbiter::nextTimeSlot() { return vlSymsp->TOP.__VdlySched.nextTimeSlot(); }

//============================================================
// Utilities

const char* Vtb_order_book_parser_builder_arbiter::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void Vtb_order_book_parser_builder_arbiter___024root___eval_final(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);

VL_ATTR_COLD void Vtb_order_book_parser_builder_arbiter::final() {
    Vtb_order_book_parser_builder_arbiter___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* Vtb_order_book_parser_builder_arbiter::hierName() const { return vlSymsp->name(); }
const char* Vtb_order_book_parser_builder_arbiter::modelName() const { return "Vtb_order_book_parser_builder_arbiter"; }
unsigned Vtb_order_book_parser_builder_arbiter::threads() const { return 1; }
void Vtb_order_book_parser_builder_arbiter::prepareClone() const { contextp()->prepareClone(); }
void Vtb_order_book_parser_builder_arbiter::atClone() const {
    contextp()->threadPoolpOnClone();
}

//============================================================
// Trace configuration

VL_ATTR_COLD void Vtb_order_book_parser_builder_arbiter::trace(VerilatedVcdC* tfp, int levels, int options) {
    vl_fatal(__FILE__, __LINE__, __FILE__,"'Vtb_order_book_parser_builder_arbiter::trace()' called on model that was Verilated without --trace option");
}
