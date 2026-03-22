// src/Parser/ParseStmt.cpp
// ============================================================================
// vex/Parser/ParseStmt.cpp
// Statement parsing: all control flow constructs.
// ============================================================================

#include "vex/Parser/Parser.h"
#include "vex/Core/Assert.h"

namespace vex {

ParseResult<StmtNode*> Parser::parseStmt() {
    TokenKind k = current().kind();

    // Label: for/while/loop
    if (k == TokenKind::Identifier && peek(1).kind() == TokenKind::Colon) {
        auto loc = currentLoc();
        std::string_view label = ctx_.intern(current().text());
        advance(); advance(); // consume name + ':'
        auto inner = parseStmt();
        if (!inner.ok()) return ParseResult<StmtNode*>::error();
        auto* ls = ctx_.make<LabeledStmt>(loc, label, inner.get());
        return ParseResult<StmtNode*>(ls);
    }

    if (k == TokenKind::KW_var || k == TokenKind::KW_let || k == TokenKind::KW_const)
        return parseVarDeclStmt();

    if (k == TokenKind::KW_if)     { advance(); return parseIfStmt(); }
    if (k == TokenKind::KW_while)  { advance(); return parseWhileStmt(); }
    if (k == TokenKind::KW_do)     { advance(); return parseDoWhileStmt(); }
    if (k == TokenKind::KW_for)    { advance(); return parseForStmt(); }
    if (k == TokenKind::KW_loop)   { advance(); return parseLoopStmt(); }
    if (k == TokenKind::KW_match)  { advance(); return parseMatchStmt(); }
    if (k == TokenKind::KW_return) { advance(); return parseReturnStmt(); }
    if (k == TokenKind::KW_skip)   { advance(); return parseSkipStmt(); }
    if (k == TokenKind::KW_stop)   { advance(); return parseStopStmt(); }
    if (k == TokenKind::KW_defer)  { advance(); return parseDeferStmt(); }
    if (k == TokenKind::KW_use)    { advance(); return parseUseLetStmt(); }
    if (k == TokenKind::KW_try)    { advance(); return parseTryCatchStmt(); }
    if (k == TokenKind::KW_select) { advance(); return parseSelectStmt(); }
    if (k == TokenKind::KW_unsafe) { advance(); return parseUnsafeBlock(); }

    // Bare '{' is a block expression statement
    if (k == TokenKind::LBrace) {
        auto block = parseBlock();
        if (!block.ok()) return ParseResult<StmtNode*>::error();
        auto* es = ctx_.make<ExprStmt>(block.get()->location(),
                       ctx_.make<BlockExpr>(block.get()->location(), block.get()));
        return ParseResult<StmtNode*>(es);
    }

    // Anything else is an expression statement
    return parseExprStmt();
}

// ── Block ─────────────────────────────────────────────────────────────────────

ParseResult<BlockStmt*> Parser::parseBlock() {
    auto loc = currentLoc();
    if (!expectConsume(TokenKind::LBrace, "expected '{'"))
        return ParseResult<BlockStmt*>::error();

    auto* block = ctx_.make<BlockStmt>(loc);

    while (!check(TokenKind::RBrace) && !atEnd()) {
        if (diags_.shouldAbort()) break;

        // Peek for trailing expression (last expr without semicolon = value)
        // We parse a statement; if it's an ExprStmt at the end without ;, it becomes trailing
        auto stmt = parseStmt();
        if (!stmt.ok()) {
            synchronize({TokenKind::RBrace, TokenKind::Semicolon});
            continue;
        }

        // Consume optional semicolons
        match(TokenKind::Semicolon);

        // Check if this is the last thing before '}'
        if (check(TokenKind::RBrace)) {
            // Last stmt — if it's an ExprStmt, promote to trailing expr
            if (stmt.get()->is(ASTNodeKind::ExprStmt)) {
                block->trailingExpr = stmt.get()->as<ExprStmt>()->expr;
            } else {
                block->stmts.push_back(stmt.get());
            }
        } else {
            block->stmts.push_back(stmt.get());
        }
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<BlockStmt*>(block);
}

// ── VarDeclStmt ───────────────────────────────────────────────────────────────

ParseResult<VarDeclStmt*> Parser::parseVarDeclStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<VarDeclStmt>(loc);

    TokenKind kw = current().kind();
    stmt->isMutable = (kw == TokenKind::KW_var);
    stmt->isLet     = (kw == TokenKind::KW_let);
    stmt->isConst   = (kw == TokenKind::KW_const);
    advance(); // consume var/let/const

    // Parse one or more bindings: var a, b, c := e1, e2, e3
    do {
        VarDeclStmt::Binding b;
        b.loc = currentLoc();

        if (check(TokenKind::KW_let) || check(TokenKind::Identifier)) {
            if (current().text() == "_") {
                b.isDiscard = true;
                b.name = "_";
            } else {
                b.name = ctx_.intern(current().text());
            }
            advance();
        } else {
            expectedError("variable name");
            break;
        }

        // Optional type annotation: : Type
        if (match(TokenKind::Colon)) {
            auto t = parseType();
            if (t.ok()) b.type = t.get();
        }

        stmt->bindings.push_back(b);
    } while (match(TokenKind::Comma));

    // Initializer: := expr or = expr
    if (match(TokenKind::ColonAssign) || match(TokenKind::Assign)) {
        // Parse comma-separated initializers matching binding count
        size_t idx = 0;
        do {
            auto expr = parseExpr();
            if (expr.ok() && idx < stmt->bindings.size()) {
                stmt->bindings[idx].initializer = expr.get();
            }
            idx++;
        } while (match(TokenKind::Comma) && idx < stmt->bindings.size());
    }

    return ParseResult<VarDeclStmt*>(stmt);
}

// ── ExprStmt ──────────────────────────────────────────────────────────────────

ParseResult<ExprStmt*> Parser::parseExprStmt() {
    auto loc = currentLoc();
    auto expr = parseExpr();
    if (!expr.ok()) return ParseResult<ExprStmt*>::error();

    match(TokenKind::Semicolon); // optional
    return ParseResult<ExprStmt*>(ctx_.make<ExprStmt>(loc, expr.get()));
}

// ── IfStmt ────────────────────────────────────────────────────────────────────

ParseResult<IfStmt*> Parser::parseIfStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<IfStmt>(loc);

