#pragma once
// include/vex/Optimizer/PGOInstrumentation.h


// ============================================================================
// vex/Optimizer/PGOInstrumentation.h  — Profile-Guided Optimization
// ============================================================================
#include "vex/Driver/Options.h"
#include "vex/Core/DiagnosticEngine.h"
#include "llvm/IR/Module.h"

namespace vex {

class PGOInstrumentation {
public:
    PGOInstrumentation(DiagnosticEngine& diags, const DriverOptions& opts)
        : diags_(diags), opts_(opts) {}

    // Add profiling instrumentation to module
    bool instrument(llvm::Module& mod);

    // Apply profile data to module for optimization
    bool applyProfile(llvm::Module& mod, std::string_view profilePath);

private:
    DiagnosticEngine&    diags_;
    const DriverOptions& opts_;
};
} // namespace vex
