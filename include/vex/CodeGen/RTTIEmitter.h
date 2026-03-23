#pragma once
// include/vex/CodeGen/RTTIEmitter.h



// ============================================================================
// vex/CodeGen/RTTIEmitter.h  — Emits runtime type info for dynamic dispatch
// ============================================================================
#include "vex/AST/Decl.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"

namespace vex {

class RTTIEmitter {
public:
    explicit RTTIEmitter(llvm::Module& mod) : mod_(mod) {}

    // Emit type ID constant for a type (stable hash)
    llvm::GlobalVariable* emitTypeId(TypeNode* ty);

    // Emit type name string
    llvm::Constant* emitTypeName(TypeNode* ty);

    // Emit size_of / align_of constants
    llvm::Constant* emitSizeOf(llvm::Type* ty);
    llvm::Constant* emitAlignOf(llvm::Type* ty);

    // Get or create the __vex_type_info global for a type
    llvm::GlobalVariable* getTypeInfo(TypeNode* ty);

private:
    llvm::Module& mod_;
    std::unordered_map<void*, llvm::GlobalVariable*> cache_;
};
} // namespace vex
