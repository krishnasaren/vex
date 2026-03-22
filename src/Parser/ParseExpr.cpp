// src/Parser/ParseExpr.cpp
// ============================================================================
// vex/Parser/ParseExpr.cpp
// Expression parsing using Pratt (top-down operator precedence) algorithm.
// ============================================================================

#include "vex/Parser/Parser.h"
#include "vex/Core/Assert.h"
#include <sstream>

namespace vex {

// ── Main entry point ──────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseExpr(Prec minPrec) {
    // Parse unary prefix or primary
    ParseResult<ExprNode*> left = parseUnary();
    if (!left.ok()) return left;

    // Pratt loop: consume infix/postfix operators while they have enough precedence
    return parseInfix(left.get(), minPrec);
}

ParseResult<ExprNode*> Parser::parseInfix(ExprNode* left, Prec minPrec) {
    while (!atEnd()) {
        TokenKind k = current().kind();
        Prec prec = infixPrecedence(k);

        if (prec == Prec::None || prec < minPrec)
            break;

        // Assignment operators (right-associative)
        if (isAssignment(k)) {
            auto loc = currentLoc();
            TokenKind op = k;
            advance();
            auto right = parseExpr(Prec::Assignment); // right-associative
            if (!right.ok()) return right;
            left = ctx_.make<AssignExpr>(loc, op, left, right.get());
            continue;
        }

        // Error propagation postfix ?
        if (k == TokenKind::Question) {
            auto loc = currentLoc();
            advance();
            left = ctx_.make<ErrorPropExpr>(loc, left);
            continue;
        }

        // Postfix: () [] . ?. ++ -- as as? is
        auto postfixed = parsePostfix(left);
        if (!postfixed.ok()) return postfixed;
        if (postfixed.get() == left) break; // no postfix consumed
        left = postfixed.get();
        continue;

        // Standard binary operator
        auto loc = currentLoc();
        TokenKind op = k;
        advance();

        Prec nextPrec = isRightAssociative(op)
            ? static_cast<Prec>(static_cast<int8_t>(prec) - 1)
            : prec;

        auto right = parseExpr(nextPrec);
        if (!right.ok()) return right;

        // Special cases
        if (op == TokenKind::QuestionQuestion) {
            left = ctx_.make<NullCoalesceExpr>(loc, left, right.get());
        } else if (op == TokenKind::PipeArrow) {
            left = ctx_.make<PipeExpr>(loc, left, right.get());
        } else if (op == TokenKind::DotDot) {
            left = ctx_.make<RangeExpr>(loc, left, right.get(), /*inclusive=*/false);
        } else if (op == TokenKind::DotDotEq) {
            left = ctx_.make<RangeExpr>(loc, left, right.get(), /*inclusive=*/true);
        } else if (op == TokenKind::KW_as) {
            // x as Type
            auto t = parseType();
            if (!t.ok()) return ParseResult<ExprNode*>::error();
            left = ctx_.make<CastExpr>(loc, left, t.get());
        } else if (op == TokenKind::KW_is) {
            auto t = parseType();
            if (!t.ok()) return ParseResult<ExprNode*>::error();
            left = ctx_.make<TypeCheckExpr>(loc, left, t.get());
        } else {
            left = ctx_.make<BinaryExpr>(loc, op, left, right.get());
        }
    }
    return ParseResult<ExprNode*>(left);
}