    // Parse if branch (and all elif branches)
    do {
        IfStmt::Branch branch;
        branch.loc = currentLoc();

        // if let pattern = expr
        if (check(TokenKind::KW_let)) {
            advance();
            auto pat = parsePattern();
            if (!pat.ok()) return ParseResult<IfStmt*>::error();
            branch.ifLetPattern = pat.get();
            if (!expectConsume(TokenKind::Assign, "expected '=' in if-let"))
                return ParseResult<IfStmt*>::error();
            auto expr = parseExpr();
            if (!expr.ok()) return ParseResult<IfStmt*>::error();
            branch.ifLetExpr = expr.get();
        } else {
            auto cond = parseExpr();
            if (!cond.ok()) return ParseResult<IfStmt*>::error();
            branch.condition = cond.get();
        }

        auto body = parseBlock();
        if (!body.ok()) return ParseResult<IfStmt*>::error();
        branch.body = body.get();

        stmt->branches.push_back(branch);
    } while (match(TokenKind::KW_elif));

    // else branch
    if (match(TokenKind::KW_else)) {
        auto elseb = parseBlock();
        if (elseb.ok()) stmt->elseBranch = elseb.get();
    }

    return ParseResult<IfStmt*>(stmt);
}

// ── WhileStmt ─────────────────────────────────────────────────────────────────

ParseResult<WhileStmt*> Parser::parseWhileStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<WhileStmt>(loc);
    ++loopDepth_;

    if (check(TokenKind::KW_let)) {
        advance();
        auto pat = parsePattern();
        if (pat.ok()) stmt->whileLetPat = pat.get();
        expectConsume(TokenKind::Assign, "expected '=' in while-let");
        auto expr = parseExpr();
        if (expr.ok()) stmt->whileLetExpr = expr.get();
    } else {
        auto cond = parseExpr();
        if (cond.ok()) stmt->condition = cond.get();
    }

    auto body = parseBlock();
    if (body.ok()) stmt->body = body.get();

    --loopDepth_;
    return ParseResult<WhileStmt*>(stmt);
}

// ── DoWhileStmt ───────────────────────────────────────────────────────────────

ParseResult<DoWhileStmt*> Parser::parseDoWhileStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<DoWhileStmt>(loc);
    ++loopDepth_;

    auto body = parseBlock();
    if (body.ok()) stmt->body = body.get();

    if (!expectConsume(TokenKind::KW_while, "expected 'while' after do block"))
        return ParseResult<DoWhileStmt*>::error();

    auto cond = parseExpr();
    if (cond.ok()) stmt->condition = cond.get();

    --loopDepth_;
    return ParseResult<DoWhileStmt*>(stmt);
}

// ── ForStmt ───────────────────────────────────────────────────────────────────

