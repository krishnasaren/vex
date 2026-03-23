#pragma once
// include/vex/Linker/WasmEmitter.h


// vex/Linker/WasmEmitter.h  — Wasm-specific linking helpers
#include "vex/Linker/LinkerOptions.h"
#include "vex/Core/DiagnosticEngine.h"
namespace vex {
class WasmEmitter {
public:
    WasmEmitter(DiagnosticEngine& diags) : diags_(diags) {}
    bool emit(const LinkerOptions& opts);
private:
    DiagnosticEngine& diags_;
    void addWasmFlags(LinkerOptions& opts);
};
} // namespace vex