// ── Postfix operators ─────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parsePostfix(ExprNode* base) {
    while (true) {
        TokenKind k = current().kind();

        // Function call: expr(args)
        if (k == TokenKind::LParen) {
            base = parseCallExpr(base).get();
            if (!base) return ParseResult<ExprNode*>::error();
            continue;
        }

        // Index: expr[index]
        if (k == TokenKind::LBracket) {
            base = parseIndexExpr(base).get();
            if (!base) return ParseResult<ExprNode*>::error();
            continue;
        }

        // Member access: expr.name
        if (k == TokenKind::Dot) {
            base = parseMemberExpr(base).get();
            if (!base) return ParseResult<ExprNode*>::error();
            continue;
        }

        // Safe call: expr?.name
        if (k == TokenKind::QuestionDot) {
            auto loc = currentLoc();
            advance();
            if (!check(TokenKind::Identifier)) {
                expectedError("member name after '?.'");
                return ParseResult<ExprNode*>::error();
            }
            std::string_view member = ctx_.intern(current().text());
            advance();
            auto* safe = ctx_.make<SafeCallExpr>(loc, base, member);
            // Parse args if followed by (
            if (check(TokenKind::LParen)) {
                safe->args = parseArgList();
            }
            base = safe;
            continue;
        }

        // Null coalesce: expr ?? default
        if (k == TokenKind::QuestionQuestion) {
            auto loc = currentLoc();
            advance();
            auto right = parseExpr(Prec::NullCoal);
            if (!right.ok()) return right;
            base = ctx_.make<NullCoalesceExpr>(loc, base, right.get());
            continue;
        }

        // Postfix ++ / --
        if (k == TokenKind::PlusPlus || k == TokenKind::MinusMinus) {
            auto loc = currentLoc();
            advance();
            auto* u = ctx_.make<UnaryExpr>(loc, k, base);
            u->isPostfix = true;
            base = u;
            continue;
        }

        // Type cast: expr as Type
        if (k == TokenKind::KW_as) {
            auto loc = currentLoc();
            advance();
            // Check for safe cast as?
            bool safe = false;
            if (check(TokenKind::Question)) { safe = true; advance(); }
            auto t = parseType();
            if (!t.ok()) return ParseResult<ExprNode*>::error();
            if (safe)
                base = ctx_.make<SafeCastExpr>(loc, base, t.get());
            else
                base = ctx_.make<CastExpr>(loc, base, t.get());
            continue;
        }

        // Type check: expr is Type
        if (k == TokenKind::KW_is) {
            auto loc = currentLoc();
            advance();
            auto t = parseType();
            if (!t.ok()) return ParseResult<ExprNode*>::error();
            base = ctx_.make<TypeCheckExpr>(loc, base, t.get());
            continue;
        }

        // Error propagation: expr?
        if (k == TokenKind::Question) {
            auto loc = currentLoc();
            advance();
            base = ctx_.make<ErrorPropExpr>(loc, base);
            continue;
        }

        // Pipe: expr |> fn
        if (k == TokenKind::PipeArrow) {
            auto loc = currentLoc();
            advance();
            auto right = parseUnary();
            if (!right.ok()) return right;
            base = ctx_.make<PipeExpr>(loc, base, right.get());
            continue;
        }

        // Module path: module::item (only in import context)
        if (k == TokenKind::ColonColon) {
            base = parseModulePathExpr(base).get();
            if (!base) return ParseResult<ExprNode*>::error();
            continue;
        }

        break;
    }
    return ParseResult<ExprNode*>(base);
}

// ── Unary prefix ──────────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseUnary() {
    auto loc = currentLoc();
    TokenKind k = current().kind();

    if (canStartUnary(k)) {
        advance();
        auto operand = parseUnary();
        if (!operand.ok()) return operand;
        auto* u = ctx_.make<UnaryExpr>(loc, k, operand.get());
        return ParseResult<ExprNode*>(u);
    }

    // comptime expr
    if (k == TokenKind::KW_comptime) {
        advance();
        auto inner = parseUnary();
        if (!inner.ok()) return inner;
        return ParseResult<ExprNode*>(ctx_.make<ComptimeExpr>(loc, inner.get()));
    }

    // unsafe expr
    if (k == TokenKind::KW_unsafe) {
        advance();
        if (check(TokenKind::LBrace)) {
            auto block = parseBlock();
            if (!block.ok()) return ParseResult<ExprNode*>::error();
            auto* be = ctx_.make<BlockExpr>(loc, block.get());
            return ParseResult<ExprNode*>(ctx_.make<UnsafeExpr>(loc, be));
        }
        auto inner = parseUnary();
        if (!inner.ok()) return inner;
        return ParseResult<ExprNode*>(ctx_.make<UnsafeExpr>(loc, inner.get()));
    }

    auto primary = parsePrimary();
    if (!primary.ok()) return primary;
    return parsePostfix(primary.get());
}

