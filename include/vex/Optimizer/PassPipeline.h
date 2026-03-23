#pragma once
// include/vex/Optimizer/PassPipeline.h


// ============================================================================
// vex/Optimizer/PassPipeline.h  — Builds LLVM pass pipelines
// ============================================================================
#include "vex/Optimizer/OptLevel.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Module.h"

namespace vex {

class PassPipeline {
public:
    // Build and run the standard optimization pipeline at opt level
    static bool run(llvm::Module& mod, OptLevel level,
                    bool enablePolly = false,
                    bool enablePGO   = false,
                    std::string_view pgoProfile = "");

    // Build only the function pass pipeline
    static bool runFunctionPasses(llvm::Module& mod, OptLevel level);

    // Build only LTO pipeline
    static bool runLTOPasses(llvm::Module& mod, OptLevel level);

private:
    static llvm::OptimizationLevel toLLVMLevel(OptLevel level);
    static void registerPollyPasses(llvm::PassBuilder& pb);
};
} // namespace vex
