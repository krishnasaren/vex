#pragma once
// ============================================================================
// vex/AST/Type.h
// All type AST nodes.
// Covers every type form from Chapter 1 symbol table and Chapter 6.
// ============================================================================

#include "vex/AST/ASTNode.h"
#include <string_view>
#include <vector>

namespace vex {

class ExprNode;

// ── NamedType ─────────────────────────────────────────────────────────────────
// int  str  bool  float  char  byte  MyStruct  etc.

class NamedType final : public TypeNode {
public:
    NamedType(SourceLocation loc)
        : TypeNode(ASTNodeKind::NamedType, loc) {}

    std::string_view name;
};

// ── QualifiedType ─────────────────────────────────────────────────────────────
// module::Type  or  mod1::mod2::Type

class QualifiedType final : public TypeNode {
public:
    QualifiedType(SourceLocation loc)
        : TypeNode(ASTNodeKind::QualifiedType, loc) {}

    std::vector<std::string_view> segments; // ["std", "collections", "Queue"]
};

// ── NullableType ─────────────────────────────────────────────────────────────
// T?

class NullableType final : public TypeNode {
public:
    NullableType(SourceLocation loc, TypeNode* inner)
        : TypeNode(ASTNodeKind::NullableType, loc), inner(inner) {}

    TypeNode* inner;
};

// ── OwnedPtrType ─────────────────────────────────────────────────────────────
// ^T — owned heap pointer

class OwnedPtrType final : public TypeNode {
public:
    OwnedPtrType(SourceLocation loc, TypeNode* pointee)
        : TypeNode(ASTNodeKind::OwnedPtrType, loc), pointee(pointee) {}

    TypeNode* pointee;
};

// ── SharedPtrType ─────────────────────────────────────────────────────────────
// ~T — ARC shared pointer

class SharedPtrType final : public TypeNode {
public:
    SharedPtrType(SourceLocation loc, TypeNode* pointee)
        : TypeNode(ASTNodeKind::SharedPtrType, loc), pointee(pointee) {}

    TypeNode* pointee;
};

// ── BorrowType ────────────────────────────────────────────────────────────────
// &T — shared borrow reference

class BorrowType final : public TypeNode {
public:
    BorrowType(SourceLocation loc, TypeNode* pointee)
        : TypeNode(ASTNodeKind::BorrowType, loc), pointee(pointee) {}

    TypeNode*        pointee;
    std::string_view lifetime; // optional: 'a  'static
};

// ── MutBorrowType ─────────────────────────────────────────────────────────────
// &mut T — mutable borrow reference

class MutBorrowType final : public TypeNode {
public:
    MutBorrowType(SourceLocation loc, TypeNode* pointee)
        : TypeNode(ASTNodeKind::MutBorrowType, loc), pointee(pointee) {}

    TypeNode*        pointee;
    std::string_view lifetime;
};

// ── RawPtrType ────────────────────────────────────────────────────────────────
// *T — raw pointer (unsafe)

class RawPtrType final : public TypeNode {
public:
    RawPtrType(SourceLocation loc, TypeNode* pointee)
        : TypeNode(ASTNodeKind::RawPtrType, loc), pointee(pointee) {}

    TypeNode* pointee;
};

// ── SliceType ─────────────────────────────────────────────────────────────────
// []T — dynamic list

class SliceType final : public TypeNode {
public:
    SliceType(SourceLocation loc, TypeNode* elem)
        : TypeNode(ASTNodeKind::SliceType, loc), elem(elem) {}

    TypeNode* elem;
};

// ── FixedArrayType ────────────────────────────────────────────────────────────
// [N]T — fixed-size array, N is comptime integer

class FixedArrayType final : public TypeNode {
public:
    FixedArrayType(SourceLocation loc, ExprNode* size, TypeNode* elem)
        : TypeNode(ASTNodeKind::FixedArrayType, loc), size(size), elem(elem) {}

    ExprNode* size;   // comptime integer expression
    TypeNode* elem;
};

// ── MapType ───────────────────────────────────────────────────────────────────
// [K:V] — hash map

class MapType final : public TypeNode {
public:
    MapType(SourceLocation loc, TypeNode* key, TypeNode* val)
        : TypeNode(ASTNodeKind::MapType, loc), key(key), val(val) {}

    TypeNode* key;
    TypeNode* val;
};

// ── SetType ───────────────────────────────────────────────────────────────────
// {}T — hash set

class SetType final : public TypeNode {
public:
    SetType(SourceLocation loc, TypeNode* elem)
        : TypeNode(ASTNodeKind::SetType, loc), elem(elem) {}

    TypeNode* elem;
};

// ── TupleType ─────────────────────────────────────────────────────────────────
// (A, B, C)

class TupleType final : public TypeNode {
public:
    TupleType(SourceLocation loc)
        : TypeNode(ASTNodeKind::TupleType, loc) {}

    std::vector<TypeNode*> elements;
};

// ── FnType ────────────────────────────────────────────────────────────────────
// fn(A, B) -> R  or  fn(A, B)

class FnType final : public TypeNode {
public:
    FnType(SourceLocation loc)
        : TypeNode(ASTNodeKind::FnType, loc) {}

    std::vector<TypeNode*> params;
    TypeNode*              returnType = nullptr; // nullptr = void
    bool                   isAsync    = false;
    bool                   isGenerator = false;
};

// ── GenericType ───────────────────────────────────────────────────────────────
// Box<T>  Result<T, E>  []T with explicit type args

class GenericType final : public TypeNode {
public:
    GenericType(SourceLocation loc, TypeNode* base)
        : TypeNode(ASTNodeKind::GenericType, loc), base(base) {}

    TypeNode*              base;
    std::vector<TypeNode*> args;
};

// ── InferType ─────────────────────────────────────────────────────────────────
// _ — let compiler infer this type

class InferType final : public TypeNode {
public:
    InferType(SourceLocation loc)
        : TypeNode(ASTNodeKind::InferType, loc) {}
};

} // namespace vex