// ── Primary expression ────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parsePrimary() {
    auto loc = currentLoc();
    TokenKind k = current().kind();

    // Literals
    if (isLiteral(k)) return parseLiteral();

    // Identifiers
    if (k == TokenKind::Identifier) return parseIdentifierExpr();

    // self / super / Self / mod
    if (k == TokenKind::KW_self) {
        advance();
        return ParseResult<ExprNode*>(ctx_.make<SelfExpr>(loc));
    }
    if (k == TokenKind::KW_super) {
        advance();
        return ParseResult<ExprNode*>(ctx_.make<SuperExpr>(loc));
    }
    if (k == TokenKind::KW_Self) {
        advance();
        return ParseResult<ExprNode*>(ctx_.make<SelfTypeExpr>(loc));
    }
    if (k == TokenKind::KW_mod) {
        advance();
        if (!match(TokenKind::Dot)) {
            diags_.report(DiagID::SEMA_ColonColonOutsideImport, loc,
                          "'mod' must be followed by '.' for module scope access");
            return ParseResult<ExprNode*>::error();
        }
        if (!check(TokenKind::Identifier)) {
            expectedError("member name after 'mod.'");
            return ParseResult<ExprNode*>::error();
        }
        std::string_view name = ctx_.intern(current().text());
        advance();
        return ParseResult<ExprNode*>(ctx_.make<ModExpr>(loc, name));
    }

    // Grouped expr or tuple: (expr) or (a, b, c)
    if (k == TokenKind::LParen) return parseGroupedExprOrTuple();

    // List literal: [1, 2, 3]
    if (k == TokenKind::LBracket) return parseListLiteral();

    // Map/set literal: { } or [k:v]
    if (k == TokenKind::LBrace) {
        // Block expression or struct literal
        if (check(TokenKind::LBrace)) {
            advance(); // lookahead needed
            // If empty or has identifier: expr pairs → could be struct lit or block
            // For simplicity: treat { } as block expression here
            // Struct literals are handled in parseIdentifierExpr
            auto block = parseBlock();
            if (!block.ok()) return ParseResult<ExprNode*>::error();
            return ParseResult<ExprNode*>(ctx_.make<BlockExpr>(loc, block.get()));
        }
    }

    // Lambda: |params| -> expr
    if (k == TokenKind::Pipe) return parseLambda();

    // if expression
    if (k == TokenKind::KW_if) { advance(); return parseIfExpr(); }

    // match expression
    if (k == TokenKind::KW_match) { advance(); return parseMatchExpr(); }

    // async fn lambda / async block
    if (k == TokenKind::KW_async) {
        advance();
        if (check(TokenKind::KW_fn)) return parseLambda();
        // async block expression
        auto block = parseBlock();
        if (!block.ok()) return ParseResult<ExprNode*>::error();
        auto* be = ctx_.make<BlockExpr>(loc, block.get());
        // Wrap in ComptimeExpr as placeholder — sema will handle async context
        return ParseResult<ExprNode*>(be);
    }

    expectedError("expression");
    advance(); // skip bad token
    return ParseResult<ExprNode*>::error();
}