ParseResult<ForStmt*> Parser::parseForStmt() {
    auto loc = currentLoc();
    ++loopDepth_;

    // C-style for: for var i := 0; i < 10; i++ { }
    if (check(TokenKind::KW_var)) {
        auto* cstmt = ctx_.make<ForCStmt>(loc);

        // init: var i := 0
        auto init = parseVarDeclStmt();
        if (init.ok()) cstmt->init = init.get();

        expectConsume(TokenKind::Semicolon, "expected ';' in C-style for");

        auto cond = parseExpr();
        if (cond.ok()) cstmt->condition = cond.get();

        expectConsume(TokenKind::Semicolon, "expected ';' in C-style for");

        auto step = parseExpr();
        if (step.ok()) cstmt->step = step.get();

        auto body = parseBlock();
        if (body.ok()) cstmt->body = body.get();

        --loopDepth_;
        return ParseResult<StmtNode*>(cstmt);
    }

    auto* stmt = ctx_.make<ForStmt>(loc);

    // Pattern-based: for (a, b) in expr
    if (check(TokenKind::LParen)) {
        stmt->isDestructuring = true;
        auto pat = parsePattern();
        if (pat.ok()) stmt->pattern = pat.get();
    } else {
        // for i, x in expr  OR  for x in expr
        if (!check(TokenKind::Identifier)) {
            expectedError("loop variable");
            --loopDepth_;
            return ParseResult<ForStmt*>::error();
        }
        std::string_view first = ctx_.intern(current().text());
        advance();

        if (match(TokenKind::Comma)) {
            // for i, x in ...
            stmt->indexVar = first;
            if (!check(TokenKind::Identifier)) {
                expectedError("value variable after ','");
                --loopDepth_;
                return ParseResult<ForStmt*>::error();
            }
            stmt->valueVar = ctx_.intern(current().text());
            advance();
        } else {
            stmt->valueVar = first;
        }
    }

    if (!expectConsume(TokenKind::KW_in, "expected 'in' in for loop"))
        return ParseResult<ForStmt*>::error();

    auto iter = parseExpr();
    if (iter.ok()) stmt->iterable = iter.get();

    // step expr
    if (match(TokenKind::KW_step)) {
        auto step = parseExpr();
        if (step.ok()) stmt->stepExpr = step.get();
    }

    auto body = parseBlock();
    if (body.ok()) stmt->body = body.get();

    --loopDepth_;
    return ParseResult<ForStmt*>(stmt);
}

// ── LoopStmt ──────────────────────────────────────────────────────────────────

ParseResult<LoopStmt*> Parser::parseLoopStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<LoopStmt>(loc);
    ++loopDepth_;

    auto body = parseBlock();
    if (body.ok()) stmt->body = body.get();

    --loopDepth_;
    return ParseResult<LoopStmt*>(stmt);
}

// ── MatchStmt ─────────────────────────────────────────────────────────────────

ParseResult<MatchStmt*> Parser::parseMatchStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<MatchStmt>(loc);

    auto subject = parseExpr();
    if (subject.ok()) stmt->subject = subject.get();

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open match body"))
        return ParseResult<MatchStmt*>::error();

    stmt->arms = parseMatchArms();

    expectConsume(TokenKind::RBrace);
    return ParseResult<MatchStmt*>(stmt);
}

// ── ReturnStmt ────────────────────────────────────────────────────────────────

ParseResult<ReturnStmt*> Parser::parseReturnStmt() {
    auto loc = currentLoc();

    ExprNode* val = nullptr;
    // If next token can start an expression (and isn't '}' or ';'), parse it
    if (!check(TokenKind::RBrace) && !check(TokenKind::Semicolon) && !atEnd()) {
        auto expr = parseExpr();
        if (expr.ok()) val = expr.get();
    }

    return ParseResult<ReturnStmt*>(ctx_.make<ReturnStmt>(loc, val));
}

// ── SkipStmt / StopStmt ───────────────────────────────────────────────────────

ParseResult<SkipStmt*> Parser::parseSkipStmt() {
    auto loc = currentLoc();
    std::string_view label;
    if (check(TokenKind::Identifier)) {
        label = ctx_.intern(current().text());
        advance();
    }
    if (loopDepth_ == 0) {
        diags_.report(DiagID::FLOW_ContinueOutsideLoop, loc,
                      "'skip' (continue) used outside a loop");
    }
    return ParseResult<SkipStmt*>(ctx_.make<SkipStmt>(loc, label));
}

ParseResult<StopStmt*> Parser::parseStopStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<StopStmt>(loc);

    if (check(TokenKind::Identifier)) {
        stmt->label = ctx_.intern(current().text());
        advance();
    } else if (!check(TokenKind::RBrace) && !check(TokenKind::Semicolon) && !atEnd()) {
        // stop expr (break with value)
        auto expr = parseExpr();
        if (expr.ok()) stmt->value = expr.get();
    }

    if (loopDepth_ == 0) {
        diags_.report(DiagID::FLOW_BreakOutsideLoop, loc,
                      "'stop' (break) used outside a loop");
    }
    return ParseResult<StopStmt*>(stmt);
}

