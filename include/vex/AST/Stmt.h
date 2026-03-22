#pragma once
// ============================================================================
// vex/AST/Stmt.h
// All statement AST nodes.
// Covers every statement form from Chapter 17 (Control Flow Rules).
// ============================================================================

#include "vex/AST/ASTNode.h"
#include <string_view>
#include <vector>

namespace vex {

class ExprNode;
class TypeNode;
class PatternNode;
class DeclNode;

// ── BlockStmt ─────────────────────────────────────────────────────────────────
// { stmt; stmt; expr }
// The optional trailing expression is the block's value (RULE F4).

class BlockStmt final : public StmtNode {
public:
    BlockStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::BlockStmt, loc) {}

    std::vector<StmtNode*> stmts;
    ExprNode*              trailingExpr = nullptr; // last expression value
    std::string_view       label;                  // optional loop label
};

// ── VarDeclStmt ───────────────────────────────────────────────────────────────
// var x := 42
// let x: int = 42
// var a, b := expr1, expr2

class VarDeclStmt final : public StmtNode {
public:
    VarDeclStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::VarDeclStmt, loc) {}

    struct Binding {
        std::string_view name;
        TypeNode*        type        = nullptr;
        ExprNode*        initializer = nullptr;
        bool             isDiscard   = false;  // _ discard
        SourceLocation   loc;
    };

    std::vector<Binding> bindings;    // multiple: var a, b := e1, e2
    bool                 isMutable   = true;    // var = mutable
    bool                 isLet       = false;   // let = immutable
    bool                 isConst     = false;   // const = comptime
};

// ── ExprStmt ──────────────────────────────────────────────────────────────────
// expression as statement (for side effects)

class ExprStmt final : public StmtNode {
public:
    ExprStmt(SourceLocation loc, ExprNode* expr)
        : StmtNode(ASTNodeKind::ExprStmt, loc), expr(expr) {}

    ExprNode* expr;
};

// ── IfStmt ────────────────────────────────────────────────────────────────────
// if cond { } elif cond { } else { }
// if let pattern = expr { }

class IfStmt final : public StmtNode {
public:
    IfStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::IfStmt, loc) {}

    struct Branch {
        ExprNode*    condition    = nullptr;
        PatternNode* ifLetPattern = nullptr;  // for if let
        ExprNode*    ifLetExpr    = nullptr;
        BlockStmt*   body         = nullptr;
        SourceLocation loc;
    };

    std::vector<Branch> branches;    // if + all elif branches
    BlockStmt*          elseBranch  = nullptr;
};

// ── WhileStmt ─────────────────────────────────────────────────────────────────
// while cond { }
// while let pattern = expr { }

class WhileStmt final : public StmtNode {
public:
    WhileStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::WhileStmt, loc) {}

    ExprNode*    condition    = nullptr;
    PatternNode* whileLetPat  = nullptr;  // for while let
    ExprNode*    whileLetExpr = nullptr;
    BlockStmt*   body         = nullptr;
    std::string_view label;
};

// ── DoWhileStmt ───────────────────────────────────────────────────────────────
// do { } while cond

class DoWhileStmt final : public StmtNode {
public:
    DoWhileStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::DoWhileStmt, loc) {}

    BlockStmt* body      = nullptr;
    ExprNode*  condition = nullptr;
    std::string_view label;
};

// ── ForStmt ───────────────────────────────────────────────────────────────────
// for x in expr { }
// for i, x in expr { }
// for k, v in map { }
// for i in a..b { }
// for i in a..b step n { }

class ForStmt final : public StmtNode {
public:
    ForStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::ForStmt, loc) {}

    std::string_view valueVar;   // x in "for x in expr"
    std::string_view indexVar;   // i in "for i, x in expr" (empty if none)
    ExprNode*        iterable   = nullptr;
    ExprNode*        stepExpr   = nullptr;  // for ... step n
    BlockStmt*       body       = nullptr;
    std::string_view label;
    bool             isDestructuring = false; // for (a, b) in expr
    PatternNode*     pattern    = nullptr;    // pattern-based for
};

// ── C-style for loop ──────────────────────────────────────────────────────────
// for var i:=0; i<10; i++ { }

class ForCStmt final : public StmtNode {
public:
    ForCStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::ForCStmt, loc) {}

    StmtNode*  init      = nullptr;  // var i := 0
    ExprNode*  condition = nullptr;  // i < 10
    ExprNode*  step      = nullptr;  // i++
    BlockStmt* body      = nullptr;
    std::string_view label;
};

// ── LoopStmt ──────────────────────────────────────────────────────────────────
// loop { } — infinite loop, must contain stop

class LoopStmt final : public StmtNode {
public:
    LoopStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::LoopStmt, loc) {}

    BlockStmt*       body  = nullptr;
    std::string_view label;
};