// ── Literal ───────────────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseLiteral() {
    auto loc = currentLoc();
    TokenKind k = current().kind();
    auto text = current().text();

    if (k == TokenKind::IntLiteral) {
        uint64_t value = 0;
        uint8_t base = current().intBase();
        // Parse integer value from text (strip underscores and prefix)
        std::string clean;
        size_t start = 0;
        if (text.size() > 2 && text[0] == '0' &&
            (text[1] == 'x' || text[1] == 'X' ||
             text[1] == 'b' || text[1] == 'B' ||
             text[1] == 'o' || text[1] == 'O')) {
            start = 2;
        }
        for (size_t i = start; i < text.size(); i++) {
            char c = text[i];
            if (c == '_') continue;
            // Stop at suffix characters
            if (c == 'u' || c == 'i' || c == 'L' ||
                c == 's' || c == 'm' || c == 'n') break;
            clean += c;
        }
        try {
            value = std::stoull(clean, nullptr, base);
        } catch (...) {
            diags_.emitSimple(DiagID::LEX_InvalidIntLiteral, loc,
                              "integer literal out of range");
        }
        auto suf = current().intSuffix();
        auto* lit = ctx_.make<IntLitExpr>(loc, value, suf, base);
        advance();
        return ParseResult<ExprNode*>(lit);
    }

    if (k == TokenKind::FloatLiteral) {
        double value = 0.0;
        std::string clean;
        for (char c : text) {
            if (c == '_') continue;
            if (c == 'f') break;
            clean += c;
        }
        try { value = std::stod(clean); } catch (...) {}
        auto suf = current().floatSuffix();
        auto* lit = ctx_.make<FloatLitExpr>(loc, value, suf);
        advance();
        return ParseResult<ExprNode*>(lit);
    }

    if (k == TokenKind::StringLiteral || k == TokenKind::RawString) {
        // Strip quotes from text
        std::string_view raw = text;
        std::string_view val = raw;
        if (raw.size() >= 2 && raw.front() == '"') {
            val = raw.substr(1, raw.size() - 2);
        }
        auto* lit = ctx_.make<StringLitExpr>(loc, ctx_.intern(val));
        lit->raw = ctx_.intern(raw);
        advance();
        return ParseResult<ExprNode*>(lit);
    }

    if (k == TokenKind::FmtString) {
        Token tok = advance();
        return parseFmtString(tok);
    }

    if (k == TokenKind::MultilineStr) {
        std::string_view raw = text;
        std::string_view val = (raw.size() >= 2) ? raw.substr(1, raw.size()-2) : raw;
        auto* lit = ctx_.make<MultilineStrExpr>(loc, ctx_.intern(val));
        advance();
        return ParseResult<ExprNode*>(lit);
    }

    if (k == TokenKind::CharLiteral) {
        // Convert char literal to codepoint
        uint32_t cp = 0;
        if (text.size() >= 3) cp = static_cast<uint8_t>(text[1]);
        auto* lit = ctx_.make<CharLitExpr>(loc, cp);
        advance();
        return ParseResult<ExprNode*>(lit);
    }

    if (k == TokenKind::BoolTrue) {
        advance();
        return ParseResult<ExprNode*>(ctx_.make<BoolLitExpr>(loc, true));
    }
    if (k == TokenKind::BoolFalse) {
        advance();
        return ParseResult<ExprNode*>(ctx_.make<BoolLitExpr>(loc, false));
    }
    if (k == TokenKind::Null) {
        advance();
        return ParseResult<ExprNode*>(ctx_.make<NullLitExpr>(loc));
    }

    expectedError("literal");
    return ParseResult<ExprNode*>::error();
}

// ── Identifier / path / struct literal ────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseIdentifierExpr() {
    auto loc = currentLoc();
    std::string_view name = ctx_.intern(current().text());
    advance();

    // Module path: Name::Item  (only valid after import in sema)
    if (check(TokenKind::ColonColon)) {
        auto* mpe = ctx_.make<ModulePathExpr>(loc);
        mpe->segments.push_back(name);
        while (match(TokenKind::ColonColon)) {
            if (!check(TokenKind::Identifier)) {
                expectedError("identifier after '::'");
                break;
            }
            mpe->segments.push_back(ctx_.intern(current().text()));
            advance();
        }
        return ParseResult<ExprNode*>(mpe);
    }

    auto* id = ctx_.make<IdentExpr>(loc, name);

    // Struct literal: TypeName { field: val, ... }
    // Look-ahead: if followed by '{' with field-like content
    if (check(TokenKind::LBrace)) {
        // Heuristic: if next after '{' is identifier followed by ':' or '...', it's a struct lit
        // We use a simple check: try to parse struct literal
        return parseStructLiteral(id);
    }

    // Generic instantiation: Box<T>(args)
    if (check(TokenKind::Less) && peek(2).kind() == TokenKind::Greater) {
        // Could be generic type args — parse tentatively
        // For now return identifier; postfix handles the call
    }

    return ParseResult<ExprNode*>(id);
}

