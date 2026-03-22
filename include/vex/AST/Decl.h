#pragma once
// ============================================================================
// vex/AST/Decl.h
// All declaration AST nodes.
// Covers: import, fn, struct, class, enum, trait, interface, impl,
//         field, prop, operator, type alias, distinct type, var, const.
// ============================================================================

#include "vex/AST/ASTNode.h"
#include "vex/AST/Visibility.h"
#include <string_view>
#include <vector>
#include <cstdint>

namespace vex {

// Forward declarations
class TypeNode;
class ExprNode;
class StmtNode;
class BlockStmt;
class GenericParamList;
class WhereClause;
class Decorator;
class PatternNode;

// ── SelectiveImportItem ───────────────────────────────────────────────────────

struct SelectiveImportItem {
    std::string_view originalName; // name in the module
    std::string_view localName;    // local binding name (same if no 'as')
    bool             isWildcard;   // true for { * }
    SourceLocation   loc;
};

// ── ImportDecl ────────────────────────────────────────────────────────────────
// Represents any of the three import forms:
//   Form 1: import std::io
//   Form 1: import std::io as alias
//   Form 2: import std::io { println, print }
//   Form 2: import std::io { println as p }
//   Form 3: import std::io { * }

class ImportDecl final : public DeclNode {
public:
    ImportDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::ImportDecl, loc) {}

    // Module path segments, e.g. ["std", "io"] for import std::io
    std::vector<std::string_view> pathSegments;

    // Alias (Form 1 with 'as'): import std::io as myIo → "myIo"
    // If no alias: same as last segment of path (e.g. "io")
    std::string_view alias;

    // Selective imports (Form 2): import std::io { println, print }
    std::vector<SelectiveImportItem> selectiveItems;

    // True if this is Form 3: import std::io { * }
    bool isOpenImport = false;

    // True if there is an explicit alias (import X as Y)
    bool hasExplicitAlias = false;

    // Resolved module path (set by ImportResolver after name resolution)
    std::string_view resolvedPath;
};

// ── Param ─────────────────────────────────────────────────────────────────────
// One parameter in a function declaration.

class Param final : public DeclNode {
public:
    Param(SourceLocation loc)
        : DeclNode(ASTNodeKind::Param, loc) {}

    std::string_view name;
    TypeNode*        type          = nullptr;
    ExprNode*        defaultValue  = nullptr;
    bool             isSelf        = false;  // the `self` parameter
    bool             isMutSelf     = false;  // `&mut self` (not supported in VEX — self is always mut)
    bool             isVariadic    = false;  // ...T (must be last param)
    bool             isMutableBorrow = false; // &mut T parameter
};

// ── GenericParamList ──────────────────────────────────────────────────────────

struct GenericParam {
    std::string_view     name;           // T, U, R, Key, Value etc.
    std::vector<TypeNode*> bounds;       // T: Ord + Clone
    ExprNode*            constDefault = nullptr; // for const generics
    bool                 isConst      = false;   // const N: int
    SourceLocation       loc;
};

class GenericParamList final : public ASTNode {
public:
    GenericParamList(SourceLocation loc)
        : ASTNode(ASTNodeKind::GenericParamList, loc) {}

    std::vector<GenericParam> params;
    bool empty() const { return params.empty(); }
};

// ── WhereClause ───────────────────────────────────────────────────────────────

struct WhereConstraint {
    std::string_view      typeName;
    std::vector<TypeNode*> bounds;
    SourceLocation         loc;
};

class WhereClause final : public ASTNode {
public:
    WhereClause(SourceLocation loc)
        : ASTNode(ASTNodeKind::WhereClause, loc) {}

    std::vector<WhereConstraint> constraints;
};

// ── Decorator ─────────────────────────────────────────────────────────────────

class Decorator final : public ASTNode {
public:
    Decorator(SourceLocation loc)
        : ASTNode(ASTNodeKind::Decorator, loc) {}

    std::string_view           name;       // e.g. "inline", "pure", "derive"
    std::vector<ExprNode*>     args;       // arguments: @derive(Clone, Debug)
    std::vector<std::string_view> namedArgs; // for @deprecated("msg")
};

// ── FnDecl ────────────────────────────────────────────────────────────────────

class FnDecl : public DeclNode {
public:
    FnDecl(ASTNodeKind kind, SourceLocation loc)
        : DeclNode(kind, loc) {}

