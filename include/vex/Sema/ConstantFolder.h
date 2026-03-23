#pragma once
// include/vex/Sema/ConstantFolder.h


// ============================================================================
// vex/Sema/ConstantFolder.h  — Compile-time constant folding pass
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Expr.h"

namespace vex {

class ConstantFolder {
public:
    explicit ConstantFolder(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    // Attempt to fold an expression; returns nullptr if not foldable.
    ExprNode* fold(ExprNode* e);

private:
    SemaContext& ctx_;
    ExprNode* foldBinary(BinaryExpr* e);
    ExprNode* foldUnary(UnaryExpr* e);
    ExprNode* foldCast(CastExpr* e);
    ExprNode* foldConditional(IfExpr* e);

    bool isConstant(ExprNode* e) const;
};

} // namespace vex