// ── Grouped expr / tuple ──────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseGroupedExprOrTuple() {
    auto loc = currentLoc();
    expectConsume(TokenKind::LParen);

    // Empty tuple: ()
    if (check(TokenKind::RParen)) {
        advance();
        auto* t = ctx_.make<TupleExpr>(loc);
        return ParseResult<ExprNode*>(t);
    }

    auto first = parseExpr();
    if (!first.ok()) return first;

    // Single grouped: (expr)
    if (match(TokenKind::RParen))
        return first;

    // Tuple: (a, b, c)
    auto* tuple = ctx_.make<TupleExpr>(loc);
    tuple->elements.push_back(first.get());

    while (match(TokenKind::Comma)) {
        if (check(TokenKind::RParen)) break;
        auto elem = parseExpr();
        if (elem.ok()) tuple->elements.push_back(elem.get());
    }
    expectConsume(TokenKind::RParen);
    return ParseResult<ExprNode*>(tuple);
}

// ── List literal ──────────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseListLiteral() {
    auto loc = currentLoc();
    expectConsume(TokenKind::LBracket);

    auto* list = ctx_.make<ListExpr>(loc);

    while (!check(TokenKind::RBracket) && !atEnd()) {
        // Spread: ...expr
        if (match(TokenKind::DotDotDot)) {
            auto elem = parseExpr();
            if (elem.ok()) list->elements.push_back(elem.get()); // sema handles spread
            if (!match(TokenKind::Comma)) break;
            continue;
        }
        auto elem = parseExpr();
        if (!elem.ok()) break;

        // Map literal: [k: v, ...]  — if we see ':' after first elem, convert to map
        if (check(TokenKind::Colon)) {
            advance();
            auto val = parseExpr();
            auto* map = ctx_.make<MapExpr>(loc);
            MapEntry entry;
            entry.key = elem.get();
            entry.value = val.ok() ? val.get() : nullptr;
            entry.loc = loc;
            map->entries.push_back(entry);

            while (match(TokenKind::Comma)) {
                if (check(TokenKind::RBracket)) break;
                auto k = parseExpr();
                expectConsume(TokenKind::Colon);
                auto v = parseExpr();
                MapEntry e2;
                e2.key = k.ok() ? k.get() : nullptr;
                e2.value = v.ok() ? v.get() : nullptr;
                e2.loc = currentLoc();
                map->entries.push_back(e2);
            }
            expectConsume(TokenKind::RBracket);
            return ParseResult<ExprNode*>(map);
        }

        list->elements.push_back(elem.get());
        if (!match(TokenKind::Comma)) break;
    }

    expectConsume(TokenKind::RBracket);
    return ParseResult<ExprNode*>(list);
}

