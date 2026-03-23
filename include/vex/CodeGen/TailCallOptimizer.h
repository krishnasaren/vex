#pragma once
// include/vex/CodeGen/TailCallOptimizer.h



// ============================================================================
// vex/CodeGen/TailCallOptimizer.h  — Marks eligible calls as musttail
// ============================================================================
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

namespace vex {

class TailCallOptimizer {
public:
    // Process module: mark all eligible tail calls
    static bool run(llvm::Module& mod);

    // Check if a specific call instruction can be marked musttail
    static bool isEligible(llvm::CallInst* call);

private:
    static bool isSelfRecursiveTailCall(llvm::CallInst* call, llvm::Function* fn);
    static void markTailCall(llvm::CallInst* call);
};
} // namespace vex