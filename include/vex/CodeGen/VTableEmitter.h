#pragma once
// include/vex/CodeGen/VTableEmitter.h



// ============================================================================
// vex/CodeGen/VTableEmitter.h  — Generates vtable / trait object dispatch tables
// ============================================================================
#include "vex/AST/Decl.h"
#include "vex/CodeGen/LLVMTypeMapper.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/GlobalVariable.h"

namespace vex {

class VTableEmitter {
public:
    VTableEmitter(llvm::Module& mod, LLVMTypeMapper& types)
        : mod_(mod), types_(types) {}

    // Emit the vtable for a class (virtual method pointers)
    llvm::GlobalVariable* emitClassVTable(ClassDecl* cls);

    // Emit a trait object vtable (impl block for a concrete type)
    llvm::GlobalVariable* emitTraitVTable(ImplDecl* impl, TraitDecl* trait);

    // Get or declare the vtable global
    llvm::GlobalVariable* getVTable(ClassDecl* cls);
    llvm::GlobalVariable* getTraitVTable(ImplDecl* impl, TraitDecl* trait);

    // Build the vtable struct type for a class
    llvm::StructType* vtableType(ClassDecl* cls);
    llvm::StructType* traitVTableType(TraitDecl* trait);

private:
    llvm::Module&   mod_;
    LLVMTypeMapper& types_;
    std::unordered_map<ClassDecl*, llvm::GlobalVariable*>  classVTables_;
    std::unordered_map<std::string, llvm::GlobalVariable*> traitVTables_;
};
} // namespace vex



