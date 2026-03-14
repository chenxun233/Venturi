// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "VRC_gearbox256__pch.h"

//============================================================
// Constructors

VRC_gearbox256::VRC_gearbox256(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new VRC_gearbox256__Syms(contextp(), _vcname__, this)}
    , clk{vlSymsp->TOP.clk}
    , rst_n{vlSymsp->TOP.rst_n}
    , m_axis_rc_tvalid{vlSymsp->TOP.m_axis_rc_tvalid}
    , m_axis_rc_tkeep{vlSymsp->TOP.m_axis_rc_tkeep}
    , m_axis_rc_tlast{vlSymsp->TOP.m_axis_rc_tlast}
    , m_axis_rc_tready{vlSymsp->TOP.m_axis_rc_tready}
    , rc_valid{vlSymsp->TOP.rc_valid}
    , rc_payload_last{vlSymsp->TOP.rc_payload_last}
    , rc_payload_dw_keep{vlSymsp->TOP.rc_payload_dw_keep}
    , m_axis_rc_tdata{vlSymsp->TOP.m_axis_rc_tdata}
    , m_axis_rc_tuser{vlSymsp->TOP.m_axis_rc_tuser}
    , rc_payload{vlSymsp->TOP.rc_payload}
    , rc_descriptor{vlSymsp->TOP.rc_descriptor}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

VRC_gearbox256::VRC_gearbox256(const char* _vcname__)
    : VRC_gearbox256(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

VRC_gearbox256::~VRC_gearbox256() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void VRC_gearbox256___024root___eval_debug_assertions(VRC_gearbox256___024root* vlSelf);
#endif  // VL_DEBUG
void VRC_gearbox256___024root___eval_static(VRC_gearbox256___024root* vlSelf);
void VRC_gearbox256___024root___eval_initial(VRC_gearbox256___024root* vlSelf);
void VRC_gearbox256___024root___eval_settle(VRC_gearbox256___024root* vlSelf);
void VRC_gearbox256___024root___eval(VRC_gearbox256___024root* vlSelf);

void VRC_gearbox256::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate VRC_gearbox256::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    VRC_gearbox256___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        VRC_gearbox256___024root___eval_static(&(vlSymsp->TOP));
        VRC_gearbox256___024root___eval_initial(&(vlSymsp->TOP));
        VRC_gearbox256___024root___eval_settle(&(vlSymsp->TOP));
    }
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    VRC_gearbox256___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool VRC_gearbox256::eventsPending() { return false; }

uint64_t VRC_gearbox256::nextTimeSlot() {
    VL_FATAL_MT(__FILE__, __LINE__, "", "%Error: No delays in the design");
    return 0;
}

//============================================================
// Utilities

const char* VRC_gearbox256::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void VRC_gearbox256___024root___eval_final(VRC_gearbox256___024root* vlSelf);

VL_ATTR_COLD void VRC_gearbox256::final() {
    VRC_gearbox256___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* VRC_gearbox256::hierName() const { return vlSymsp->name(); }
const char* VRC_gearbox256::modelName() const { return "VRC_gearbox256"; }
unsigned VRC_gearbox256::threads() const { return 1; }
void VRC_gearbox256::prepareClone() const { contextp()->prepareClone(); }
void VRC_gearbox256::atClone() const {
    contextp()->threadPoolpOnClone();
}

//============================================================
// Trace configuration

VL_ATTR_COLD void VRC_gearbox256::trace(VerilatedVcdC* tfp, int levels, int options) {
    vl_fatal(__FILE__, __LINE__, __FILE__,"'VRC_gearbox256::trace()' called on model that was Verilated without --trace option");
}
