#pragma once
// include/vex/Sema/OverflowChecker.h


// ============================================================================
// vex/Sema/OverflowChecker.h  — Detects compile-time integer overflow
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Expr.h"

namespace vex {

class OverflowChecker {
public:
    explicit OverflowChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);
    bool checkExpr(ExprNode* e);

private:
    SemaContext& ctx_;
    bool checkBinary(BinaryExpr* e);
    bool checkUnary(UnaryExpr* e);
    bool checkLiteral(LiteralExpr* e);
    bool checkCast(CastExpr* e);
};

} // namespace vex
