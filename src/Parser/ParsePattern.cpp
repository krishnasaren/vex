// src/Parser/ParsePattern.cpp
// ============================================================================
// vex/Parser/ParsePattern.cpp
// Pattern parsing for match statements and if-let / while-let.
// ============================================================================

#include "vex/Parser/Parser.h"

namespace vex {

ParseResult<PatternNode*> Parser::parsePattern() {
    return parseOrPattern();
}

// OR pattern: pat1 | pat2 | pat3
ParseResult<PatternNode*> Parser::parseOrPattern() {
    auto loc = currentLoc();
    auto first = parsePatternAtom();
    if (!first.ok()) return first;

    if (!check(TokenKind::Pipe)) return first;

    auto* op = ctx_.make<OrPattern>(loc);
    op->alternatives.push_back(first.get());

    while (match(TokenKind::Pipe)) {
        auto alt = parsePatternAtom();
        if (alt.ok()) op->alternatives.push_back(alt.get());
    }

    return ParseResult<PatternNode*>(op);
}

ParseResult<PatternNode*> Parser::parsePatternAtom() {
    auto loc = currentLoc();
    TokenKind k = current().kind();

    // Wildcard: _
    if (k == TokenKind::Identifier && current().text() == "_") {
        advance();
        return ParseResult<PatternNode*>(ctx_.make<WildcardPattern>(loc));
    }

    // Type pattern: as TypeName binding
    if (k == TokenKind::KW_as) {
        advance();
        auto t = parseType();
        std::string_view bindName;
        if (check(TokenKind::Identifier)) {
            bindName = ctx_.intern(current().text());
            advance();
        }
        if (!t.ok()) return ParseResult<PatternNode*>::error();
        return ParseResult<PatternNode*>(ctx_.make<TypePattern>(loc, t.get(), bindName));
    }

    // Tuple pattern: (a, b, c)
    if (k == TokenKind::LParen) {
        advance();
        auto* tp = ctx_.make<TuplePattern>(loc);
        while (!check(TokenKind::RParen) && !atEnd()) {
            auto p = parseOrPattern();
            if (p.ok()) tp->elements.push_back(p.get());
            if (!match(TokenKind::Comma)) break;
        }
        expectConsume(TokenKind::RParen);
        return ParseResult<PatternNode*>(tp);
    }

    // Struct pattern: { x, y: 0 } or { x, .. }
    if (k == TokenKind::LBrace) {
        advance();
        auto* sp = ctx_.make<StructPattern>(loc);
        while (!check(TokenKind::RBrace) && !atEnd()) {
            if (check(TokenKind::DotDot)) {
                advance();
                sp->hasIgnoreRest = true;
                break;
            }
            StructFieldPattern sfp;
            sfp.loc = currentLoc();
            if (!check(TokenKind::Identifier)) break;
            sfp.fieldName = ctx_.intern(current().text());
            advance();
            if (match(TokenKind::Colon)) {
                auto p = parseOrPattern();
                if (p.ok()) sfp.pattern = p.get();
            }
            sp->fields.push_back(sfp);
            if (!match(TokenKind::Comma)) break;
        }
        expectConsume(TokenKind::RBrace);
        return ParseResult<PatternNode*>(sp);
    }

    // Literal patterns: numbers, strings, bools, null
    if (isLiteral(k)) {
        auto expr = parseLiteral();
        if (!expr.ok()) return ParseResult<PatternNode*>::error();

        // Range pattern: lo..hi or lo..=hi
        if (check(TokenKind::DotDot) || check(TokenKind::DotDotEq)) {
            bool inclusive = (current().kind() == TokenKind::DotDotEq);
            advance();
            auto hi = parseLiteral();
            if (!hi.ok()) return ParseResult<PatternNode*>::error();
            return ParseResult<PatternNode*>(
                ctx_.make<RangePattern>(loc, expr.get(), hi.get(), inclusive));
        }

        return ParseResult<PatternNode*>(ctx_.make<LiteralPattern>(loc, expr.get()));
    }

    // Identifier: either binding pattern or enum variant
    if (k == TokenKind::Identifier || isKeyword(k)) {
        std::string_view name = ctx_.intern(current().text());
        advance();

        // Enum variant with data: VariantName(fields...)
        if (check(TokenKind::LParen)) {
            advance();
            auto* ep = ctx_.make<EnumPattern>(loc);
            ep->name = name;
            while (!check(TokenKind::RParen) && !atEnd()) {
                EnumFieldPattern efp;
                efp.loc = currentLoc();
                // Named field: fieldName: pat
                if (check(TokenKind::Identifier) && peek(1).kind() == TokenKind::Colon) {
                    efp.name   = ctx_.intern(current().text());
                    efp.isNamed = true;
                    advance(); advance();
                }
                auto p = parseOrPattern();
                if (p.ok()) efp.pattern = p.get();
                ep->fields.push_back(efp);
                if (!match(TokenKind::Comma)) break;
            }
            expectConsume(TokenKind::RParen);
            return ParseResult<PatternNode*>(ep);
        }

        // Enum variant no data: just a name — could be EnumPattern or binding
        // Sema resolves whether it's a variant or a new binding
        // For now treat as binding unless it starts with uppercase
        bool looksLikeVariant = (!name.empty() && name[0] >= 'A' && name[0] <= 'Z');
        if (looksLikeVariant) {
            auto* ep = ctx_.make<EnumPattern>(loc);
            ep->name = name;
            return ParseResult<PatternNode*>(ep);
        }

        auto* bp = ctx_.make<BindingPattern>(loc, name);
        if (name.size() > 3 && name.substr(0,3) == "var") {
            bp->isMutable = true;
        }
        return ParseResult<PatternNode*>(bp);
    }

    // Negative number: -42
    if (k == TokenKind::Minus) {
        advance();
        auto expr = parseLiteral();
        if (!expr.ok()) return ParseResult<PatternNode*>::error();
        // Wrap in unary minus — sema folds this
        auto* neg = ctx_.make<UnaryExpr>(loc, TokenKind::Minus, expr.get());
        return ParseResult<PatternNode*>(ctx_.make<LiteralPattern>(loc, neg));
    }

    expectedError("pattern");
    advance();
    return ParseResult<PatternNode*>::error();
}

} // namespace vex