// ── MatchStmt ─────────────────────────────────────────────────────────────────

class MatchArm;  // defined in Expr.h, reused here

class MatchStmt final : public StmtNode {
public:
    MatchStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::MatchStmt, loc) {}

    ExprNode*               subject = nullptr;
    std::vector<MatchArm*>  arms;
};

// ── ReturnStmt ────────────────────────────────────────────────────────────────
// return expr  or  return  (void)

class ReturnStmt final : public StmtNode {
public:
    ReturnStmt(SourceLocation loc, ExprNode* value = nullptr)
        : StmtNode(ASTNodeKind::ReturnStmt, loc), value(value) {}

    ExprNode* value; // nullptr = void return
};

// ── SkipStmt (continue) ───────────────────────────────────────────────────────
// skip  or  skip label

class SkipStmt final : public StmtNode {
public:
    SkipStmt(SourceLocation loc, std::string_view label = {})
        : StmtNode(ASTNodeKind::SkipStmt, loc), label(label) {}

    std::string_view label;
};

// ── StopStmt (break) ──────────────────────────────────────────────────────────
// stop  or  stop label  or  stop expr (break with value)

class StopStmt final : public StmtNode {
public:
    StopStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::StopStmt, loc) {}

    std::string_view label;
    ExprNode*        value = nullptr; // optional break value
};

// ── DeferStmt ─────────────────────────────────────────────────────────────────
// defer expr  — runs at scope exit (RULE C12)

class DeferStmt final : public StmtNode {
public:
    DeferStmt(SourceLocation loc, ExprNode* expr)
        : StmtNode(ASTNodeKind::DeferStmt, loc), expr(expr) {}

    ExprNode* expr;
};

// ── UseLetStmt ────────────────────────────────────────────────────────────────
// use let x := resource { body }  — calls x.close() at end (RULE C13)

class UseLetStmt final : public StmtNode {
public:
    UseLetStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::UseLetStmt, loc) {}

    std::string_view  name;
    TypeNode*         type        = nullptr;
    ExprNode*         resource    = nullptr;
    BlockStmt*        body        = nullptr;
};

// ── TryCatchStmt ─────────────────────────────────────────────────────────────
// try { } catch e { }  — catches panics only (RULE ERR4)

class TryCatchStmt final : public StmtNode {
public:
    TryCatchStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::TryCatchStmt, loc) {}

    BlockStmt*       tryBlock   = nullptr;
    std::string_view catchVar;            // the PanicInfo variable name
    BlockStmt*       catchBlock = nullptr;
};

// ── SelectStmt ────────────────────────────────────────────────────────────────
// select { recv case ... => {} timeout(dur) => {} _ => {} }
// Chapter 24, RULE CH3

struct SelectArm {
    enum class Kind { Recv, Timeout, Default };
    Kind             kind       = Kind::Recv;
    std::string_view bindVar;             // for recv: varName := chan.recv()
    ExprNode*        chanExpr   = nullptr; // channel expression
    ExprNode*        timeoutDur = nullptr; // for timeout(dur)
    BlockStmt*       body       = nullptr;
    SourceLocation   loc;
};

class SelectStmt final : public StmtNode {
public:
    SelectStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::SelectStmt, loc) {}

    std::vector<SelectArm> arms;
};

// ── UnsafeStmt ────────────────────────────────────────────────────────────────
// unsafe { }

class UnsafeStmt final : public StmtNode {
public:
    UnsafeStmt(SourceLocation loc, BlockStmt* body)
        : StmtNode(ASTNodeKind::UnsafeStmt, loc), body(body) {}

    BlockStmt* body;
};

// ── AsmStmt ───────────────────────────────────────────────────────────────────
// asm x86_64 { ... }  or  asm c { }  (RULE ASM1-ASM6)

class AsmStmt final : public StmtNode {
public:
    AsmStmt(SourceLocation loc)
        : StmtNode(ASTNodeKind::AsmStmt, loc) {}

    std::string_view            dialect;    // "x86_64" "arm64" "c" "cpp"
    std::string_view            code;
    std::vector<std::pair<std::string_view, std::string_view>> inputs;   // ("reg", varName)
    std::vector<std::pair<std::string_view, std::string_view>> outputs;  // ("reg", varName)
};

// ── LabeledStmt ───────────────────────────────────────────────────────────────
// label: for/while/loop ...

class LabeledStmt final : public StmtNode {
public:
    LabeledStmt(SourceLocation loc, std::string_view label, StmtNode* inner)
        : StmtNode(ASTNodeKind::LabeledStmt, loc)
        , label(label), inner(inner) {}

    std::string_view label;
    StmtNode*        inner;
};

} // namespace vex