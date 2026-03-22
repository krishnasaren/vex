#pragma once
// ============================================================================
// vex/AST/Expr.h
// All expression AST nodes.
// Covers every expression form from Chapter 2 (operators) and Chapter 17.
// ============================================================================

#include "vex/AST/ASTNode.h"
#include "vex/Lexer/Token.h"
#include <string_view>
#include <vector>
#include <cstdint>

namespace vex {

class TypeNode;
class PatternNode;
class StmtNode;
class BlockStmt;

// ── Literal expressions ───────────────────────────────────────────────────────

class IntLitExpr final : public ExprNode {
public:
    IntLitExpr(SourceLocation loc, uint64_t val, IntSuffix suffix, uint8_t base)
        : ExprNode(ASTNodeKind::IntLitExpr, loc)
        , value(val), suffix(suffix), base(base) {}

    uint64_t  value;
    IntSuffix suffix;
    uint8_t   base;    // 2, 8, 10, 16
};

class FloatLitExpr final : public ExprNode {
public:
    FloatLitExpr(SourceLocation loc, double val, FloatSuffix suffix)
        : ExprNode(ASTNodeKind::FloatLitExpr, loc)
        , value(val), suffix(suffix) {}

    double      value;
    FloatSuffix suffix;
};

class StringLitExpr final : public ExprNode {
public:
    StringLitExpr(SourceLocation loc, std::string_view val)
        : ExprNode(ASTNodeKind::StringLitExpr, loc), value(val) {}

    std::string_view value;  // processed (escape sequences applied)
    std::string_view raw;    // original source text
};

class CharLitExpr final : public ExprNode {
public:
    CharLitExpr(SourceLocation loc, uint32_t codepoint)
        : ExprNode(ASTNodeKind::CharLitExpr, loc), codepoint(codepoint) {}

    uint32_t codepoint; // Unicode codepoint value
};

class BoolLitExpr final : public ExprNode {
public:
    BoolLitExpr(SourceLocation loc, bool val)
        : ExprNode(ASTNodeKind::BoolLitExpr, loc), value(val) {}

    bool value;
};

class NullLitExpr final : public ExprNode {
public:
    NullLitExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::NullLitExpr, loc) {}
};

// ── FmtStringExpr ─────────────────────────────────────────────────────────────
// f"Hello, {name}! You have {count:d} messages."
// Broken into alternating text and expression segments.

struct FmtSegment {
    enum class Kind { Text, Expr };
    Kind             kind;
    std::string_view text;       // for Text segments: literal text
    ExprNode*        expr = nullptr; // for Expr segments: the interpolated expr
    std::string_view fmtSpec;    // optional format specifier after :
    std::string_view convFlag;   // optional conversion flag: !s !r !d
};

class FmtStringExpr final : public ExprNode {
public:
    FmtStringExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::FmtStringExpr, loc) {}

    std::vector<FmtSegment> segments;
    std::string_view        raw; // original f"..." source
};

class MultilineStrExpr final : public ExprNode {
public:
    MultilineStrExpr(SourceLocation loc, std::string_view val)
        : ExprNode(ASTNodeKind::MultilineStrExpr, loc), value(val) {}

    std::string_view value;
};

// ── Identifier expressions ────────────────────────────────────────────────────

class IdentExpr final : public ExprNode {
public:
    IdentExpr(SourceLocation loc, std::string_view name)
        : ExprNode(ASTNodeKind::IdentExpr, loc), name(name) {}

    std::string_view name;
};

class SelfExpr final : public ExprNode {
public:
    SelfExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::SelfExpr, loc) {}
};

class SuperExpr final : public ExprNode {
public:
    SuperExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::SuperExpr, loc) {}
};

class SelfTypeExpr final : public ExprNode {  // Self (capital S)
public:
    SelfTypeExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::SelfTypeExpr, loc) {}
};

// mod.x — access module-level scope (RULE MOD16)
class ModExpr final : public ExprNode {
public:
    ModExpr(SourceLocation loc, std::string_view memberName)
        : ExprNode(ASTNodeKind::ModExpr, loc), memberName(memberName) {}

    std::string_view memberName;
};

// ── Collection literals ───────────────────────────────────────────────────────

class TupleExpr final : public ExprNode {
public:
    TupleExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::TupleExpr, loc) {}

    std::vector<ExprNode*> elements;
};

class ListExpr final : public ExprNode {
public:
    ListExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::ListExpr, loc) {}

    std::vector<ExprNode*> elements;
};

struct MapEntry {
    ExprNode* key;
    ExprNode* value;
    SourceLocation loc;
};

class MapExpr final : public ExprNode {
public:
    MapExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::MapExpr, loc) {}

    std::vector<MapEntry> entries;
};