// ── Lambda ────────────────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseLambda() {
    auto loc = currentLoc();
    auto* lambda = ctx_.make<LambdaExpr>(loc);

    // Capture mode: copy | or move |
    if (check(TokenKind::KW_copy)) { lambda->isCopyCapture = true; advance(); }
    if (check(TokenKind::KW_move)) { lambda->isMoveCapture = true; advance(); }
    if (check(TokenKind::KW_async)) { lambda->isAsync = true; advance(); }

    // Expect '|' to open params
    if (!expectConsume(TokenKind::Pipe, "expected '|' to open lambda parameters"))
        return ParseResult<ExprNode*>::error();

    // Parse parameters until closing '|'
    while (!check(TokenKind::Pipe) && !atEnd()) {
        LambdaExpr::LambdaParam p;
        p.loc = currentLoc();
        if (!check(TokenKind::Identifier)) break;
        p.name = ctx_.intern(current().text());
        advance();
        if (match(TokenKind::Colon)) {
            auto t = parseType();
            if (t.ok()) p.type = t.get();
        }
        lambda->params.push_back(p);
        if (!match(TokenKind::Comma)) break;
    }

    expectConsume(TokenKind::Pipe, "expected '|' to close lambda parameters");

    // Return type annotation: -> Type
    if (match(TokenKind::Arrow)) {
        // Check if it's a type or an expression
        if (check(TokenKind::LBrace)) {
            // Block body
            auto block = parseBlock();
            if (block.ok()) lambda->bodyBlock = block.get();
        } else {
            auto t = parseType();
            if (t.ok()) {
                lambda->returnType = t.get();
                // Now parse body
                if (check(TokenKind::LBrace)) {
                    auto block = parseBlock();
                    if (block.ok()) lambda->bodyBlock = block.get();
                } else {
                    auto expr = parseExpr();
                    if (expr.ok()) lambda->bodyExpr = expr.get();
                }
            }
        }
    } else {
        // No arrow — body is just an expression or block
        if (check(TokenKind::LBrace)) {
            auto block = parseBlock();
            if (block.ok()) lambda->bodyBlock = block.get();
        } else {
            auto expr = parseExpr();
            if (expr.ok()) lambda->bodyExpr = expr.get();
        }
    }

    return ParseResult<ExprNode*>(lambda);
}

// ── If expression ─────────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseIfExpr() {
    auto loc = currentLoc();
    auto* expr = ctx_.make<IfExpr>(loc);

    if (check(TokenKind::KW_let)) {
        advance();
        auto pat = parsePattern();
        if (pat.ok()) expr->ifLetPattern = pat.get();
        expectConsume(TokenKind::Assign);
    }

    auto cond = parseExpr();
    if (!cond.ok()) return ParseResult<ExprNode*>::error();
    expr->condition = cond.get();

    auto thenBlock = parseBlock();
    if (!thenBlock.ok()) return ParseResult<ExprNode*>::error();
    expr->thenBranch = ctx_.make<BlockExpr>(loc, thenBlock.get());

    if (match(TokenKind::KW_else)) {
        if (check(TokenKind::KW_if)) {
            advance();
            auto elseIfExpr = parseIfExpr();
            if (elseIfExpr.ok()) expr->elseBranch = elseIfExpr.get();
        } else {
            auto elseBlock = parseBlock();
            if (elseBlock.ok())
                expr->elseBranch = ctx_.make<BlockExpr>(loc, elseBlock.get());
        }
    }

    return ParseResult<ExprNode*>(expr);
}

// ── Match expression ──────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseMatchExpr() {
    auto loc = currentLoc();
    auto subject = parseExpr();
    if (!subject.ok()) return subject;

    auto* me = ctx_.make<MatchExpr>(loc, subject.get());

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open match body"))
        return ParseResult<ExprNode*>::error();

    me->arms = parseMatchArms();
    expectConsume(TokenKind::RBrace);
    return ParseResult<ExprNode*>(me);
}

std::vector<MatchArm*> Parser::parseMatchArms() {
    std::vector<MatchArm*> arms;
    while (!check(TokenKind::RBrace) && !atEnd()) {
        auto arm = parseMatchArm();
        if (arm.ok()) arms.push_back(arm.get());
        else synchronize({TokenKind::RBrace, TokenKind::Comma});
        match(TokenKind::Comma);
    }
    return arms;
}

