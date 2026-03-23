#pragma once
// include/vex/Linker/ObjectEmitter.h


// ============================================================================
// vex/Linker/ObjectEmitter.h  — Emits object files from LLVM modules
// ============================================================================
#include "vex/Driver/Options.h"
#include "vex/Core/DiagnosticEngine.h"
#include "llvm/IR/Module.h"
#include <string_view>

namespace vex {

class ObjectEmitter {
public:
    ObjectEmitter(DiagnosticEngine& diags, const DriverOptions& opts)
        : diags_(diags), opts_(opts) {}

    // Emit an object file (.o) from an LLVM module
    bool emit(llvm::Module& mod, std::string_view outputPath);

private:
    DiagnosticEngine&    diags_;
    const DriverOptions& opts_;

    bool setupTargetMachine(llvm::Module& mod,
                             std::unique_ptr<llvm::TargetMachine>& tm);
};
} // namespace vex