    explicit FnDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::FnDecl, loc) {}

    std::string_view           name;
    Visibility                 vis        = Visibility::Pub;
    GenericParamList*          generics   = nullptr;
    WhereClause*               where_     = nullptr;
    std::vector<Param*>        params;
    TypeNode*                  returnType = nullptr;  // nullptr = void
    BlockStmt*                 body       = nullptr;  // nullptr = abstract/interface
    std::vector<Decorator*>    decorators;

    bool isStatic    = false;  // static fn inside struct/class
    bool isAsync     = false;  // async fn
    bool isGenerator = false;  // fn*
    bool isAbstract  = false;  // no body (trait/interface)
    bool isFinal     = false;  // final fn (no override)
    bool isInline    = false;  // @inline
    bool isPure      = false;  // @pure
    bool isOverride  = false;  // @override
};

// ── InitDecl (constructor) ────────────────────────────────────────────────────

class InitDecl final : public DeclNode {
public:
    InitDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::InitDecl, loc) {}

    Visibility          vis    = Visibility::Pub;
    std::vector<Param*> params;
    BlockStmt*          body   = nullptr;
    bool                isShortForm = false; // init(x, y) auto-assigns fields
    std::vector<Decorator*> decorators;
};

// ── DeinitDecl (destructor) ───────────────────────────────────────────────────

class DeinitDecl final : public DeclNode {
public:
    DeinitDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::DeinitDecl, loc) {}

    BlockStmt* body = nullptr;
};

// ── FieldDecl ─────────────────────────────────────────────────────────────────

class FieldDecl final : public DeclNode {
public:
    FieldDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::FieldDecl, loc) {}

    std::string_view        name;
    TypeNode*               type         = nullptr;
    ExprNode*               defaultValue = nullptr;
    Visibility              vis          = Visibility::Pub;
    bool                    isStatic     = false;
    std::vector<Decorator*> decorators;
};

// ── PropDecl (property) ───────────────────────────────────────────────────────

class PropDecl final : public DeclNode {
public:
    PropDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::PropDecl, loc) {}

    std::string_view name;
    TypeNode*        type     = nullptr;
    ExprNode*        getter   = nullptr;  // get => expr
    BlockStmt*       getterBlock = nullptr;
    Param*           setterParam = nullptr;  // set(v)
    ExprNode*        setter   = nullptr;
    BlockStmt*       setterBlock = nullptr;
    Visibility       vis      = Visibility::Pub;
    bool             isStatic = false;
    bool             isReadOnly = false;  // no setter
};

// ── OperatorDecl ──────────────────────────────────────────────────────────────

class OperatorDecl final : public DeclNode {
public:
    OperatorDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::OperatorDecl, loc) {}

    std::string_view           opName;    // "+", "-", "[]", "str()" etc.
    std::vector<Param*>        params;    // usually one (other: Self) or none (unary)
    TypeNode*                  returnType = nullptr;
    BlockStmt*                 body       = nullptr;
    ExprNode*                  bodyExpr   = nullptr;  // single-expr form
    Visibility                 vis        = Visibility::Pub;
    std::vector<Decorator*>    decorators;
};

// ── StructDecl ────────────────────────────────────────────────────────────────

class StructDecl final : public DeclNode {
public:
    StructDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::StructDecl, loc) {}

    std::string_view           name;
    Visibility                 vis      = Visibility::Pub;
    GenericParamList*          generics = nullptr;
    WhereClause*               where_   = nullptr;
    std::vector<TypeNode*>     impls;    // : Interface, Trait list
    std::vector<DeclNode*>     members; // fields, methods, props, ops, init, deinit
    std::vector<Decorator*>    decorators;
    bool                       isRef    = false; // ref struct
    bool                       isPacked = false; // @packed
    uint32_t                   alignN   = 0;     // @align(N), 0 = default
};

// ── ClassDecl ─────────────────────────────────────────────────────────────────

class ClassDecl final : public DeclNode {
public:
    ClassDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::ClassDecl, loc) {}

    std::string_view           name;
    Visibility                 vis        = Visibility::Pub;
    GenericParamList*          generics   = nullptr;
    WhereClause*               where_     = nullptr;
    TypeNode*                  superClass = nullptr;  // : Parent
    std::vector<TypeNode*>     impls;     // trait/interface list
    std::vector<DeclNode*>     members;
    std::vector<Decorator*>    decorators;
    bool                       isAbstract = false;
    bool                       isSealed   = false;
};

// ── EnumVariantDecl ───────────────────────────────────────────────────────────

class EnumVariantDecl final : public DeclNode {
public:
    EnumVariantDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::EnumVariantDecl, loc) {}

    std::string_view           name;
    std::vector<Param*>        dataFields;   // data variant fields
    ExprNode*                  constValue = nullptr; // for const enum: = expr
    std::vector<Decorator*>    decorators;
};

// ── EnumDecl ──────────────────────────────────────────────────────────────────