ParseResult<MatchArm*> Parser::parseMatchArm() {
    auto loc = currentLoc();
    auto* arm = ctx_.make<MatchArm>(loc);

    arm->pattern = parseOrPattern().get();

    // Guard: if condition
    if (match(TokenKind::KW_if)) {
        auto guard = parseExpr();
        if (guard.ok()) arm->guard = guard.get();
    }

    if (!expectConsume(TokenKind::FatArrow, "expected '=>' in match arm"))
        return ParseResult<MatchArm*>::error();

    if (check(TokenKind::LBrace)) {
        auto block = parseBlock();
        if (block.ok()) arm->bodyBlock = block.get();
    } else {
        auto expr = parseExpr();
        if (expr.ok()) arm->bodyExpr = expr.get();
    }

    return ParseResult<MatchArm*>(arm);
}

// ── Call / Index / Member / Path ──────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseCallExpr(ExprNode* callee) {
    auto loc = currentLoc();
    auto* call = ctx_.make<CallExpr>(loc, callee);
    call->args = parseArgList();
    return ParseResult<ExprNode*>(call);
}

std::vector<Parser::ParsedArg> Parser::parseArgList() {
    std::vector<ParsedArg> args;
    expectConsume(TokenKind::LParen);

    while (!check(TokenKind::RParen) && !atEnd()) {
        ParsedArg arg;
        arg.loc = currentLoc();

        // Named arg: name: expr
        if (check(TokenKind::Identifier) && peek(1).kind() == TokenKind::Colon) {
            arg.name = std::string(current().text());
            advance(); advance(); // consume name + ':'
        }

        // Spread: ...expr
        bool isSpread = false;
        if (match(TokenKind::DotDotDot)) isSpread = true;

        auto expr = parseExpr();
        if (expr.ok()) arg.value = expr.get();

        args.push_back(arg);
        if (!match(TokenKind::Comma)) break;
    }

    expectConsume(TokenKind::RParen);
    return args;
}

ParseResult<ExprNode*> Parser::parseIndexExpr(ExprNode* base) {
    auto loc = currentLoc();
    expectConsume(TokenKind::LBracket);
    auto idx = parseExpr();
    if (!idx.ok()) return idx;
    expectConsume(TokenKind::RBracket);
    return ParseResult<ExprNode*>(ctx_.make<IndexExpr>(loc, base, idx.get()));
}

ParseResult<ExprNode*> Parser::parseMemberExpr(ExprNode* base) {
    auto loc = currentLoc();
    expectConsume(TokenKind::Dot);

    if (!check(TokenKind::Identifier) && !isKeyword(current().kind())) {
        expectedError("member name");
        return ParseResult<ExprNode*>(base);
    }

    std::string_view member = ctx_.intern(current().text());
    advance();
    return ParseResult<ExprNode*>(ctx_.make<MemberExpr>(loc, base, member));
}

ParseResult<ExprNode*> Parser::parseModulePathExpr(ExprNode* base) {
    // base is already an IdentExpr; convert to ModulePathExpr
    auto loc = base->location();
    auto* mpe = ctx_.make<ModulePathExpr>(loc);

    if (base->is(ASTNodeKind::IdentExpr)) {
        mpe->segments.push_back(base->as<IdentExpr>()->name);
    }

    while (match(TokenKind::ColonColon)) {
        if (!check(TokenKind::Identifier)) {
            expectedError("identifier after '::'");
            break;
        }
        mpe->segments.push_back(ctx_.intern(current().text()));
        advance();
    }

    return ParseResult<ExprNode*>(mpe);
}

// ── Struct literal ────────────────────────────────────────────────────────────

