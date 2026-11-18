#pragma once
// ============================================================================
// vex/Sema/VisibilityChecker.h
// Verifies that all access respects pub/priv/internal/prot rules.
// ============================================================================

#include "vex/Sema/SemaContext.h"
#include "vex/AST/ASTVisitor.h"

namespace vex {

class VisibilityChecker : public ASTVisitor {
public:
    explicit VisibilityChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    void visitIdentExpr(IdentExpr* n)   override;
    void visitMemberExpr(MemberExpr* n) override;
    void visitCallExpr(CallExpr* n)     override;

private:
    SemaContext& ctx_;

    bool isAccessible(Symbol* sym, SourceLocation accessLoc);
    void reportNotAccessible(Symbol* sym, SourceLocation loc);
};

} // namespace vex