class SetExpr final : public ExprNode {
public:
    SetExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::SetExpr, loc) {}

    std::vector<ExprNode*> elements;
};

// ── Struct literal ────────────────────────────────────────────────────────────
// TypeName { field1: val1, field2: val2, ..spreadExpr }

struct StructLitField {
    std::string_view name;
    ExprNode*        value;
    SourceLocation   loc;
};

class StructLitExpr final : public ExprNode {
public:
    StructLitExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::StructLitExpr, loc) {}

    ExprNode*                  typeName  = nullptr; // the struct type
    std::vector<StructLitField> fields;
    ExprNode*                  spreadFrom = nullptr; // { ...other, field: val }
};

// ── Lambda expression ─────────────────────────────────────────────────────────
// |params| -> expr   or   |params| -> { block }
// copy |params| -> ...   or   move |params| -> ...

class LambdaExpr final : public ExprNode {
public:
    LambdaExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::LambdaExpr, loc) {}

    struct LambdaParam {
        std::string_view name;
        TypeNode*        type = nullptr; // optional
        SourceLocation   loc;
    };

    std::vector<LambdaParam> params;
    TypeNode*                returnType = nullptr;   // optional
    ExprNode*                bodyExpr   = nullptr;   // single expression
    BlockStmt*               bodyBlock  = nullptr;   // block body
    bool                     isCopyCapture = false;  // copy |...| -> ...
    bool                     isMoveCapture = false;  // move |...| -> ...
    bool                     isAsync       = false;  // async lambda
};

// ── Unary expression ──────────────────────────────────────────────────────────

class UnaryExpr final : public ExprNode {
public:
    UnaryExpr(SourceLocation loc, TokenKind op, ExprNode* operand)
        : ExprNode(ASTNodeKind::UnaryExpr, loc)
        , op(op), operand(operand) {}

    TokenKind op;        // !, ~, -, *, &, ++, --
    ExprNode* operand;
    bool      isPostfix = false; // x++ vs ++x
};

// ── Binary expression ─────────────────────────────────────────────────────────

class BinaryExpr final : public ExprNode {
public:
    BinaryExpr(SourceLocation loc, TokenKind op, ExprNode* left, ExprNode* right)
        : ExprNode(ASTNodeKind::BinaryExpr, loc)
        , op(op), left(left), right(right) {}

    TokenKind op;
    ExprNode* left;
    ExprNode* right;
};

// ── Assignment expression ─────────────────────────────────────────────────────

class AssignExpr final : public ExprNode {
public:
    AssignExpr(SourceLocation loc, TokenKind op, ExprNode* target, ExprNode* value)
        : ExprNode(ASTNodeKind::AssignExpr, loc)
        , op(op), target(target), value(value) {}

    TokenKind op;     // =  :=  +=  -=  etc.
    ExprNode* target;
    ExprNode* value;
};

// ── Call expression ───────────────────────────────────────────────────────────

struct CallArg {
    std::string_view name;     // empty if positional
    ExprNode*        value;
    bool             isSpread; // ...list
    SourceLocation   loc;
};

class CallExpr final : public ExprNode {
public:
    CallExpr(SourceLocation loc, ExprNode* callee)
        : ExprNode(ASTNodeKind::CallExpr, loc), callee(callee) {}

    ExprNode*              callee;
    std::vector<CallArg>   args;
    std::vector<TypeNode*> typeArgs; // explicit type args: f<int>(...)
};

// ── Index expression ──────────────────────────────────────────────────────────

class IndexExpr final : public ExprNode {
public:
    IndexExpr(SourceLocation loc, ExprNode* base, ExprNode* index)
        : ExprNode(ASTNodeKind::IndexExpr, loc), base(base), index(index) {}

    ExprNode* base;
    ExprNode* index;
};

// ── Member access expression ──────────────────────────────────────────────────
// expr.name — instance field, static field (if base is type), or method

class MemberExpr final : public ExprNode {
public:
    MemberExpr(SourceLocation loc, ExprNode* base, std::string_view member)
        : ExprNode(ASTNodeKind::MemberExpr, loc)
        , base(base), member(member) {}

    ExprNode*        base;
    std::string_view member;
};

// ── Module path expression ────────────────────────────────────────────────────
// io::println  or  std::math::sqrt

class ModulePathExpr final : public ExprNode {
public:
    ModulePathExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::ModulePathExpr, loc) {}

    std::vector<std::string_view> segments;
};

// ── Cast expressions ──────────────────────────────────────────────────────────

class CastExpr final : public ExprNode {
public:
    CastExpr(SourceLocation loc, ExprNode* expr, TypeNode* targetType)
        : ExprNode(ASTNodeKind::CastExpr, loc)
        , expr(expr), targetType(targetType) {}

    ExprNode* expr;
    TypeNode* targetType;
};

