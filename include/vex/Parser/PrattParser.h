#pragma once
// include/vex/Parser/PrattParser.h


// ============================================================================
// vex/Parser/PrattParser.h  — Pratt (top-down operator precedence) parser
// for expressions.
// ============================================================================
#include "vex/Parser/ParseContext.h"
#include "vex/Parser/Precedence.h"
#include "vex/AST/Expr.h"

namespace vex {

class PrattParser {
public:
    explicit PrattParser(ParseContext& ctx) : ctx_(ctx) {}

    // Parse an expression with minimum binding power
    ExprNode* parse(Prec minPrec = Prec::Lowest);

private:
    ParseContext& ctx_;

    // Null denotation: parse a token that begins an expression
    ExprNode* nud();
    // Left denotation: extend left side with an infix/postfix operator
    ExprNode* led(ExprNode* left, Prec minPrec);

    ExprNode* parsePrefix();
    ExprNode* parsePrimary();
    ExprNode* parseGrouped();
    ExprNode* parseLiteral();
    ExprNode* parseIdent();
    ExprNode* parseIf();
    ExprNode* parseMatch();
    ExprNode* parseClosure();
    ExprNode* parseBlock();
    ExprNode* parseTuple();
    ExprNode* parseArray();
    ExprNode* parseRange(ExprNode* left, bool inclusive);
    ExprNode* parseCall(ExprNode* callee);
    ExprNode* parseIndex(ExprNode* base);
    ExprNode* parseMember(ExprNode* base);
    ExprNode* parseAwait(ExprNode* base);
    ExprNode* parseTry(ExprNode* base);
    ExprNode* parseCast(ExprNode* base);
};

} // namespace vex
