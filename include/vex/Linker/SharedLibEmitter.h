#pragma once
// include/vex/Linker/SharedLibEmitter.h


// vex/Linker/SharedLibEmitter.h  — Helpers for shared library emission
#include "vex/Linker/LinkerOptions.h"
#include "vex/Core/DiagnosticEngine.h"
namespace vex {
class SharedLibEmitter {
public:
    SharedLibEmitter(DiagnosticEngine& diags) : diags_(diags) {}
    bool emit(const LinkerOptions& opts);
private:
    DiagnosticEngine& diags_;
    void addExportFlags(LinkerOptions& opts);
};
} // namespace vex
