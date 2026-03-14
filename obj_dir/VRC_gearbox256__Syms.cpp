// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table implementation internals

#include "VRC_gearbox256__pch.h"
#include "VRC_gearbox256.h"
#include "VRC_gearbox256___024root.h"

// FUNCTIONS
VRC_gearbox256__Syms::~VRC_gearbox256__Syms()
{
}

VRC_gearbox256__Syms::VRC_gearbox256__Syms(VerilatedContext* contextp, const char* namep, VRC_gearbox256* modelp)
    : VerilatedSyms{contextp}
    // Setup internal state of the Syms class
    , __Vm_modelp{modelp}
    // Setup module instances
    , TOP{this, namep}
{
    // Configure time unit / time precision
    _vm_contextp__->timeunit(-12);
    _vm_contextp__->timeprecision(-12);
    // Setup each module's pointers to their submodules
    // Setup each module's pointer back to symbol table (for public functions)
    TOP.__Vconfigure(true);
}
