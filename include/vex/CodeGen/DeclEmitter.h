#pragma once
// include/vex/CodeGen/DeclEmitter.h




// ============================================================================
// vex/CodeGen/DeclEmitter.h  — Emits top-level declarations (functions, globals)
// ============================================================================
#include "vex/AST/Decl.h"
#include "vex/CodeGen/LLVMTypeMapper.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

namespace vex {
class LLVMEmitter;

class DeclEmitter {
public:
    DeclEmitter(LLVMEmitter& parent, llvm::Module& mod) : parent_(parent), mod_(mod) {}

    llvm::Function*       emitFnDecl(FnDecl* fn);
    llvm::GlobalVariable* emitConstDecl(ConstDecl* c);
    llvm::GlobalVariable* emitGlobal(VarDecl* v);
    llvm::StructType*     emitStructType(StructDecl* s);
    llvm::StructType*     emitClassType(ClassDecl* c);
    void                  emitVTable(ClassDecl* c);
    void                  emitImplMethods(ImplDecl* impl);

private:
    LLVMEmitter& parent_;
    llvm::Module& mod_;
};
} // namespace vex
