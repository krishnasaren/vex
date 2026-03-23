#pragma once
// include/vex/CodeGen/IntrinsicEmitter.h



// ============================================================================
// vex/CodeGen/IntrinsicEmitter.h  — Emits LLVM intrinsic calls for
// built-in operations (math, memory, atomics, SIMD).
// ============================================================================
#include "vex/AST/Expr.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Intrinsics.h"
#include <vector>
#include <string_view>

namespace vex {

class IntrinsicEmitter {
public:
    IntrinsicEmitter(llvm::Module& mod, llvm::IRBuilder<>& b)
        : mod_(mod), builder_(b) {}

    // Emit a call to an LLVM intrinsic by Intrinsic::ID
    llvm::Value* emit(llvm::Intrinsic::ID id,
                      std::vector<llvm::Value*> args,
                      std::string_view name = "");

    // Common intrinsics
    llvm::Value* emitMemCpy(llvm::Value* dst, llvm::Value* src,
                             llvm::Value* len, unsigned align = 1);
    llvm::Value* emitMemSet(llvm::Value* ptr, llvm::Value* val,
                             llvm::Value* len, unsigned align = 1);
    llvm::Value* emitMathSqrt(llvm::Value* x);
    llvm::Value* emitMathAbs(llvm::Value* x);
    llvm::Value* emitMathPow(llvm::Value* base, llvm::Value* exp);
    llvm::Value* emitMathFloor(llvm::Value* x);
    llvm::Value* emitMathCeil(llvm::Value* x);
    llvm::Value* emitBitReverse(llvm::Value* x);
    llvm::Value* emitPopCount(llvm::Value* x);
    llvm::Value* emitCLZ(llvm::Value* x, bool isZeroUndef = false);
    llvm::Value* emitCTZ(llvm::Value* x, bool isZeroUndef = false);

    // Checked arithmetic (overflow intrinsics)
    llvm::Value* emitSAddOverflow(llvm::Value* a, llvm::Value* b);
    llvm::Value* emitSSubOverflow(llvm::Value* a, llvm::Value* b);
    llvm::Value* emitSMulOverflow(llvm::Value* a, llvm::Value* b);

private:
    llvm::Module&      mod_;
    llvm::IRBuilder<>& builder_;
};
} // namespace vex
