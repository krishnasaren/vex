// ============================================================================
// vex/Parser/Parser.cpp
// Core parser infrastructure: token navigation, expect, synchronize.
// ============================================================================

#include "vex/Parser/Parser.h"
#include "vex/Core/Assert.h"
#include <sstream>

namespace vex {

// ── Constructor ───────────────────────────────────────────────────────────────

Parser::Parser(std::vector<Token>  tokens,
               ASTContext&         ctx,
               SourceManager&      srcMgr,
               DiagnosticEngine&   diags)
    : tokens_(std::move(tokens))
    , pos_(0)
    , ctx_(ctx)
    , srcMgr_(srcMgr)
    , diags_(diags)
{}

// ── parseModule ───────────────────────────────────────────────────────────────

ModuleDecl* Parser::parseModule(std::string_view moduleName) {
    auto* mod = ctx_.make<ModuleDecl>(currentLoc());
    mod->name = ctx_.intern(moduleName);

    while (!atEnd()) {
        if (diags_.shouldAbort()) break;

        // Parse decorators first (they attach to the next decl)
        auto decorators = parseDecorators();

        if (atEnd()) break;

        auto result = parseTopLevelDecl();
        if (result.ok()) {
            auto* decl = result.get();
            // Attach decorators
            if (decl->is(ASTNodeKind::FnDecl) ||
                decl->is(ASTNodeKind::AsyncFnDecl) ||
                decl->is(ASTNodeKind::GeneratorFnDecl)) {
                auto* fn = decl->as<FnDecl>();
                for (auto* d : decorators) fn->decorators.push_back(d);
            }
            // Record special module functions
            if (decl->is(ASTNodeKind::FnDecl)) {
                auto* fn = decl->as<FnDecl>();
                if (fn->name == "init")   mod->initFn  = fn;
                if (fn->name == "deinit") mod->deinitFn = fn;
                if (fn->name == "main")   mod->mainFn  = fn;
            }
            if (decl->is(ASTNodeKind::ImportDecl))
                mod->imports.push_back(decl->as<ImportDecl>());
            else
                mod->decls.push_back(decl);
        } else {
            synchronize();
        }
    }

    return mod;
}

// ── Token navigation ──────────────────────────────────────────────────────────

Token& Parser::current() {
    VEX_ASSERT(pos_ < tokens_.size(), "current() called past end");
    return tokens_[pos_];
}

Token& Parser::peek(uint32_t offset) {
    size_t idx = pos_ + offset;
    if (idx >= tokens_.size()) return tokens_.back(); // Eof
    return tokens_[idx];
}

Token Parser::advance() {
    Token tok = current();
    if (pos_ + 1 < tokens_.size()) ++pos_;
    return tok;
}

bool Parser::atEnd() const {
    return pos_ >= tokens_.size() ||
           tokens_[pos_].kind() == TokenKind::Eof;
}

bool Parser::check(TokenKind kind) const {
    if (pos_ >= tokens_.size()) return kind == TokenKind::Eof;
    return tokens_[pos_].kind() == kind;
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect(TokenKind kind, std::string_view msg) {
    if (check(kind)) return advance();

    std::string message;
    if (!msg.empty()) {
        message = std::string(msg);
    } else {
        auto expected = tokenKindName(kind);
        auto got      = tokenKindName(current().kind());
        message = "expected '" + std::string(expected) +
                  "', found '" + std::string(got) + "'";
    }

    diags_.report(DiagID::PARSE_Expected, currentLoc(), message)
          .setPrimaryLabel(currentRange(), "unexpected token here");

    return Token::makeError(currentLoc(), current().text());
}

bool Parser::expectConsume(TokenKind kind, std::string_view msg) {
    Token t = expect(kind, msg);
    return !t.isError();
}

SourceLocation Parser::currentLoc() const {
    if (pos_ < tokens_.size()) return tokens_[pos_].location();
    if (!tokens_.empty()) return tokens_.back().location();
    return SourceLocation::invalid();
}

SourceRange Parser::currentRange() const {
    if (pos_ < tokens_.size()) return tokens_[pos_].range();
    return SourceRange(currentLoc());
}

void Parser::expectedError(std::string_view what) {
    auto got = tokenKindName(current().kind());
    std::string msg = "expected " + std::string(what) +
                      ", found '" + std::string(got) + "'";
    diags_.report(DiagID::PARSE_Expected, currentLoc(), msg)
          .setPrimaryLabel(currentRange());
}

// ── Error recovery ────────────────────────────────────────────────────────────

void Parser::synchronize(std::initializer_list<TokenKind> syncSet) {
    // Default sync tokens: declaration starters
    while (!atEnd()) {
        TokenKind k = current().kind();

        // Always sync at these statement/declaration starters
        switch (k) {
            case TokenKind::KW_fn:
            case TokenKind::KW_async:
            case TokenKind::KW_struct:
            case TokenKind::KW_class:
            case TokenKind::KW_enum:
            case TokenKind::KW_trait:
            case TokenKind::KW_interface:
            case TokenKind::KW_impl:
            case TokenKind::KW_import:
            case TokenKind::KW_var:
            case TokenKind::KW_let:
            case TokenKind::KW_const:
            case TokenKind::KW_type:
            case TokenKind::KW_distinct:
            case TokenKind::KW_pub:
            case TokenKind::KW_priv:
            case TokenKind::KW_prot:
            case TokenKind::KW_internal:
            case TokenKind::RBrace:
                return;
            default:
                break;
        }

        // Sync at caller-specified tokens
        for (auto sk : syncSet) {
            if (k == sk) return;
        }

        advance();
    }
}

void Parser::skipToStmtEnd() {
    int depth = 0;
    while (!atEnd()) {
        TokenKind k = current().kind();
        if (k == TokenKind::LBrace) { ++depth; advance(); }
        else if (k == TokenKind::RBrace) {
            if (depth == 0) return;
            --depth; advance();
        }
        else if (k == TokenKind::Semicolon && depth == 0) {
            advance();
            return;
        }
        else advance();
    }
}

// ── Visibility parsing ────────────────────────────────────────────────────────

Visibility Parser::parseVisibility() {
    if (match(TokenKind::KW_pub))      return Visibility::Pub;
    if (match(TokenKind::KW_priv))     return Visibility::Priv;
    if (match(TokenKind::KW_prot))     return Visibility::Prot;
    if (match(TokenKind::KW_internal)) return Visibility::Internal;
    return Visibility::Unspecified;
}

// ── Decorator parsing @name or @name(args) ────────────────────────────────────

std::vector<Decorator*> Parser::parseDecorators() {
    std::vector<Decorator*> decorators;
    while (check(TokenKind::At)) {
        auto loc = currentLoc();
        advance(); // consume @

        auto* dec = ctx_.make<Decorator>(loc);

        if (!check(TokenKind::Identifier)) {
            expectedError("decorator name");
            continue;
        }

        dec->name = ctx_.intern(current().text());
        advance();

        // Optional argument list: @name(args...)
        if (match(TokenKind::LParen)) {
            while (!check(TokenKind::RParen) && !atEnd()) {
                auto* arg = parseExpr();
                if (arg.ok()) dec->args.push_back(arg.get());
                if (!match(TokenKind::Comma)) break;
            }
            expectConsume(TokenKind::RParen);
        }

        decorators.push_back(dec);
    }
    return decorators;
}

// ── Generic parameter parsing <T, U: Bound> ───────────────────────────────────

ParseResult<GenericParamList*> Parser::parseGenericParams() {
    if (!check(TokenKind::Less)) return ParseResult<GenericParamList*>(nullptr);

    auto loc = currentLoc();
    advance(); // consume <

    auto* list = ctx_.make<GenericParamList>(loc);

    while (!check(TokenKind::Greater) && !atEnd()) {
        GenericParam gp;
        gp.loc = currentLoc();

        // const N: int  (const generic)
        if (match(TokenKind::KW_const)) {
            gp.isConst = true;
            if (!check(TokenKind::Identifier)) {
                expectedError("generic parameter name");
                return ParseResult<GenericParamList*>::error();
            }
            gp.name = ctx_.intern(current().text());
            advance();
            if (match(TokenKind::Colon)) {
                auto typeRes = parseType();
                // stored as a named type bound for const generics
            }
        } else {
            // T  or  T: Bound  or  T: Bound + Bound2
            if (!check(TokenKind::Identifier)) {
                expectedError("generic type parameter name");
                return ParseResult<GenericParamList*>::error();
            }
            gp.name = ctx_.intern(current().text());
            advance();

            if (match(TokenKind::Colon)) {
                // Parse bounds: Ord + Clone + ...
                do {
                    auto bound = parseType();
                    if (bound.ok()) gp.bounds.push_back(bound.get());
                } while (match(TokenKind::Plus));
            }
        }

        list->params.push_back(std::move(gp));
        if (!match(TokenKind::Comma)) break;
    }

    if (!expectConsume(TokenKind::Greater, "expected '>' to close generic parameters"))
        return ParseResult<GenericParamList*>::error();

    return ParseResult<GenericParamList*>(list);
}

// ── Where clause: where T: Bound, U: Bound ────────────────────────────────────

ParseResult<WhereClause*> Parser::parseWhereClause() {
    if (!match(TokenKind::KW_where))
        return ParseResult<WhereClause*>(nullptr);

    auto loc = currentLoc();
    auto* wc = ctx_.make<WhereClause>(loc);

    do {
        WhereConstraint wcon;
        wcon.loc = currentLoc();

        if (!check(TokenKind::Identifier)) {
            expectedError("type name in where clause");
            break;
        }
        wcon.typeName = ctx_.intern(current().text());
        advance();

        if (!expectConsume(TokenKind::Colon, "expected ':' in where constraint"))
            break;

        // Parse bounds
        do {
            auto bound = parseType();
            if (bound.ok()) wcon.bounds.push_back(bound.get());
        } while (match(TokenKind::Plus));

        wc->constraints.push_back(std::move(wcon));
    } while (match(TokenKind::Comma));

    return ParseResult<WhereClause*>(wc);
}

// ── Function parameter list ───────────────────────────────────────────────────

std::vector<Parser::ParsedParam> Parser::parseParamList() {
    std::vector<ParsedParam> params;

    if (!check(TokenKind::LParen)) return params;
    advance(); // consume (

    while (!check(TokenKind::RParen) && !atEnd()) {
        ParsedParam p;
        p.loc = currentLoc();

        // self  or  &self  (we don't use &mut self in VEX — self is always accessible)
        if (check(TokenKind::KW_self)) {
            p.isSelf = true;
            p.name   = "self";
            advance();
            params.push_back(std::move(p));
            if (!match(TokenKind::Comma)) break;
            continue;
        }

        // variadic: ...name: T
        if (match(TokenKind::DotDotDot)) {
            p.isVariadic = true;
        }

        // &mut T parameter
        if (match(TokenKind::Amp)) {
            if (match(TokenKind::KW_move)) {
                p.isMutableBorrow = true;
            }
        }

        // parameter name
        if (!check(TokenKind::Identifier) && !check(TokenKind::KW_mod)) {
            expectedError("parameter name");
            skipToStmtEnd();
            break;
        }

        // Check that `mod` is not used as parameter name (RULE MOD16)
        if (check(TokenKind::KW_mod)) {
            diags_.report(DiagID::SEMA_ColonColonOutsideImport,
                          currentLoc(),
                          "`mod` is a reserved keyword and cannot be used as a parameter name")
                  .setPrimaryLabel(currentRange());
            advance();
            break;
        }

        p.name = ctx_.intern(current().text());
        advance();

        // : Type
        if (match(TokenKind::Colon)) {
            auto typeRes = parseType();
            if (typeRes.ok()) p.type = typeRes.get();
        }

        // = defaultValue
        if (match(TokenKind::Assign)) {
            auto defVal = parseExpr();
            if (defVal.ok()) p.defVal = defVal.get();
        }

        params.push_back(std::move(p));
        if (!match(TokenKind::Comma)) break;
    }

    expectConsume(TokenKind::RParen, "expected ')' to close parameter list");
    return params;
}

// ── Return type: -> Type ──────────────────────────────────────────────────────

TypeNode* Parser::parseReturnType() {
    if (!match(TokenKind::Arrow)) return nullptr;
    auto result = parseType();
    return result.ok() ? result.get() : nullptr;
}

// ── Import path: std::io or mylib::shapes ────────────────────────────────────

std::vector<std::string> Parser::parseImportPath() {
    std::vector<std::string> segments;

    if (!check(TokenKind::Identifier)) {
        expectedError("module name in import path");
        return segments;
    }

    segments.emplace_back(current().text());
    advance();

    // Keep consuming :: identifier
    while (check(TokenKind::ColonColon)) {
        advance(); // consume ::

        if (!check(TokenKind::Identifier)) {
            expectedError("module name segment after '::'");
            break;
        }
        segments.emplace_back(current().text());
        advance();
    }

    return segments;
}

// ── Selective import list { name1, name2 as local } ──────────────────────────

std::vector<Parser::SelectiveImportItem> Parser::parseSelectiveImportList() {
    std::vector<SelectiveImportItem> items;

    if (!expectConsume(TokenKind::LBrace, "expected '{' for selective import list"))
        return items;

    // { * } — open import
    if (check(TokenKind::Star)) {
        advance();
        SelectiveImportItem item;
        item.isWildcard  = true;
        item.originalName = "*";
        item.localName    = "*";
        items.push_back(std::move(item));
        expectConsume(TokenKind::RBrace);
        return items;
    }

    while (!check(TokenKind::RBrace) && !atEnd()) {
        SelectiveImportItem item;
        item.loc = currentLoc();
        item.isWildcard = false;

        if (!check(TokenKind::Identifier)) {
            expectedError("imported name");
            break;
        }

        item.originalName = std::string(current().text());
        item.localName    = item.originalName;
        advance();

        // as localName
        if (match(TokenKind::KW_as)) {
            if (!check(TokenKind::Identifier)) {
                expectedError("local alias name");
                break;
            }
            // Check that alias is not `mod` (reserved)
            if (check(TokenKind::KW_mod)) {
                diags_.report(DiagID::SEMA_ColonColonOutsideImport,
                              currentLoc(),
                              "`mod` is reserved and cannot be used as an import alias");
                advance();
            } else {
                item.localName = std::string(current().text());
                advance();
            }
        }

        items.push_back(std::move(item));
        if (!match(TokenKind::Comma)) break;
    }

    expectConsume(TokenKind::RBrace, "expected '}' to close selective import list");
    return items;
}

} // namespace vex