// ── DeferStmt ─────────────────────────────────────────────────────────────────

ParseResult<DeferStmt*> Parser::parseDeferStmt() {
    auto loc = currentLoc();
    auto expr = parseExpr();
    if (!expr.ok()) return ParseResult<DeferStmt*>::error();
    return ParseResult<DeferStmt*>(ctx_.make<DeferStmt>(loc, expr.get()));
}

// ── UseLetStmt ────────────────────────────────────────────────────────────────

ParseResult<UseLetStmt*> Parser::parseUseLetStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<UseLetStmt>(loc);

    if (!expectConsume(TokenKind::KW_let, "expected 'let' after 'use'"))
        return ParseResult<UseLetStmt*>::error();

    if (!check(TokenKind::Identifier)) {
        expectedError("resource name");
        return ParseResult<UseLetStmt*>::error();
    }
    stmt->name = ctx_.intern(current().text());
    advance();

    if (match(TokenKind::Colon)) {
        auto t = parseType();
        if (t.ok()) stmt->type = t.get();
    }

    if (!expectConsume(TokenKind::ColonAssign, "expected ':=' in use-let"))
        return ParseResult<UseLetStmt*>::error();

    auto resource = parseExpr();
    if (!resource.ok()) return ParseResult<UseLetStmt*>::error();
    stmt->resource = resource.get();

    auto body = parseBlock();
    if (!body.ok()) return ParseResult<UseLetStmt*>::error();
    stmt->body = body.get();

    return ParseResult<UseLetStmt*>(stmt);
}

// ── TryCatchStmt ─────────────────────────────────────────────────────────────

ParseResult<TryCatchStmt*> Parser::parseTryCatchStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<TryCatchStmt>(loc);

    auto tryBlock = parseBlock();
    if (!tryBlock.ok()) return ParseResult<TryCatchStmt*>::error();
    stmt->tryBlock = tryBlock.get();

    if (!expectConsume(TokenKind::KW_catch, "expected 'catch' after try block"))
        return ParseResult<TryCatchStmt*>::error();

    if (check(TokenKind::Identifier)) {
        stmt->catchVar = ctx_.intern(current().text());
        advance();
    }

    auto catchBlock = parseBlock();
    if (!catchBlock.ok()) return ParseResult<TryCatchStmt*>::error();
    stmt->catchBlock = catchBlock.get();

    return ParseResult<TryCatchStmt*>(stmt);
}

// ── SelectStmt ────────────────────────────────────────────────────────────────

ParseResult<SelectStmt*> Parser::parseSelectStmt() {
    auto loc = currentLoc();
    auto* stmt = ctx_.make<SelectStmt>(loc);

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open select body"))
        return ParseResult<SelectStmt*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        SelectArm arm;
        arm.loc = currentLoc();

        if (check(TokenKind::KW_timeout)) {
            arm.kind = SelectArm::Kind::Timeout;
            advance();
            expectConsume(TokenKind::LParen);
            auto dur = parseExpr();
            if (dur.ok()) arm.timeoutDur = dur.get();
            expectConsume(TokenKind::RParen);
        } else if (check(TokenKind::KW_let) || check(TokenKind::Identifier)) {
            arm.kind = SelectArm::Kind::Recv;
            // varName := channel.recv()
            if (check(TokenKind::Identifier)) {
                arm.bindVar = ctx_.intern(current().text());
                advance();
            }
            expectConsume(TokenKind::ColonAssign, "expected ':=' in select arm");
            auto expr = parseExpr();
            if (expr.ok()) arm.chanExpr = expr.get();
        } else if (check(TokenKind::KW_let) && current().text() == "_") {
            arm.kind = SelectArm::Kind::Default;
            advance();
        }

        if (!expectConsume(TokenKind::FatArrow, "expected '=>' in select arm"))
            break;

        auto body = parseBlock();
        if (body.ok()) arm.body = body.get();

        stmt->arms.push_back(arm);
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<SelectStmt*>(stmt);
}

// ── UnsafeBlock ───────────────────────────────────────────────────────────────

ParseResult<UnsafeStmt*> Parser::parseUnsafeBlock() {
    auto loc = currentLoc();
    auto body = parseBlock();
    if (!body.ok()) return ParseResult<UnsafeStmt*>::error();
    return ParseResult<UnsafeStmt*>(ctx_.make<UnsafeStmt>(loc, body.get()));
}

} // namespace vex