ParseResult<ExprNode*> Parser::parseStructLiteral(ExprNode* typeName) {
    auto loc = currentLoc();

    // Look-ahead: is this really a struct literal or a block?
    // Heuristic: if first token after '{' is identifier + ':' or '...', it's struct lit
    // Otherwise treat as block
    auto lookahead1 = peek(1).kind();
    auto lookahead2 = peek(2).kind();

    bool isStructLit = false;
    if (lookahead1 == TokenKind::DotDotDot) isStructLit = true;
    if (lookahead1 == TokenKind::Identifier && lookahead2 == TokenKind::Colon) isStructLit = true;
    if (lookahead1 == TokenKind::RBrace) isStructLit = true; // empty struct lit

    if (!isStructLit) return ParseResult<ExprNode*>(typeName);

    auto* lit = ctx_.make<StructLitExpr>(loc);
    lit->typeName = typeName;

    expectConsume(TokenKind::LBrace);

    while (!check(TokenKind::RBrace) && !atEnd()) {
        // Spread: ...other
        if (match(TokenKind::DotDotDot)) {
            auto spread = parseExpr();
            if (spread.ok()) lit->spreadFrom = spread.get();
            if (!match(TokenKind::Comma)) break;
            continue;
        }

        // field: value
        if (!check(TokenKind::Identifier)) break;

        StructLitField field;
        field.loc  = currentLoc();
        field.name = ctx_.intern(current().text());
        advance();

        if (match(TokenKind::Colon)) {
            auto val = parseExpr();
            if (val.ok()) field.value = val.get();
        } else {
            // Shorthand: { x, y } = { x: x, y: y }
            field.value = ctx_.make<IdentExpr>(field.loc, field.name);
        }

        lit->fields.push_back(field);
        if (!match(TokenKind::Comma)) break;
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<ExprNode*>(lit);
}

// ── Format string ─────────────────────────────────────────────────────────────

ParseResult<FmtStringExpr*> Parser::parseFmtString(Token fmtTok) {
    auto loc = fmtTok.location();
    auto* fse = ctx_.make<FmtStringExpr>(loc);
    fse->raw = ctx_.intern(fmtTok.text());

    // Parse f"...{expr}..." segments
    // The token already contains the full source text; we re-parse the interpolations
    std::string_view src = fmtTok.text();
    if (src.size() < 2) return ParseResult<FmtStringExpr*>(fse);

    // Strip f" and "
    size_t start = 2; // skip f"
    size_t end   = src.size() - 1; // skip trailing "

    std::string textBuf;
    size_t i = start;
    while (i < end) {
        if (src[i] == '{' && i+1 < end && src[i+1] == '{') {
            textBuf += '{';
            i += 2;
            continue;
        }
        if (src[i] == '}' && i+1 < end && src[i+1] == '}') {
            textBuf += '}';
            i += 2;
            continue;
        }
        if (src[i] == '{') {
            // Flush text segment
            if (!textBuf.empty()) {
                FmtSegment seg;
                seg.kind = FmtSegment::Kind::Text;
                seg.text = ctx_.intern(textBuf);
                fse->segments.push_back(seg);
                textBuf.clear();
            }
            // Find closing '}'
            size_t j = i + 1;
            int depth = 1;
            while (j < end && depth > 0) {
                if (src[j] == '{') ++depth;
                if (src[j] == '}') --depth;
                ++j;
            }
            // src[i+1..j-2] is the expression source
            std::string_view exprSrc = src.substr(i+1, j-i-2);
            // Check for format spec after ':'
            std::string_view fmtSpec;
            auto colonPos = exprSrc.find(':');
            if (colonPos != std::string_view::npos) {
                fmtSpec  = exprSrc.substr(colonPos + 1);
                exprSrc  = exprSrc.substr(0, colonPos);
            }

            FmtSegment seg;
            seg.kind    = FmtSegment::Kind::Expr;
            seg.fmtSpec = ctx_.intern(fmtSpec);
            // We create a placeholder ident for now; real parsing needs sub-lexer
            // The semantic layer can re-lex/parse the expression string
            seg.expr    = ctx_.make<IdentExpr>(loc, ctx_.intern(exprSrc));
            fse->segments.push_back(seg);

            i = j;
            continue;
        }
        textBuf += src[i];
        ++i;
    }
    if (!textBuf.empty()) {
        FmtSegment seg;
        seg.kind = FmtSegment::Kind::Text;
        seg.text = ctx_.intern(textBuf);
        fse->segments.push_back(seg);
    }

    return ParseResult<FmtStringExpr*>(fse);
}

} // namespace vex