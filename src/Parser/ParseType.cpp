// src/Parser/ParseType.cpp
// ============================================================================
// vex/Parser/ParseType.cpp
// Type expression parsing.
// ============================================================================

#include "vex/Parser/Parser.h"

namespace vex {

ParseResult<TypeNode*> Parser::parseType() {
    return parseTypeAtom();
}

ParseResult<TypeNode*> Parser::parseTypeAtom() {
    auto loc = currentLoc();
    TokenKind k = current().kind();

    // Nullable: T? — handled as postfix after base type
    // We parse base type then check for '?'

    TypeNode* base = nullptr;

    // Function type: fn(A, B) -> R
    if (k == TokenKind::KW_fn) {
        auto result = parseFnType();
        if (!result.ok()) return result;
        base = result.get();
    }
    // Owned pointer: ^T
    else if (k == TokenKind::Hat) {
        advance();
        auto inner = parseType();
        if (!inner.ok()) return inner;
        base = ctx_.make<OwnedPtrType>(loc, inner.get());
    }
    // Shared pointer: ~T
    else if (k == TokenKind::Tilde) {
        advance();
        auto inner = parseType();
        if (!inner.ok()) return inner;
        base = ctx_.make<SharedPtrType>(loc, inner.get());
    }
    // Raw pointer: *T
    else if (k == TokenKind::Star) {
        advance();
        auto inner = parseType();
        if (!inner.ok()) return inner;
        base = ctx_.make<RawPtrType>(loc, inner.get());
    }
    // Borrow: &T or &mut T
    else if (k == TokenKind::Amp) {
        advance();
        bool isMut = false;
        if (check(TokenKind::KW_mod) || (check(TokenKind::Identifier) && current().text() == "mut")) {
            isMut = true; advance();
        }
        // lifetime annotation: 'a
        std::string_view lifetime;
        if (check(TokenKind::Identifier) && !current().text().empty() && current().text()[0] == '\'') {
            lifetime = ctx_.intern(current().text());
            advance();
        }
        auto inner = parseType();
        if (!inner.ok()) return inner;
        if (isMut) {
            auto* bt = ctx_.make<MutBorrowType>(loc, inner.get());
            bt->lifetime = lifetime;
            base = bt;
        } else {
            auto* bt = ctx_.make<BorrowType>(loc, inner.get());
            bt->lifetime = lifetime;
            base = bt;
        }
    }
    // Slice: []T
    else if (k == TokenKind::LBracket) {
        advance();
        // Fixed array: [N]T — N is an expr
        if (!check(TokenKind::RBracket)) {
            // Try to parse as integer → fixed array
            if (check(TokenKind::IntLiteral)) {
                auto sizeExpr = parseLiteral();
                expectConsume(TokenKind::RBracket);
                auto elem = parseType();
                if (!elem.ok()) return elem;
                base = ctx_.make<FixedArrayType>(loc, sizeExpr.get(), elem.get());
            }
            // Map type: [K:V]
            else {
                auto keyType = parseType();
                if (!keyType.ok()) return keyType;
                if (match(TokenKind::Colon)) {
                    auto valType = parseType();
                    if (!valType.ok()) return valType;
                    expectConsume(TokenKind::RBracket);
                    base = ctx_.make<MapType>(loc, keyType.get(), valType.get());
                } else {
                    expectConsume(TokenKind::RBracket);
                    base = ctx_.make<SliceType>(loc, keyType.get());
                }
            }
        } else {
            // []T dynamic slice
            expectConsume(TokenKind::RBracket);
            auto elem = parseType();
            if (!elem.ok()) return elem;
            base = ctx_.make<SliceType>(loc, elem.get());
        }
    }
    // Set type: {}T
    else if (k == TokenKind::LBrace) {
        advance();
        expectConsume(TokenKind::RBrace);
        auto elem = parseType();
        if (!elem.ok()) return elem;
        base = ctx_.make<SetType>(loc, elem.get());
    }
    // Tuple type: (A, B, C)
    else if (k == TokenKind::LParen) {
        advance();
        auto* tuple = ctx_.make<TupleType>(loc);
        while (!check(TokenKind::RParen) && !atEnd()) {
            auto t = parseType();
            if (t.ok()) tuple->elements.push_back(t.get());
            if (!match(TokenKind::Comma)) break;
        }
        expectConsume(TokenKind::RParen);
        base = tuple;
    }
    // Type inference: _
    else if (k == TokenKind::KW_let || (k == TokenKind::Identifier && current().text() == "_")) {
        advance();
        base = ctx_.make<InferType>(loc);
    }
    // Named type (includes qualified paths)
    else if (k == TokenKind::Identifier || isKeyword(k)) {
        std::string_view name = ctx_.intern(current().text());
        advance();

        // Qualified type: module::Type
        if (check(TokenKind::ColonColon)) {
            auto* qt = ctx_.make<QualifiedType>(loc);
            qt->segments.push_back(name);
            while (match(TokenKind::ColonColon)) {
                if (check(TokenKind::Identifier)) {
                    qt->segments.push_back(ctx_.intern(current().text()));
                    advance();
                }
            }
            base = qt;
        } else {
            auto* nt = ctx_.make<NamedType>(loc);
            nt->name = name;
            base = nt;
        }

        // Generic: Name<T, U>
        if (check(TokenKind::Less)) {
            auto gt = parseGenericType(base);
            if (gt.ok()) base = gt.get();
        }
    }
    else {
        expectedError("type");
        return ParseResult<TypeNode*>::error();
    }

    // Postfix nullable: T?
    while (check(TokenKind::Question)) {
        advance();
        base = ctx_.make<NullableType>(loc, base);
    }

    return ParseResult<TypeNode*>(base);
}

ParseResult<TypeNode*> Parser::parseFnType() {
    auto loc = currentLoc();
    expectConsume(TokenKind::KW_fn);

    auto* ft = ctx_.make<FnType>(loc);

    // Optional async
    if (match(TokenKind::KW_async)) ft->isAsync = true;

    expectConsume(TokenKind::LParen);
    while (!check(TokenKind::RParen) && !atEnd()) {
        auto t = parseType();
        if (t.ok()) ft->params.push_back(t.get());
        if (!match(TokenKind::Comma)) break;
    }
    expectConsume(TokenKind::RParen);

    if (match(TokenKind::Arrow)) {
        auto ret = parseType();
        if (ret.ok()) ft->returnType = ret.get();
    }

    return ParseResult<TypeNode*>(ft);
}

ParseResult<TypeNode*> Parser::parseGenericType(TypeNode* base) {
    auto loc = currentLoc();
    expectConsume(TokenKind::Less);

    auto* gt = ctx_.make<GenericType>(loc, base);
    while (!check(TokenKind::Greater) && !atEnd()) {
        auto t = parseType();
        if (t.ok()) gt->args.push_back(t.get());
        if (!match(TokenKind::Comma)) break;
    }
    expectConsume(TokenKind::Greater, "expected '>' to close generic type arguments");
    return ParseResult<TypeNode*>(gt);
}

} // namespace vex