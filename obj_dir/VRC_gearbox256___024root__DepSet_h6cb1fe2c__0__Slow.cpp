// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See VRC_gearbox256.h for the primary calling header

#include "VRC_gearbox256__pch.h"
#include "VRC_gearbox256__Syms.h"
#include "VRC_gearbox256___024root.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__stl(VRC_gearbox256___024root* vlSelf);
#endif  // VL_DEBUG

VL_ATTR_COLD void VRC_gearbox256___024root___eval_triggers__stl(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_triggers__stl\n"); );
    // Body
    vlSelf->__VstlTriggered.set(0U, (IData)(vlSelf->__VstlFirstIteration));
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        VRC_gearbox256___024root___dump_triggers__stl(vlSelf);
    }
#endif
}
