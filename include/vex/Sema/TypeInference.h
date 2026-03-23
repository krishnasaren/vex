#pragma once
// include/vex/Sema/TypeInference.h


// ============================================================================
// vex/Sema/TypeInference.h  — Hindley-Milner style type inference with
// unification for VEX generics and auto-typed locals.
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Type.h"
#include "vex/AST/Expr.h"
#include <vector>
#include <unordered_map>

namespace vex {

// A type variable used during inference
struct TypeVar {
    uint32_t  id;
    TypeNode* bound = nullptr;   // null ⇒ still free
};

class TypeInference {
public:
    explicit TypeInference(SemaContext& ctx) : ctx_(ctx) {}

    // Infer and annotate all untyped expressions in a function body
    bool inferFunction(FnDecl* fn);

    // Infer a single expression (returns inferred type)
    TypeNode* infer(ExprNode* e);

    // Unify two types; emits error on failure
    bool unify(TypeNode* a, TypeNode* b, SourceLocation loc);

    // Create a fresh type variable
    TypeNode* freshVar();

    // Substitute type vars with solved types
    TypeNode* substitute(TypeNode* t);

private:
    SemaContext& ctx_;
    uint32_t     nextVarId_ = 0;
    std::unordered_map<uint32_t, TypeNode*> solution_;

    TypeNode* inferLiteral(LiteralExpr* e);
    TypeNode* inferBinary(BinaryExpr* e);
    TypeNode* inferUnary(UnaryExpr* e);
    TypeNode* inferCall(CallExpr* e);
    TypeNode* inferMember(MemberExpr* e);
    TypeNode* inferBlock(BlockExpr* e);
    TypeNode* inferIf(IfExpr* e);
    TypeNode* inferMatch(MatchExpr* e);
    TypeNode* inferClosure(ClosureExpr* e);
    TypeNode* inferIndex(IndexExpr* e);
    TypeNode* inferCast(CastExpr* e);
    TypeNode* inferRange(RangeExpr* e);
    TypeNode* inferTuple(TupleExpr* e);
    TypeNode* inferArray(ArrayExpr* e);
    TypeNode* inferAwait(AwaitExpr* e);
    TypeNode* inferYield(YieldExpr* e);
    TypeNode* inferTry(TryExpr* e);

    bool occursIn(uint32_t varId, TypeNode* t);
};

} // namespace vex






