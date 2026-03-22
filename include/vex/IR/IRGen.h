#pragma once
// ============================================================================
// vex/IR/IRGen.h
// Translates type-checked AST into VexIR.
// ============================================================================

#include "vex/IR/VexIR.h"
#include "vex/IR/LoweringContext.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Stmt.h"
#include "vex/AST/Expr.h"
#include "vex/Core/DiagnosticEngine.h"
#include <memory>

namespace vex {

class IRGen {
public:
    IRGen(DiagnosticEngine& diags)
        : diags_(diags) {}

    // Entry point: generate IR for a full module
    std::unique_ptr<IRModule> generate(ModuleDecl* mod);

private:
    DiagnosticEngine& diags_;
    LoweringContext   ctx_;

    // ── Top-level declarations ────────────────────────────────────────────────
    void lowerTopLevelDecl(DeclNode* decl);
    void lowerFnDecl(FnDecl* fn);
    void lowerStructDecl(StructDecl* decl);
    void lowerClassDecl(ClassDecl* decl);
    void lowerEnumDecl(EnumDecl* decl);
    void lowerImplDecl(ImplDecl* decl);
    void lowerGlobalVarDecl(VarDecl* decl);
    void lowerConstDecl(ConstDecl* decl);

    // ── Statements ───────────────────────────────────────────────────────────
    void lowerBlock(BlockStmt* block);
    void lowerStmt(StmtNode* stmt);
    void lowerVarDeclStmt(VarDeclStmt* stmt);
    void lowerIfStmt(IfStmt* stmt);
    void lowerWhileStmt(WhileStmt* stmt);
    void lowerForStmt(ForStmt* stmt);
    void lowerLoopStmt(LoopStmt* stmt);
    void lowerReturnStmt(ReturnStmt* stmt);
    void lowerDeferStmt(DeferStmt* stmt);
    void lowerMatchStmt(MatchStmt* stmt);

    // ── Expressions ──────────────────────────────────────────────────────────
    IRValue* lowerExpr(ExprNode* expr);
};

} // namespace vex