#pragma once
// include/vex/Sema/ComptimeEvaluator.h


// ============================================================================
// vex/Sema/ComptimeEvaluator.h  — Evaluates @comptime expressions and blocks
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Decl.h"
#include <variant>
#include <string>
#include <vector>

namespace vex {

// Compile-time value
struct ComptimeValue {
    using IntVal    = int64_t;
    using FloatVal  = double;
    using BoolVal   = bool;
    using StringVal = std::string;
    using ArrayVal  = std::vector<ComptimeValue>;

    std::variant<std::monostate, IntVal, FloatVal, BoolVal, StringVal, ArrayVal> data;

    bool isInt()    const { return std::holds_alternative<IntVal>(data); }
    bool isFloat()  const { return std::holds_alternative<FloatVal>(data); }
    bool isBool()   const { return std::holds_alternative<BoolVal>(data); }
    bool isString() const { return std::holds_alternative<StringVal>(data); }
    bool isArray()  const { return std::holds_alternative<ArrayVal>(data); }
    bool isUnit()   const { return std::holds_alternative<std::monostate>(data); }

    int64_t     asInt()    const { return std::get<IntVal>(data); }
    double      asFloat()  const { return std::get<FloatVal>(data); }
    bool        asBool()   const { return std::get<BoolVal>(data); }
    std::string asString() const { return std::get<StringVal>(data); }
};

class ComptimeEvaluator {
public:
    explicit ComptimeEvaluator(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    // Evaluate a comptime expression
    ComptimeValue evaluate(ExprNode* e);

    bool isComptime(ExprNode* e) const;

private:
    SemaContext& ctx_;
    std::unordered_map<std::string, ComptimeValue> env_;

    ComptimeValue evalLiteral(LiteralExpr* e);
    ComptimeValue evalBinary(BinaryExpr* e);
    ComptimeValue evalUnary(UnaryExpr* e);
    ComptimeValue evalIdent(IdentExpr* e);
    ComptimeValue evalCall(CallExpr* e);
    ComptimeValue evalIf(IfExpr* e);
    ComptimeValue evalBlock(BlockExpr* e);
    ComptimeValue evalIndex(IndexExpr* e);
    ComptimeValue evalArray(ArrayExpr* e);
    ComptimeValue evalCast(CastExpr* e);
};

} // namespace vex
