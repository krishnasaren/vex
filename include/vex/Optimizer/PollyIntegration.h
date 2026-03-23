#pragma once
// include/vex/Optimizer/PollyIntegration.h


// ============================================================================
// vex/Optimizer/PollyIntegration.h  — Polly loop optimization integration
// ============================================================================
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"

namespace vex {

class PollyIntegration {
public:
    // Returns true if Polly is compiled in
    static bool isAvailable();

    // Register Polly passes with the pass builder
    static void registerPasses(llvm::PassBuilder& pb);

    // Run Polly on module
    static bool run(llvm::Module& mod);
};
} // namespace vex