class SafeCastExpr final : public ExprNode {
public:
    SafeCastExpr(SourceLocation loc, ExprNode* expr, TypeNode* targetType)
        : ExprNode(ASTNodeKind::SafeCastExpr, loc)
        , expr(expr), targetType(targetType) {}

    ExprNode* expr;
    TypeNode* targetType;
};

class TypeCheckExpr final : public ExprNode {
public:
    TypeCheckExpr(SourceLocation loc, ExprNode* expr, TypeNode* checkType)
        : ExprNode(ASTNodeKind::TypeCheckExpr, loc)
        , expr(expr), checkType(checkType) {}

    ExprNode* expr;
    TypeNode* checkType;
};

// ── Conditional expression ────────────────────────────────────────────────────

class IfExpr final : public ExprNode {
public:
    IfExpr(SourceLocation loc)
        : ExprNode(ASTNodeKind::IfExpr, loc) {}

    ExprNode*  condition  = nullptr;
    PatternNode* ifLetPattern = nullptr;  // if let pattern = expr
    ExprNode*  thenBranch = nullptr;
    ExprNode*  elseBranch = nullptr;  // nullptr if no else
};

// ── Match expression ──────────────────────────────────────────────────────────

class MatchArm final : public ASTNode {
public:
    MatchArm(SourceLocation loc)
        : ASTNode(ASTNodeKind::MatchArm, loc) {}

    PatternNode*           pattern   = nullptr;
    ExprNode*              guard     = nullptr;  // optional: if condition
    ExprNode*              bodyExpr  = nullptr;
    BlockStmt*             bodyBlock = nullptr;
};

class MatchExpr final : public ExprNode {
public:
    MatchExpr(SourceLocation loc, ExprNode* subject)
        : ExprNode(ASTNodeKind::MatchExpr, loc), subject(subject) {}

    ExprNode*              subject;
    std::vector<MatchArm*> arms;
};

// ── Special operator expressions ─────────────────────────────────────────────

class NullCoalesceExpr final : public ExprNode {
public:
    NullCoalesceExpr(SourceLocation loc, ExprNode* left, ExprNode* right)
        : ExprNode(ASTNodeKind::NullCoalesceExpr, loc)
        , left(left), right(right) {}

    ExprNode* left;
    ExprNode* right;
};

class SafeCallExpr final : public ExprNode {
public:
    SafeCallExpr(SourceLocation loc, ExprNode* base, std::string_view member)
        : ExprNode(ASTNodeKind::SafeCallExpr, loc)
        , base(base), member(member) {}

    ExprNode*              base;
    std::string_view       member;
    std::vector<CallArg>   args;
};

class ErrorPropExpr final : public ExprNode {
public:
    ErrorPropExpr(SourceLocation loc, ExprNode* inner)
        : ExprNode(ASTNodeKind::ErrorPropExpr, loc), inner(inner) {}

    ExprNode* inner;
};

class PipeExpr final : public ExprNode {
public:
    PipeExpr(SourceLocation loc, ExprNode* left, ExprNode* right)
        : ExprNode(ASTNodeKind::PipeExpr, loc)
        , left(left), right(right) {}

    ExprNode* left;
    ExprNode* right;
};

class RangeExpr final : public ExprNode {
public:
    RangeExpr(SourceLocation loc, ExprNode* start, ExprNode* end, bool inclusive)
        : ExprNode(ASTNodeKind::RangeExpr, loc)
        , start(start), end(end), inclusive(inclusive) {}

    ExprNode* start;
    ExprNode* end;
    bool      inclusive; // true = ..=, false = ..
};

class BlockExpr final : public ExprNode {
public:
    BlockExpr(SourceLocation loc, BlockStmt* block)
        : ExprNode(ASTNodeKind::BlockExpr, loc), block(block) {}

    BlockStmt* block;
};

class ComptimeExpr final : public ExprNode {
public:
    ComptimeExpr(SourceLocation loc, ExprNode* inner)
        : ExprNode(ASTNodeKind::ComptimeExpr, loc), inner(inner) {}

    ExprNode* inner;
};

class UnsafeExpr final : public ExprNode {
public:
    UnsafeExpr(SourceLocation loc, ExprNode* inner)
        : ExprNode(ASTNodeKind::UnsafeExpr, loc), inner(inner) {}

    ExprNode* inner;
};

// ── Generic instantiation expression ─────────────────────────────────────────
// Box<int>(42)  or  maxOf<int>(3, 5)

class GenericInstExpr final : public ExprNode {
public:
    GenericInstExpr(SourceLocation loc, ExprNode* base)
        : ExprNode(ASTNodeKind::GenericInstExpr, loc), base(base) {}

    ExprNode*              base;
    std::vector<TypeNode*> typeArgs;
    std::vector<CallArg>   args;      // constructor/call args after type args
};

} // namespace vex