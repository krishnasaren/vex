#pragma once
// ============================================================================
// vex/AST/ASTNode.h
// Base class for all AST nodes.
// Every node stores its source location for diagnostics.
// ============================================================================

#include "vex/Core/SourceLocation.h"
#include <cstdint>

namespace vex {

// ── ASTNodeKind ───────────────────────────────────────────────────────────────
// Discriminator for every concrete AST node type.
// Used for fast RTTI without dynamic_cast.

enum class ASTNodeKind : uint16_t {

    // ── Declarations ─────────────────────────────────────────────────────────
    ModuleDecl,
    ImportDecl,
    FnDecl,
    AsyncFnDecl,
    GeneratorFnDecl,
    InitDecl,
    DeinitDecl,
    StructDecl,
    ClassDecl,
    EnumDecl,
    EnumVariantDecl,
    TraitDecl,
    InterfaceDecl,
    ImplDecl,
    FieldDecl,
    StaticFieldDecl,
    PropDecl,
    OperatorDecl,
    TypeAliasDecl,
    DistinctDecl,
    VarDecl,
    ConstDecl,
    BitfieldDecl,
    GenericParamList,
    WhereClause,
    Decorator,
    Param,

    // ── Statements ───────────────────────────────────────────────────────────
    BlockStmt,
    VarDeclStmt,
    ExprStmt,
    IfStmt,
    WhileStmt,
    DoWhileStmt,
    ForStmt,       // for x in expr
    ForRangeStmt,  // for i in a..b step n
    ForCStmt,      // for var i:=e1; c; e2
    LoopStmt,
    MatchStmt,
    ReturnStmt,
    SkipStmt,      // continue
    StopStmt,      // break
    DeferStmt,
    UseLetStmt,
    TryCatchStmt,
    SelectStmt,
    SelectArm,
    UnsafeStmt,
    AsmStmt,
    LabeledStmt,

    // ── Expressions ──────────────────────────────────────────────────────────
    IntLitExpr,
    FloatLitExpr,
    StringLitExpr,
    CharLitExpr,
    BoolLitExpr,
    NullLitExpr,
    FmtStringExpr,
    MultilineStrExpr,
    IdentExpr,
    SelfExpr,
    SuperExpr,
    SelfTypeExpr,   // Self
    ModExpr,        // mod.name
    TupleExpr,
    ListExpr,
    MapExpr,
    SetExpr,
    StructLitExpr,
    LambdaExpr,
    UnaryExpr,
    BinaryExpr,
    AssignExpr,
    CallExpr,
    IndexExpr,
    MemberExpr,
    ModulePathExpr, // a::b::c
    CastExpr,       // x as T
    SafeCastExpr,   // x as? T
    TypeCheckExpr,  // x is T
    IfExpr,
    MatchExpr,
    MatchArm,
    NullCoalesceExpr, // x ?? y
    SafeCallExpr,     // x?.method()
    ErrorPropExpr,    // expr?
    PipeExpr,         // x |> f
    RangeExpr,        // a..b  a..=b
    BlockExpr,        // { stmts; expr }
    ComptimeExpr,
    UnsafeExpr,
    GenericInstExpr,  // Box<int>(42)

    // ── Types ─────────────────────────────────────────────────────────────────
    NamedType,       // int str bool etc.
    QualifiedType,   // module::Type
    NullableType,    // T?
    OwnedPtrType,    // ^T
    SharedPtrType,   // ~T
    BorrowType,      // &T
    MutBorrowType,   // &mut T
    RawPtrType,      // *T
    SliceType,       // []T
    FixedArrayType,  // [N]T
    MapType,         // [K:V]
    SetType,         // {}T
    TupleType,       // (A, B, C)
    FnType,          // fn(A, B) -> R
    GenericType,     // Name<T, U>
    InferType,       // _ (infer)

    // ── Patterns ─────────────────────────────────────────────────────────────
    WildcardPattern,    // _
    BindingPattern,     // name
    LiteralPattern,     // 42  "hello"  true
    RangePattern,       // 1..10  1..=10
    OrPattern,          // a | b | c
    TuplePattern,       // (a, b)
    StructPattern,      // { x, y: 0 }
    EnumPattern,        // Variant(a, b)
    TypePattern,        // as TypeName binding
    GuardPattern,       // pattern if condition

    _COUNT
};

// ── ASTNode ───────────────────────────────────────────────────────────────────

class ASTNode {
public:
    explicit ASTNode(ASTNodeKind kind, SourceLocation loc = SourceLocation::invalid())
        : kind_(kind), loc_(loc) {}

    virtual ~ASTNode() = default;

    ASTNodeKind    kind()     const { return kind_; }
    SourceLocation location() const { return loc_; }

    void setLocation(SourceLocation loc) { loc_ = loc; }

    // Fast non-virtual type check
    bool is(ASTNodeKind k) const { return kind_ == k; }

    template<typename T>
    T* as() { return static_cast<T*>(this); }

    template<typename T>
    const T* as() const { return static_cast<const T*>(this); }

    template<typename T>
    bool isa() const;

private:
    ASTNodeKind    kind_;
    SourceLocation loc_;
};

// ── DeclNode, StmtNode, ExprNode, TypeNode, PatternNode ──────────────────────
// Category base classes — allow ParseResult<DeclNode*> etc.

class DeclNode    : public ASTNode {
public:
    using ASTNode::ASTNode;
};

class StmtNode    : public ASTNode {
public:
    using ASTNode::ASTNode;
};

class ExprNode    : public ASTNode {
public:
    using ASTNode::ASTNode;
    // Type annotation set by the type checker
    // TypeRef* type_ = nullptr;  (added when type system is built)
};

class TypeNode    : public ASTNode {
public:
    using ASTNode::ASTNode;
};

class PatternNode : public ASTNode {
public:
    using ASTNode::ASTNode;
};

} // namespace vex