#pragma once
// include/vex/Optimizer/Optimizer.h



// ============================================================================
// vex/Optimizer/Optimizer.h  — Top-level optimizer driver
// ============================================================================
#include "vex/Optimizer/OptLevel.h"
#include "vex/Driver/Options.h"
#include "vex/Core/DiagnosticEngine.h"
#include "llvm/IR/Module.h"

namespace vex {

class Optimizer {
public:
    Optimizer(DiagnosticEngine& diags, const DriverOptions& opts)
        : diags_(diags), opts_(opts) {}

    // Optimize the LLVM module in-place
    bool run(llvm::Module& mod);

private:
    DiagnosticEngine&    diags_;
    const DriverOptions& opts_;

    OptLevel effectiveLevel() const;
};
} // namespace vex
