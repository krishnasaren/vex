#pragma once
// include/vex/CodeGen/TypeEmitter.h


// ============================================================================
// vex/CodeGen/TypeEmitter.h  — Emits RTTI / type descriptor structs
// ============================================================================
#include "vex/AST/Type.h"
#include "vex/AST/Decl.h"
#include "llvm/IR/Module.h"

namespace vex {
class LLVMTypeMapper;

class TypeEmitter {
public:
    TypeEmitter(llvm::Module& mod, LLVMTypeMapper& types)
        : mod_(mod), types_(types) {}

    // Emit a struct type layout
    llvm::StructType* emitStruct(StructDecl* s);
    // Emit an enum discriminant + variant union
    llvm::StructType* emitEnum(EnumDecl* e);
    // Emit a class layout with vtable pointer
    llvm::StructType* emitClass(ClassDecl* c);
    // Emit a trait object fat pointer: (data*, vtable*)
    llvm::StructType* traitObjectType(TraitDecl* t);

private:
    llvm::Module&   mod_;
    LLVMTypeMapper& types_;
};
} // namespace vex
