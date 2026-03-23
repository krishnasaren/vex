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
EOF

cat > $BASE/include/vex/Sema/VisibilityChecker.h << 'EOF'
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

