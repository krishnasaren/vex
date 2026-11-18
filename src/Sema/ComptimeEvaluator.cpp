#include "vex/Sema/ComptimeEvaluator.h"
#include "vex/AST/Expr.h"
#include <variant>
namespace vex {

static ComptimeValue makeInt(int64_t v)  { ComptimeValue cv; cv.data = v;    return cv; }
static ComptimeValue makeBool(bool v)    { ComptimeValue cv; cv.data = v;    return cv; }
static ComptimeValue makeVoid()          { ComptimeValue cv;                 return cv; }
static ComptimeValue makeStr(std::string s){ ComptimeValue cv; cv.data = s; return cv; }

ComptimeValue ComptimeEvaluator::evaluate(ExprNode* e) {
    if (!e) return makeVoid();
    switch (e->kind()) {
    case ASTNodeKind::IntLitExpr:   return evalLiteral(static_cast<IntLitExpr*>(e));
    case ASTNodeKind::BinaryExpr:   return evalBinary(static_cast<BinaryExpr*>(e));
    case ASTNodeKind::UnaryExpr:    return evalUnary(static_cast<UnaryExpr*>(e));
    case ASTNodeKind::IdentExpr:    return evalIdent(static_cast<IdentExpr*>(e));
    case ASTNodeKind::BoolLitExpr:  return makeBool(static_cast<BoolLitExpr*>(e)->value);
    case ASTNodeKind::FloatLitExpr: {
        ComptimeValue cv; cv.data = static_cast<FloatLitExpr*>(e)->value; return cv; }
    case ASTNodeKind::StringLitExpr: return makeStr(std::string(static_cast<StringLitExpr*>(e)->value));
    case ASTNodeKind::IfExpr:        return evalIf(static_cast<IfExpr*>(e));
    case ASTNodeKind::IndexExpr:     return evalIndex(static_cast<IndexExpr*>(e));
    case ASTNodeKind::CallExpr:      return evalCall(static_cast<CallExpr*>(e));
    case ASTNodeKind::CastExpr:      return evalCast(static_cast<CastExpr*>(e));
    default: return makeVoid();
    }
}

ComptimeValue ComptimeEvaluator::evalLiteral(IntLitExpr* e) {
    return makeInt(static_cast<int64_t>(e->value));
}

ComptimeValue ComptimeEvaluator::evalBinary(BinaryExpr* e) {
    ComptimeValue lv = evaluate(e->left);
    ComptimeValue rv = evaluate(e->right);
    if (lv.isInt() && rv.isInt()) {
        int64_t l = lv.asInt(), r = rv.asInt();
        switch (e->op) {
        case TokenKind::Plus:      return makeInt(l + r);
        case TokenKind::Minus:     return makeInt(l - r);
        case TokenKind::Star:      return makeInt(l * r);
        case TokenKind::Slash:     return r ? makeInt(l / r) : makeVoid();
        case TokenKind::Percent:   return r ? makeInt(l % r) : makeVoid();
        case TokenKind::Eq:        return makeBool(l == r);
        case TokenKind::NotEq:     return makeBool(l != r);
        case TokenKind::Less:      return makeBool(l <  r);
        case TokenKind::LessEq:    return makeBool(l <= r);
        case TokenKind::Greater:   return makeBool(l >  r);
        case TokenKind::GreaterEq: return makeBool(l >= r);
        case TokenKind::Amp:       return makeInt(l & r);
        case TokenKind::Pipe:      return makeInt(l | r);
        case TokenKind::Caret:     return makeInt(l ^ r);
        case TokenKind::LShift:    return makeInt(l << r);
        case TokenKind::RShift:    return makeInt(l >> r);
        default: break;
        }
    }
    if (lv.isBool() && rv.isBool()) {
        bool l = lv.asBool(), r = rv.asBool();
        switch (e->op) {
        case TokenKind::AmpAmp:  return makeBool(l && r);
        case TokenKind::PipePipe:return makeBool(l || r);
        case TokenKind::Eq:      return makeBool(l == r);
        case TokenKind::NotEq:   return makeBool(l != r);
        default: break;
        }
    }
    return makeVoid();
}

ComptimeValue ComptimeEvaluator::evalUnary(UnaryExpr* e) {
    ComptimeValue v = evaluate(e->operand);
    if (e->op == TokenKind::Minus && v.isInt())   return makeInt(-v.asInt());
    if (e->op == TokenKind::Minus && v.isFloat()) { ComptimeValue c; c.data = -v.asFloat(); return c; }
    if (e->op == TokenKind::Bang  && v.isBool())  return makeBool(!v.asBool());
    return v;
}

ComptimeValue ComptimeEvaluator::evalIdent(IdentExpr* e) {
    auto it = env_.find(std::string(e->name));
    return (it != env_.end()) ? it->second : makeVoid();
}

ComptimeValue ComptimeEvaluator::evalIf(IfExpr* e) {
    ComptimeValue cond = evaluate(e->condition);
    if (cond.isBool())
        return cond.asBool() ? evaluate(e->thenBranch) : evaluate(e->elseBranch);
    return makeVoid();
}

ComptimeValue ComptimeEvaluator::evalCall(CallExpr*)   { return makeVoid(); }
ComptimeValue ComptimeEvaluator::evalBlock(BlockExpr*) { return makeVoid(); }
ComptimeValue ComptimeEvaluator::evalIndex(IndexExpr*) { return makeVoid(); }
ComptimeValue ComptimeEvaluator::evalArray(ListExpr*)  { return makeVoid(); }
ComptimeValue ComptimeEvaluator::evalCast(CastExpr*)   { return makeVoid(); }

bool ComptimeEvaluator::run(ModuleDecl* mod) {
    if (!mod) return true;
    for (auto* d : mod->decls) {
        if (d && d->kind() == ASTNodeKind::ConstDecl) {
            auto* c = static_cast<ConstDecl*>(d);
            if (c->value) evaluate(c->value);
        }
    }
    return !ctx_.diags().hasErrors();
}

} // namespace vex
