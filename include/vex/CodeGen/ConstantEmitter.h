#pragma once
// include/vex/CodeGen/ConstantEmitter.h


// ============================================================================
// vex/CodeGen/ConstantEmitter.h  — Emits LLVM constants from comptime values
// ============================================================================
#include "vex/Sema/ComptimeEvaluator.h"
#include "vex/CodeGen/LLVMTypeMapper.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Module.h"

namespace vex {

class ConstantEmitter {
public:
    ConstantEmitter(llvm::Module& mod, LLVMTypeMapper& types)
        : mod_(mod), types_(types) {}

    llvm::Constant* emit(const ComptimeValue& val, llvm::Type* ty = nullptr);
    llvm::Constant* emitInt(int64_t v, unsigned bits, bool isSigned = true);
    llvm::Constant* emitFloat(double v, unsigned bits);
    llvm::Constant* emitBool(bool v);
    llvm::Constant* emitString(std::string_view s);
    llvm::Constant* emitNull(llvm::PointerType* ty);
    llvm::Constant* emitArray(const std::vector<ComptimeValue>& elems, llvm::Type* elemTy);
    llvm::Constant* emitZeroInit(llvm::Type* ty);

private:
    llvm::Module&   mod_;
    LLVMTypeMapper& types_;
};
} // namespace vex
