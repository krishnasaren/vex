#pragma once
// include/vex/CodeGen/LLVMTypeMapper.h



// ============================================================================
// vex/CodeGen/LLVMTypeMapper.h  — Maps VEX types → LLVM types
// ============================================================================
#include "vex/IR/IRType.h"
#include "vex/AST/Type.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DerivedTypes.h"
#include <unordered_map>

namespace vex {

class LLVMTypeMapper {
public:
    explicit LLVMTypeMapper(llvm::LLVMContext& ctx) : ctx_(ctx) {}

    // Map a VEX high-level AST type to an LLVM type
    llvm::Type* map(TypeNode* ty);

    // Map a VEX IR type to an LLVM type
    llvm::Type* map(IRType* ty);

    // Common shortcuts
    llvm::Type*     voidTy()    { return llvm::Type::getVoidTy(ctx_); }
    llvm::Type*     boolTy()    { return llvm::Type::getInt1Ty(ctx_); }
    llvm::Type*     i8Ty()      { return llvm::Type::getInt8Ty(ctx_); }
    llvm::Type*     i32Ty()     { return llvm::Type::getInt32Ty(ctx_); }
    llvm::Type*     i64Ty()     { return llvm::Type::getInt64Ty(ctx_); }
    llvm::Type*     f32Ty()     { return llvm::Type::getFloatTy(ctx_); }
    llvm::Type*     f64Ty()     { return llvm::Type::getDoubleTy(ctx_); }
    llvm::PointerType* ptrTy(llvm::Type* pointee = nullptr, unsigned addrSpace = 0);
    llvm::StructType* opaqueStructTy(std::string_view name);

private:
    llvm::LLVMContext& ctx_;
    std::unordered_map<void*, llvm::Type*> cache_;

    llvm::Type* mapBuiltinType(std::string_view name);
    llvm::Type* mapPointerType(TypeNode* ty);
    llvm::Type* mapArrayType(TypeNode* ty);
    llvm::Type* mapSliceType(TypeNode* ty);
    llvm::Type* mapTupleType(TypeNode* ty);
    llvm::Type* mapFnType(TypeNode* ty);
    llvm::Type* mapStructType(StructDecl* decl);
    llvm::Type* mapClassType(ClassDecl* decl);
    llvm::Type* mapEnumType(EnumDecl* decl);
    llvm::Type* mapGenericType(TypeNode* ty);
};

} // namespace vex
