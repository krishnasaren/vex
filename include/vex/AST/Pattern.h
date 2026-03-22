#pragma once
// ============================================================================
// vex/AST/Pattern.h
// All pattern AST nodes.
// Covers Chapter 17 RULE C5 — complete pattern list.
// ============================================================================

#include "vex/AST/ASTNode.h"
#include <string_view>
#include <vector>

namespace vex {

class ExprNode;
class TypeNode;

// ── WildcardPattern ───────────────────────────────────────────────────────────
// _

class WildcardPattern final : public PatternNode {
public:
    WildcardPattern(SourceLocation loc)
        : PatternNode(ASTNodeKind::WildcardPattern, loc) {}
};

// ── BindingPattern ────────────────────────────────────────────────────────────
// name — binds value to a new variable

class BindingPattern final : public PatternNode {
public:
    BindingPattern(SourceLocation loc, std::string_view name)
        : PatternNode(ASTNodeKind::BindingPattern, loc), name(name) {}

    std::string_view name;
    bool             isMutable = false; // var name
};

// ── LiteralPattern ────────────────────────────────────────────────────────────
// 42  "hello"  true  3.14  'A'  null

class LiteralPattern final : public PatternNode {
public:
    LiteralPattern(SourceLocation loc, ExprNode* lit)
        : PatternNode(ASTNodeKind::LiteralPattern, loc), literal(lit) {}

    ExprNode* literal;
};

// ── RangePattern ─────────────────────────────────────────────────────────────
// 1..10  1..=10

class RangePattern final : public PatternNode {
public:
    RangePattern(SourceLocation loc, ExprNode* lo, ExprNode* hi, bool inclusive)
        : PatternNode(ASTNodeKind::RangePattern, loc)
        , lo(lo), hi(hi), inclusive(inclusive) {}

    ExprNode* lo;
    ExprNode* hi;
    bool      inclusive;
};

// ── OrPattern ────────────────────────────────────────────────────────────────
// 1 | 2 | 3

class OrPattern final : public PatternNode {
public:
    OrPattern(SourceLocation loc)
        : PatternNode(ASTNodeKind::OrPattern, loc) {}

    std::vector<PatternNode*> alternatives;
};

// ── TuplePattern ─────────────────────────────────────────────────────────────
// (a, b)  or  (_, b, c)

class TuplePattern final : public PatternNode {
public:
    TuplePattern(SourceLocation loc)
        : PatternNode(ASTNodeKind::TuplePattern, loc) {}

    std::vector<PatternNode*> elements;
};

// ── StructPattern ─────────────────────────────────────────────────────────────
// { x, y }   { x: 0, y }   { x, .. }

struct StructFieldPattern {
    std::string_view  fieldName;
    PatternNode*      pattern   = nullptr;  // nullptr = bind to same name
    bool              isIgnoreRest = false; // ..
    SourceLocation    loc;
};

class StructPattern final : public PatternNode {
public:
    StructPattern(SourceLocation loc)
        : PatternNode(ASTNodeKind::StructPattern, loc) {}

    std::string_view                  typeName;  // optional type constraint
    std::vector<StructFieldPattern>   fields;
    bool                              hasIgnoreRest = false; // { x, .. }
};

// ── EnumPattern ───────────────────────────────────────────────────────────────
// VariantName(a, b)  or  VariantName(radius: r)

struct EnumFieldPattern {
    std::string_view  name;     // named field: radius: r
    PatternNode*      pattern;
    bool              isNamed = false;
    SourceLocation    loc;
};

class EnumPattern final : public PatternNode {
public:
    EnumPattern(SourceLocation loc)
        : PatternNode(ASTNodeKind::EnumPattern, loc) {}

    std::string_view                name;      // variant name
    std::string_view                enumName;  // optional: Color.Red → "Color"
    std::vector<EnumFieldPattern>   fields;
};

// ── TypePattern ───────────────────────────────────────────────────────────────
// as TypeName binding — matches if runtime type is TypeName (for `any`)

class TypePattern final : public PatternNode {
public:
    TypePattern(SourceLocation loc, TypeNode* type, std::string_view bindName)
        : PatternNode(ASTNodeKind::TypePattern, loc)
        , type(type), bindName(bindName) {}

    TypeNode*        type;
    std::string_view bindName;  // empty if no binding
};

// ── GuardPattern ─────────────────────────────────────────────────────────────
// pattern if condition

class GuardPattern final : public PatternNode {
public:
    GuardPattern(SourceLocation loc, PatternNode* inner, ExprNode* guard)
        : PatternNode(ASTNodeKind::GuardPattern, loc)
        , inner(inner), guard(guard) {}

    PatternNode* inner;
    ExprNode*    guard;
};

} // namespace vex