class EnumDecl final : public DeclNode {
public:
    EnumDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::EnumDecl, loc) {}

    std::string_view                name;
    Visibility                      vis      = Visibility::Pub;
    GenericParamList*               generics = nullptr;
    TypeNode*                       baseType = nullptr; // for const enum: int
    std::vector<EnumVariantDecl*>   variants;
    std::vector<FnDecl*>            methods;
    std::vector<TypeNode*>          impls;
    std::vector<Decorator*>         decorators;
    bool                            isConst  = false; // const enum
};

// ── TraitDecl ─────────────────────────────────────────────────────────────────

class TraitDecl final : public DeclNode {
public:
    TraitDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::TraitDecl, loc) {}

    std::string_view           name;
    Visibility                 vis      = Visibility::Pub;
    GenericParamList*          generics = nullptr;
    WhereClause*               where_   = nullptr;
    std::vector<TypeNode*>     superTraits; // trait B : A
    std::vector<DeclNode*>     members;     // abstract + default methods
    std::vector<Decorator*>    decorators;
};

// ── InterfaceDecl ─────────────────────────────────────────────────────────────

class InterfaceDecl final : public DeclNode {
public:
    InterfaceDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::InterfaceDecl, loc) {}

    std::string_view           name;
    Visibility                 vis        = Visibility::Pub;
    GenericParamList*          generics   = nullptr;
    std::vector<TypeNode*>     superInterfaces;
    std::vector<FnDecl*>       methods;   // all abstract (no body)
    std::vector<Decorator*>    decorators;
};

// ── ImplDecl ──────────────────────────────────────────────────────────────────

class ImplDecl final : public DeclNode {
public:
    ImplDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::ImplDecl, loc) {}

    TypeNode*           forType  = nullptr; // the type being extended
    TypeNode*           ofTrait  = nullptr; // optional: impl Trait for Type
    GenericParamList*   generics = nullptr;
    WhereClause*        where_   = nullptr;
    std::vector<DeclNode*> members;
};

// ── TypeAliasDecl ─────────────────────────────────────────────────────────────

class TypeAliasDecl final : public DeclNode {
public:
    TypeAliasDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::TypeAliasDecl, loc) {}

    std::string_view    name;
    Visibility          vis      = Visibility::Pub;
    GenericParamList*   generics = nullptr;
    TypeNode*           aliasOf  = nullptr;
};

// ── DistinctDecl ──────────────────────────────────────────────────────────────
// RULE DT1: distinct type NewName = BaseType

class DistinctDecl final : public DeclNode {
public:
    DistinctDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::DistinctDecl, loc) {}

    std::string_view    name;
    Visibility          vis      = Visibility::Pub;
    TypeNode*           baseType = nullptr;
};

// ── VarDecl (module-level) ────────────────────────────────────────────────────

class VarDecl final : public DeclNode {
public:
    VarDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::VarDecl, loc) {}

    std::string_view    name;
    Visibility          vis        = Visibility::Priv; // RULE VIS1: global var = priv
    TypeNode*           type       = nullptr;
    ExprNode*           initializer = nullptr;
    bool                isMutable  = true;   // var = mutable
    bool                isLet      = false;  // let = immutable
    bool                isConst    = false;  // const = comptime
};

// ── ConstDecl (module-level) ──────────────────────────────────────────────────

class ConstDecl final : public DeclNode {
public:
    ConstDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::ConstDecl, loc) {}

    std::string_view    name;
    Visibility          vis         = Visibility::Pub;
    TypeNode*           type        = nullptr;
    ExprNode*           value       = nullptr;
    bool                isComptime  = true;
};

// ── BitfieldDecl ──────────────────────────────────────────────────────────────

struct BitfieldField {
    std::string_view name;
    uint32_t         bits;
    SourceLocation   loc;
};

class BitfieldDecl final : public DeclNode {
public:
    BitfieldDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::BitfieldDecl, loc) {}

    std::string_view         name;
    Visibility               vis      = Visibility::Pub;
    TypeNode*                baseType = nullptr;  // must be integer type
    std::vector<BitfieldField> fields;
};

// ── ModuleDecl ────────────────────────────────────────────────────────────────
// Root node of a parsed .vex file.

class ModuleDecl final : public DeclNode {
public:
    ModuleDecl(SourceLocation loc)
        : DeclNode(ASTNodeKind::ModuleDecl, loc) {}

    std::string_view           name;       // filename without extension
    std::string_view           filePath;   // absolute path
    std::vector<ImportDecl*>   imports;
    std::vector<DeclNode*>     decls;      // all top-level declarations
    FnDecl*                    initFn  = nullptr;    // fn init() if present
    FnDecl*                    deinitFn = nullptr;   // fn deinit() if present
    FnDecl*                    mainFn  = nullptr;    // fn main() if present
};

} // namespace vex