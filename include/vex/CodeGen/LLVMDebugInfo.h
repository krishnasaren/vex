#pragma once
// include/vex/CodeGen/LLVMDebugInfo.h

// ============================================================================
// vex/CodeGen/LLVMDebugInfo.h  — DWARF debug info generation
// ============================================================================
#include "vex/Core/SourceLocation.h"
#include "vex/AST/Decl.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Module.h"
#include <string_view>

namespace vex {

class LLVMTypeMapper;

class LLVMDebugInfoEmitter {
public:
    LLVMDebugInfoEmitter(llvm::Module& mod, LLVMTypeMapper& typeMap,
                          std::string_view producer);

    void finalize();

    llvm::DIFile*       createFile(std::string_view filename, std::string_view dir);
    llvm::DISubprogram* createFunction(FnDecl* fn, llvm::Function* llvmFn);
    llvm::DIType*       createType(TypeNode* ty);
    llvm::DILocalVariable* createLocalVar(std::string_view name, TypeNode* ty,
                                           unsigned line, llvm::DIScope* scope);
    void setLocation(llvm::IRBuilder<>& b, SourceLocation loc, llvm::DIScope* scope);

private:
    llvm::DIBuilder    builder_;
    LLVMTypeMapper&    typeMap_;
    llvm::DICompileUnit* cu_  = nullptr;
    llvm::DIFile*       file_ = nullptr;

    llvm::DIType* mapBuiltinType(std::string_view name);
};

} // namespace vex
