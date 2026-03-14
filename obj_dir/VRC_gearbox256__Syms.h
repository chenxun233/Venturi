// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VRC_GEARBOX256__SYMS_H_
#define VERILATED_VRC_GEARBOX256__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "VRC_gearbox256.h"

// INCLUDE MODULE CLASSES
#include "VRC_gearbox256___024root.h"

// SYMS CLASS (contains all model state)
class alignas(VL_CACHE_LINE_BYTES)VRC_gearbox256__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    VRC_gearbox256* const __Vm_modelp;
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    VRC_gearbox256___024root       TOP;

    // CONSTRUCTORS
    VRC_gearbox256__Syms(VerilatedContext* contextp, const char* namep, VRC_gearbox256* modelp);
    ~VRC_gearbox256__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
};

#endif  // guard
