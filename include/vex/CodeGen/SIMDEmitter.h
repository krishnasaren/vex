#pragma once
// include/vex/CodeGen/SIMDEmitter.h



// ============================================================================
// vex/CodeGen/SIMDEmitter.h  — SIMD vector intrinsics
// ============================================================================
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include <vector>

namespace vex {

class SIMDEmitter {
public:
    explicit SIMDEmitter(llvm::IRBuilder<>& b) : builder_(b) {}

    llvm::Value* emitSplat(llvm::Value* scalar, unsigned lanes);
    llvm::Value* emitVectorAdd(llvm::Value* a, llvm::Value* b);
    llvm::Value* emitVectorSub(llvm::Value* a, llvm::Value* b);
    llvm::Value* emitVectorMul(llvm::Value* a, llvm::Value* b);
    llvm::Value* emitVectorDiv(llvm::Value* a, llvm::Value* b);
    llvm::Value* emitHorizontalSum(llvm::Value* v);
    llvm::Value* emitShuffle(llvm::Value* a, llvm::Value* b,
                              std::vector<int> mask);
    llvm::Value* emitExtractElement(llvm::Value* v, unsigned idx);
    llvm::Value* emitInsertElement(llvm::Value* v, llvm::Value* elem, unsigned idx);

private:
    llvm::IRBuilder<>& builder_;
};
} // namespace vex
