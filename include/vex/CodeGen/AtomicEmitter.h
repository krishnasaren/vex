#pragma once
// include/vex/CodeGen/AtomicEmitter.h



// ============================================================================
// vex/CodeGen/AtomicEmitter.h  — Emits atomic operations for std::sync types
// ============================================================================
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include <string_view>

namespace vex {

class AtomicEmitter {
public:
    explicit AtomicEmitter(llvm::IRBuilder<>& b) : builder_(b) {}

    llvm::Value* emitAtomicLoad(llvm::Value* ptr,
                                 llvm::AtomicOrdering order = llvm::AtomicOrdering::SequentiallyConsistent);
    void         emitAtomicStore(llvm::Value* ptr, llvm::Value* val,
                                  llvm::AtomicOrdering order = llvm::AtomicOrdering::SequentiallyConsistent);
    llvm::Value* emitAtomicRMW(llvm::AtomicRMWInst::BinOp op,
                                 llvm::Value* ptr, llvm::Value* val,
                                 llvm::AtomicOrdering order);
    llvm::Value* emitCmpXchg(llvm::Value* ptr, llvm::Value* cmp,
                               llvm::Value* newVal,
                               llvm::AtomicOrdering success,
                               llvm::AtomicOrdering failure);
    void         emitFence(llvm::AtomicOrdering order);

    llvm::AtomicOrdering orderFromString(std::string_view s);

private:
    llvm::IRBuilder<>& builder_;
};
} // namespace vex
