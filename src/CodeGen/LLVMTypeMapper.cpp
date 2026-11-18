// src/CodeGen/LLVMTypeMapper.cpp
#include "vex/CodeGen/LLVMTypeMapper.h"
#include "vex/AST/Type.h"
#include "vex/AST/Decl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

namespace vex {

// ── Map VexIR type to LLVM type ───────────────────────────────────────────────

llvm::Type* LLVMTypeMapper::map(IRType* ty) {
    if (!ty) return i64Ty();
    switch (ty->kind()) {
    case IRTypeKind::Void:    return voidTy();
    case IRTypeKind::Bool:    return boolTy();
    case IRTypeKind::Int1:    return llvm::Type::getInt1Ty(ctx_);
    case IRTypeKind::Int8:
    case IRTypeKind::UInt8:   return i8Ty();
    case IRTypeKind::Int16:
    case IRTypeKind::UInt16:  return llvm::Type::getInt16Ty(ctx_);
    case IRTypeKind::Int32:
    case IRTypeKind::UInt32:  return i32Ty();
    case IRTypeKind::Int64:
    case IRTypeKind::UInt64:  return i64Ty();
    case IRTypeKind::Int128:  return llvm::Type::getInt128Ty(ctx_);
    case IRTypeKind::Float32: return f32Ty();
    case IRTypeKind::Float64: return f64Ty();
    case IRTypeKind::Float128:return llvm::Type::getX86_FP80Ty(ctx_);
    case IRTypeKind::Pointer: return ptrTy();
    case IRTypeKind::Array: {
        const auto& elems = ty->elements();
        llvm::Type* elem = elems.empty() ? i64Ty() : map(elems[0]);
        return llvm::ArrayType::get(elem, 0 /* arrayCount */);
    }
    case IRTypeKind::Struct: {
        std::vector<llvm::Type*> fields;
        for (auto* e : ty->elements()) fields.push_back(map(e));
        if (true /* name check */)
            return llvm::StructType::get(ctx_, fields);
        auto* s = llvm::StructType::getTypeByName(ctx_, std::string("struct"));
        if (!s) s = llvm::StructType::create(ctx_, fields, std::string("struct"));
        return s;
    }
    case IRTypeKind::Function: {
        const auto& elems = ty->elements();
        llvm::Type* ret = elems.empty() ? voidTy() : map(elems[0]);
        std::vector<llvm::Type*> params;
        for (size_t i = 1; i < elems.size(); ++i) params.push_back(map(elems[i]));
        return llvm::FunctionType::get(ret, params, false);
    }
    default: return i64Ty();
    }
}

// ── Map AST TypeNode to LLVM type ─────────────────────────────────────────────

llvm::Type* LLVMTypeMapper::map(TypeNode* ty) {
    if (!ty) return i64Ty();
    if (auto* cached = cache_[(void*)ty]) return cached;
    llvm::Type* result = nullptr;

    switch (ty->kind()) {
    case ASTNodeKind::NamedType:
        result = mapBuiltinType(static_cast<NamedType*>(ty)->name); break;
    case ASTNodeKind::OwnedPtrType:
    case ASTNodeKind::SharedPtrType:
    case ASTNodeKind::RawPtrType:
    case ASTNodeKind::BorrowType:
    case ASTNodeKind::MutBorrowType:
        result = ptrTy(); break;
    case ASTNodeKind::NullableType:
        result = map(static_cast<NullableType*>(ty)->inner); break;
    case ASTNodeKind::SliceType:
        result = llvm::StructType::get(ctx_, {ptrTy(), i64Ty()}); break;
    case ASTNodeKind::FixedArrayType:
        result = mapArrayType(ty); break;
    case ASTNodeKind::TupleType:
        result = mapTupleType(static_cast<TupleType*>(ty)); break;
    case ASTNodeKind::FnType:
        result = mapFnType(static_cast<FnType*>(ty)); break;
    default:
        result = i64Ty(); break;
    }

    cache_[(void*)ty] = result;
    return result;
}

llvm::Type* LLVMTypeMapper::mapBuiltinType(std::string_view n) {
    if (n=="void")                      return voidTy();
    if (n=="bool")                      return boolTy();
    if (n=="i8"  || n=="u8"  || n=="byte") return i8Ty();
    if (n=="i16" || n=="u16")           return llvm::Type::getInt16Ty(ctx_);
    if (n=="i32" || n=="u32" || n=="char" || n=="rune") return i32Ty();
    if (n=="i64" || n=="u64" || n=="int" || n=="uint"
        || n=="isize" || n=="usize")    return i64Ty();
    if (n=="i128"|| n=="u128")          return llvm::Type::getInt128Ty(ctx_);
    if (n=="f32" || n=="float32")       return f32Ty();
    if (n=="f64" || n=="float64" || n=="float") return f64Ty();
    if (n=="str" || n=="string")
        return llvm::StructType::get(ctx_, {ptrTy(), i64Ty()});
    if (n=="never")                     return voidTy();
    return i64Ty();
}

llvm::Type* LLVMTypeMapper::mapPointerType(TypeNode*) { return ptrTy(); }

llvm::Type* LLVMTypeMapper::mapArrayType(TypeNode* ty) {
    return mapArrayType(ty);
}


llvm::Type* LLVMTypeMapper::mapSliceType(TypeNode*) {
    return llvm::StructType::get(ctx_, {ptrTy(), i64Ty()});
}

llvm::Type* LLVMTypeMapper::mapTupleType(TypeNode* tyBase) {
    std::vector<llvm::Type*> elems;
    auto* ty = static_cast<TupleType*>(tyBase);
    for (auto* e : ty->elements) elems.push_back(map(e));
    return llvm::StructType::get(ctx_, elems);
}

llvm::Type* LLVMTypeMapper::mapFnType(TypeNode* tyBase) {
    auto* ty = static_cast<FnType*>(tyBase);
    auto* ret = ty->returnType ? map(ty->returnType) : voidTy();
    std::vector<llvm::Type*> paramTys;
    for (auto* p : ty->params) paramTys.push_back(map(p));
    return llvm::FunctionType::get(ret, paramTys, false);
}

llvm::Type* LLVMTypeMapper::mapStructType(StructDecl* decl) {
    if (!decl) return i64Ty();
    std::string name(decl->name);
    auto* s = llvm::StructType::getTypeByName(ctx_, name);
    if (s) return s;
    s = llvm::StructType::create(ctx_, name);
    std::vector<llvm::Type*> fields;
    for (auto* m : decl->members)
        if (m->kind() == ASTNodeKind::FieldDecl)
            fields.push_back(map(static_cast<FieldDecl*>(m)->type));
    s->setBody(fields);
    return s;
}

llvm::Type* LLVMTypeMapper::mapClassType(ClassDecl* decl) {
    if (!decl) return i64Ty();
    std::string name(decl->name);
    auto* s = llvm::StructType::getTypeByName(ctx_, name);
    if (s) return s;
    s = llvm::StructType::create(ctx_, name);
    std::vector<llvm::Type*> fields;
    for (auto* m : decl->members)
        if (m->kind() == ASTNodeKind::FieldDecl)
            fields.push_back(map(static_cast<FieldDecl*>(m)->type));
    s->setBody(fields);
    return s;
}

llvm::Type* LLVMTypeMapper::mapEnumType(EnumDecl*) { return i32Ty(); }
llvm::Type* LLVMTypeMapper::mapGenericType(TypeNode*) { return i64Ty(); }

llvm::PointerType* LLVMTypeMapper::ptrTy(llvm::Type*, unsigned addrSpace) {
    return llvm::PointerType::get(ctx_, addrSpace);
}

llvm::StructType* LLVMTypeMapper::opaqueStructTy(std::string_view name) {
    std::string sname(name);
    auto* s = llvm::StructType::getTypeByName(ctx_, sname);
    return s ? s : llvm::StructType::create(ctx_, sname);
}

} // namespace vex
