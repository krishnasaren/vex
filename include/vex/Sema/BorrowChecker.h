#pragma once
// include/vex/Sema/BorrowChecker.h




#pragma once
// ============================================================================
// vex/Sema/BorrowChecker.h
// Ownership and borrow checker (NLL-style static analysis).
// ============================================================================

#include "vex/Sema/SemaContext.h"
#include "vex/AST/ASTVisitor.h"
#include <unordered_set>
#include <string>

namespace vex {

class BorrowChecker : public ASTVisitor {
public:
    explicit BorrowChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    void visitFnDecl(FnDecl* n)           override;
    void visitBlockStmt(BlockStmt* n)     override;
    void visitVarDeclStmt(VarDeclStmt* n) override;
    void visitAssignExpr(AssignExpr* n)   override;
    void visitCallExpr(CallExpr* n)       override;
    void visitReturnStmt(ReturnStmt* n)   override;

private:
    SemaContext& ctx_;

    // Track moved variables by name
    std::unordered_set<std::string> movedVars_;
    // Track variables with active borrows
    std::unordered_set<std::string> borrowedVars_;
    std::unordered_set<std::string> mutBorrowedVars_;

    void markMoved(std::string_view name, SourceLocation loc);
    void checkNotMoved(std::string_view name, SourceLocation loc);
    void checkBorrowCompatibility(std::string_view name,
                                   bool isMutable,
                                   SourceLocation loc);
    void clearScope();
};

} // namespace vex
