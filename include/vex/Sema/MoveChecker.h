#pragma once
// include/vex/Sema/MoveChecker.h



// ============================================================================
// vex/Sema/MoveChecker.h  — Detects use-after-move and double-move errors
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/ASTVisitor.h"
#include <unordered_set>
#include <string>

namespace vex {

class MoveChecker : public ASTVisitor {
public:
    explicit MoveChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    void visitFnDecl(FnDecl* n)           override;
    void visitBlockStmt(BlockStmt* n)     override;
    void visitVarDeclStmt(VarDeclStmt* n) override;
    void visitAssignExpr(AssignExpr* n)   override;
    void visitCallExpr(CallExpr* n)       override;
    void visitReturnStmt(ReturnStmt* n)   override;

private:
    SemaContext&                    ctx_;
    std::unordered_set<std::string> moved_;
    std::unordered_set<std::string> copyable_;

    bool isCopyable(TypeNode* t) const;
    void checkUse(std::string_view name, SourceLocation loc);
    void markMoved(std::string_view name);
};

} // namespace vex

