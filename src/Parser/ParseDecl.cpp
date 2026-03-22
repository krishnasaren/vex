// src/Parser/ParseDecl.cpp
// ============================================================================
// vex/Parser/ParseDecl.cpp
// Top-level and member declaration parsing.
// ============================================================================

#include "vex/Parser/Parser.h"
#include "vex/Core/Assert.h"

namespace vex {

// ── Top-level dispatch ────────────────────────────────────────────────────────

ParseResult<DeclNode*> Parser::parseTopLevelDecl() {
    Visibility vis = parseVisibility();

    TokenKind k = current().kind();

    // async fn
    if (k == TokenKind::KW_async) {
        advance();
        if (!check(TokenKind::KW_fn)) {
            expectedError("'fn' after 'async'");
            return ParseResult<DeclNode*>::error();
        }
        return parseFnDecl(/*isAsync=*/true, /*isGenerator=*/false, vis);
    }

    if (k == TokenKind::KW_fn) {
        advance();
        // fn* generator
        if (match(TokenKind::Star))
            return parseFnDecl(false, /*isGenerator=*/true, vis);
        return parseFnDecl(false, false, vis);
    }

    if (k == TokenKind::KW_struct)    { advance(); return parseStructDecl(vis); }
    if (k == TokenKind::KW_class)     { advance(); return parseClassDecl(vis); }
    if (k == TokenKind::KW_enum)      { advance(); return parseEnumDecl(vis); }
    if (k == TokenKind::KW_trait)     { advance(); return parseTraitDecl(vis); }
    if (k == TokenKind::KW_interface) { advance(); return parseInterfaceDecl(vis); }
    if (k == TokenKind::KW_impl)      { advance(); return parseImplDecl(); }
    if (k == TokenKind::KW_type)      { advance(); return parseTypeAliasDecl(vis); }
    if (k == TokenKind::KW_distinct)  { advance(); return parseDistinctDecl(vis); }
    if (k == TokenKind::KW_var)       { advance(); return parseVarDecl(vis); }
    if (k == TokenKind::KW_let)       { advance(); return parseVarDecl(vis); }
    if (k == TokenKind::KW_const)     { advance(); return parseConstDecl(vis); }
    if (k == TokenKind::KW_import)    { advance(); return parseImport(); }
    if (k == TokenKind::KW_bitfield)  { advance(); return parseBitfieldDecl(vis); }

    expectedError("top-level declaration (fn, struct, class, enum, trait, ...)");
    advance(); // consume bad token
    return ParseResult<DeclNode*>::error();
}

// ── Function declaration ──────────────────────────────────────────────────────

ParseResult<FnDecl*> Parser::parseFnDecl(bool isAsync, bool isGenerator, Visibility vis) {
    auto loc = currentLoc();

    if (!check(TokenKind::Identifier)) {
        expectedError("function name");
        return ParseResult<FnDecl*>::error();
    }

    auto* fn = ctx_.make<FnDecl>(loc);
    fn->name      = ctx_.intern(current().text());
    fn->vis       = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    fn->isAsync    = isAsync;
    fn->isGenerator= isGenerator;
    advance(); // consume name

    // Generic parameters <T, U>
    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) fn->generics = gp.get();
    }

    // Parameter list
    auto params = parseParamList();
    for (auto& p : params) {
        auto* param = ctx_.make<Param>(p.loc);
        param->name          = ctx_.intern(p.name);
        param->type          = p.type;
        param->defaultValue  = p.defVal;
        param->isSelf        = p.isSelf;
        param->isVariadic    = p.isVariadic;
        param->isMutableBorrow = p.isMutableBorrow;
        fn->params.push_back(param);
    }

    // Return type
    fn->returnType = parseReturnType();

    // Where clause
    if (check(TokenKind::KW_where)) {
        auto wc = parseWhereClause();
        if (wc.ok()) fn->where_ = wc.get();
    }

    // Body: either '=' expr (expression body) or '{' block
    if (match(TokenKind::Assign)) {
        // Single-expression body: fn f() -> T = expr
        auto expr = parseExpr();
        if (!expr.ok()) return ParseResult<FnDecl*>::error();
        // Wrap in a block with trailing expression
        auto* block = ctx_.make<BlockStmt>(loc);
        block->trailingExpr = expr.get();
        fn->body = block;
    } else if (check(TokenKind::LBrace)) {
        auto block = parseBlock();
        if (!block.ok()) return ParseResult<FnDecl*>::error();
        fn->body = block.get();
    } else {
        // Abstract (no body — valid in traits/interfaces)
        fn->isAbstract = true;
    }

    return ParseResult<FnDecl*>(fn);
}

// ── Struct declaration ────────────────────────────────────────────────────────

ParseResult<StructDecl*> Parser::parseStructDecl(Visibility vis) {
    auto loc = currentLoc();

    if (!check(TokenKind::Identifier)) {
        expectedError("struct name");
        return ParseResult<StructDecl*>::error();
    }

    auto* decl = ctx_.make<StructDecl>(loc);
    decl->name = ctx_.intern(current().text());
    decl->vis  = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    advance();

    // Generic parameters
    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) decl->generics = gp.get();
    }

    // Implementations: struct Foo : Interface1, Trait2 { }
    if (match(TokenKind::Colon)) {
        do {
            auto t = parseType();
            if (t.ok()) decl->impls.push_back(t.get());
        } while (match(TokenKind::Comma));
    }

    // Where clause
    if (check(TokenKind::KW_where)) {
        auto wc = parseWhereClause();
        if (wc.ok()) decl->where_ = wc.get();
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open struct body"))
        return ParseResult<StructDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        auto decorators = parseDecorators();
        auto member = parseMemberDecl(/*inClass=*/false, /*inTrait=*/false, /*inInterface=*/false);
        if (member.ok()) {
            decl->members.push_back(member.get());
        } else {
            synchronize({TokenKind::RBrace});
        }
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<StructDecl*>(decl);
}

// ── Class declaration ─────────────────────────────────────────────────────────

ParseResult<ClassDecl*> Parser::parseClassDecl(Visibility vis) {
    auto loc = currentLoc();

    bool isAbstract = false;
    bool isSealed   = false;
    if (current().kind() == TokenKind::KW_abstract) { isAbstract = true; advance(); }
    if (current().kind() == TokenKind::KW_sealed)   { isSealed   = true; advance(); }

    // Consume 'class' keyword if not yet consumed
    if (check(TokenKind::KW_class)) advance();

    if (!check(TokenKind::Identifier)) {
        expectedError("class name");
        return ParseResult<ClassDecl*>::error();
    }

    auto* decl = ctx_.make<ClassDecl>(loc);
    decl->name       = ctx_.intern(current().text());
    decl->vis        = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    decl->isAbstract = isAbstract;
    decl->isSealed   = isSealed;
    advance();

    // Generic parameters
    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) decl->generics = gp.get();
    }

    // Inheritance: class Child : Parent, Interface1, Trait2
    if (match(TokenKind::Colon)) {
        // First type is parent (if it is a class name — sema resolves)
        auto first = parseType();
        if (first.ok()) decl->superClass = first.get();

        while (match(TokenKind::Comma)) {
            auto t = parseType();
            if (t.ok()) decl->impls.push_back(t.get());
        }
    }

    if (check(TokenKind::KW_where)) {
        auto wc = parseWhereClause();
        if (wc.ok()) decl->where_ = wc.get();
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open class body"))
        return ParseResult<ClassDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        auto decorators = parseDecorators();
        auto member = parseMemberDecl(/*inClass=*/true, false, false);
        if (member.ok()) {
            decl->members.push_back(member.get());
        } else {
            synchronize({TokenKind::RBrace});
        }
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<ClassDecl*>(decl);
}

// ── Enum declaration ──────────────────────────────────────────────────────────

ParseResult<EnumDecl*> Parser::parseEnumDecl(Visibility vis) {
    auto loc = currentLoc();

    bool isConst = false;
    if (check(TokenKind::KW_const)) { isConst = true; advance(); }
    if (check(TokenKind::KW_enum))  { advance(); }

    if (!check(TokenKind::Identifier)) {
        expectedError("enum name");
        return ParseResult<EnumDecl*>::error();
    }

    auto* decl = ctx_.make<EnumDecl>(loc);
    decl->name    = ctx_.intern(current().text());
    decl->vis     = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    decl->isConst = isConst;
    advance();

    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) decl->generics = gp.get();
    }

    // const enum base type: enum Flags: uint32 { }
    if (isConst && match(TokenKind::Colon)) {
        auto bt = parseType();
        if (bt.ok()) decl->baseType = bt.get();
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open enum body"))
        return ParseResult<EnumDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        // Methods inside enum
        if (check(TokenKind::KW_fn) || check(TokenKind::KW_async)) {
            Visibility mvis = parseVisibility();
            if (check(TokenKind::KW_async)) advance();
            if (check(TokenKind::KW_fn)) advance();
            auto fn = parseFnDecl(false, false, mvis);
            if (fn.ok()) decl->methods.push_back(fn.get());
            continue;
        }
        auto variant = parseEnumVariant();
        if (variant.ok()) {
            decl->variants.push_back(variant.get());
        } else {
            synchronize({TokenKind::RBrace, TokenKind::Identifier});
        }
        match(TokenKind::Comma); // optional comma between variants
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<EnumDecl*>(decl);
}

ParseResult<EnumVariantDecl*> Parser::parseEnumVariant() {
    auto loc = currentLoc();

    if (!check(TokenKind::Identifier)) {
        expectedError("enum variant name");
        return ParseResult<EnumVariantDecl*>::error();
    }

    auto* v = ctx_.make<EnumVariantDecl>(loc);
    v->name = ctx_.intern(current().text());
    advance();

    // Data variant: Variant(field1: T, field2: T)
    if (match(TokenKind::LParen)) {
        while (!check(TokenKind::RParen) && !atEnd()) {
            auto params = parseParamList(); // reuse param parsing logic
            // Actually parse field by field
            auto ploc = currentLoc();
            auto* p = ctx_.make<Param>(ploc);
            if (check(TokenKind::Identifier)) {
                p->name = ctx_.intern(current().text());
                advance();
                if (match(TokenKind::Colon)) {
                    auto t = parseType();
                    if (t.ok()) p->type = t.get();
                }
            }
            v->dataFields.push_back(p);
            if (!match(TokenKind::Comma)) break;
        }
        expectConsume(TokenKind::RParen);
    }

    // Const enum value: Variant = 42
    if (match(TokenKind::Assign)) {
        auto expr = parseExpr();
        if (expr.ok()) v->constValue = expr.get();
    }

    return ParseResult<EnumVariantDecl*>(v);
}

// ── Trait declaration ─────────────────────────────────────────────────────────

ParseResult<TraitDecl*> Parser::parseTraitDecl(Visibility vis) {
    auto loc = currentLoc();

    if (!check(TokenKind::Identifier)) {
        expectedError("trait name");
        return ParseResult<TraitDecl*>::error();
    }

    auto* decl = ctx_.make<TraitDecl>(loc);
    decl->name = ctx_.intern(current().text());
    decl->vis  = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    advance();

    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) decl->generics = gp.get();
    }

    // Super traits: trait B : A + C
    if (match(TokenKind::Colon)) {
        do {
            auto t = parseType();
            if (t.ok()) decl->superTraits.push_back(t.get());
        } while (match(TokenKind::Plus));
    }

    if (check(TokenKind::KW_where)) {
        auto wc = parseWhereClause();
        if (wc.ok()) decl->where_ = wc.get();
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open trait body"))
        return ParseResult<TraitDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        auto decorators = parseDecorators();
        auto member = parseMemberDecl(false, /*inTrait=*/true, false);
        if (member.ok()) decl->members.push_back(member.get());
        else synchronize({TokenKind::RBrace});
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<TraitDecl*>(decl);
}

// ── Interface declaration ─────────────────────────────────────────────────────

ParseResult<InterfaceDecl*> Parser::parseInterfaceDecl(Visibility vis) {
    auto loc = currentLoc();

    if (!check(TokenKind::Identifier)) {
        expectedError("interface name");
        return ParseResult<InterfaceDecl*>::error();
    }

    auto* decl = ctx_.make<InterfaceDecl>(loc);
    decl->name = ctx_.intern(current().text());
    decl->vis  = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    advance();

    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) decl->generics = gp.get();
    }

    if (match(TokenKind::Colon)) {
        do {
            auto t = parseType();
            if (t.ok()) decl->superInterfaces.push_back(t.get());
        } while (match(TokenKind::Comma));
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open interface body"))
        return ParseResult<InterfaceDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        auto decorators = parseDecorators();
        Visibility mvis = parseVisibility();
        bool isAsync = false;
        if (match(TokenKind::KW_async)) isAsync = true;
        if (!check(TokenKind::KW_fn)) {
            expectedError("'fn' in interface body");
            synchronize({TokenKind::RBrace, TokenKind::KW_fn});
            continue;
        }
        advance(); // consume 'fn'
        auto fn = parseFnDecl(isAsync, false, mvis);
        if (fn.ok()) {
            fn.get()->isAbstract = true;
            decl->methods.push_back(fn.get());
        }
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<InterfaceDecl*>(decl);
}

// ── Impl declaration ──────────────────────────────────────────────────────────

ParseResult<ImplDecl*> Parser::parseImplDecl() {
    auto loc = currentLoc();
    auto* decl = ctx_.make<ImplDecl>(loc);

    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) decl->generics = gp.get();
    }

    // impl TraitName for TypeName  OR  impl TypeName
    auto firstType = parseType();
    if (!firstType.ok()) return ParseResult<ImplDecl*>::error();

    if (match(TokenKind::KW_for)) {
        // impl Trait for Type
        decl->ofTrait = firstType.get();
        auto forType = parseType();
        if (!forType.ok()) return ParseResult<ImplDecl*>::error();
        decl->forType = forType.get();
    } else {
        // impl Type
        decl->forType = firstType.get();
    }

    if (check(TokenKind::KW_where)) {
        auto wc = parseWhereClause();
        if (wc.ok()) decl->where_ = wc.get();
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open impl body"))
        return ParseResult<ImplDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        auto decorators = parseDecorators();
        auto member = parseMemberDecl(false, false, false);
        if (member.ok()) decl->members.push_back(member.get());
        else synchronize({TokenKind::RBrace});
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<ImplDecl*>(decl);
}

// ── Member declaration dispatcher ────────────────────────────────────────────

ParseResult<DeclNode*> Parser::parseMemberDecl(bool inClass, bool inTrait, bool inInterface) {
    Visibility vis = parseVisibility();

    bool isStatic   = false;
    bool isAbstract = false;
    bool isAsync    = false;
    bool isGenerator= false;
    bool isFinal    = false;

    if (match(TokenKind::KW_static))   isStatic = true;
    if (match(TokenKind::KW_abstract)) isAbstract = true;
    if (current().kind() == TokenKind::KW_async) {
        isAsync = true; advance();
    }

    TokenKind k = current().kind();

    if (k == TokenKind::KW_fn) {
        advance();
        if (check(TokenKind::Star)) { isGenerator = true; advance(); }
        return parseMemberFn(vis, isStatic, isAsync, isGenerator, isAbstract || inInterface);
    }

    if (k == TokenKind::KW_init) {
        advance();
        return parseInitDecl(vis);
    }

    if (k == TokenKind::KW_deinit) {
        advance();
        return parseDeinitDecl();
    }

    if (k == TokenKind::KW_prop) {
        advance();
        return parsePropDecl(vis, isStatic);
    }

    if (k == TokenKind::KW_op) {
        advance();
        return parseOperatorDecl(vis);
    }

    if (k == TokenKind::KW_var || k == TokenKind::KW_let) {
        advance();
        return parseFieldDecl(vis, isStatic);
    }

    // Identifier as field name (bare field)
    if (k == TokenKind::Identifier) {
        return parseFieldDecl(vis, isStatic);
    }

    expectedError("member declaration (fn, init, deinit, prop, op, or field)");
    advance();
    return ParseResult<DeclNode*>::error();
}

ParseResult<FnDecl*> Parser::parseMemberFn(Visibility vis, bool isStatic,
                                             bool isAsync, bool isGenerator,
                                             bool isAbstract) {
    auto loc = currentLoc();
    if (!check(TokenKind::Identifier)) {
        expectedError("method name");
        return ParseResult<FnDecl*>::error();
    }

    auto* fn = ctx_.make<FnDecl>(loc);
    fn->name       = ctx_.intern(current().text());
    fn->vis        = (vis == Visibility::Unspecified) ? (isStatic ? Visibility::Pub : Visibility::Pub) : vis;
    fn->isStatic   = isStatic;
    fn->isAsync    = isAsync;
    fn->isGenerator= isGenerator;
    fn->isAbstract = isAbstract;
    advance();

    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) fn->generics = gp.get();
    }

    auto params = parseParamList();
    for (auto& p : params) {
        auto* param = ctx_.make<Param>(p.loc);
        param->name         = ctx_.intern(p.name);
        param->type         = p.type;
        param->defaultValue = p.defVal;
        param->isSelf       = p.isSelf;
        param->isVariadic   = p.isVariadic;
        fn->params.push_back(param);
    }

    fn->returnType = parseReturnType();

    if (check(TokenKind::KW_where)) {
        auto wc = parseWhereClause();
        if (wc.ok()) fn->where_ = wc.get();
    }

    if (isAbstract) {
        // No body expected
        return ParseResult<FnDecl*>(fn);
    }

    if (match(TokenKind::Assign)) {
        auto expr = parseExpr();
        if (!expr.ok()) return ParseResult<FnDecl*>::error();
        auto* block = ctx_.make<BlockStmt>(loc);
        block->trailingExpr = expr.get();
        fn->body = block;
    } else if (check(TokenKind::LBrace)) {
        auto block = parseBlock();
        if (!block.ok()) return ParseResult<FnDecl*>::error();
        fn->body = block.get();
    } else {
        fn->isAbstract = true;
    }

    return ParseResult<FnDecl*>(fn);
}

// ── Init / Deinit ─────────────────────────────────────────────────────────────

ParseResult<InitDecl*> Parser::parseInitDecl(Visibility vis) {
    auto loc = currentLoc();
    auto* decl = ctx_.make<InitDecl>(loc);
    decl->vis = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;

    auto params = parseParamList();
    for (auto& p : params) {
        auto* param = ctx_.make<Param>(p.loc);
        param->name = ctx_.intern(p.name);
        param->type = p.type;
        param->isSelf = p.isSelf;
        decl->params.push_back(param);
    }

    if (match(TokenKind::Assign)) {
        // Short-form: init(x, y) = auto-assigns
        decl->isShortForm = true;
        // Consume optional expression (short form body)
        if (canStartExpr(current().kind())) {
            parseExpr(); // discard — sema generates assignments
        }
    } else if (check(TokenKind::LBrace)) {
        auto block = parseBlock();
        if (block.ok()) decl->body = block.get();
    }

    return ParseResult<InitDecl*>(decl);
}

ParseResult<DeinitDecl*> Parser::parseDeinitDecl() {
    auto loc = currentLoc();
    auto* decl = ctx_.make<DeinitDecl>(loc);

    if (check(TokenKind::LBrace)) {
        auto block = parseBlock();
        if (block.ok()) decl->body = block.get();
    }

    return ParseResult<DeinitDecl*>(decl);
}

// ── Field / Prop / Operator ───────────────────────────────────────────────────

ParseResult<FieldDecl*> Parser::parseFieldDecl(Visibility vis, bool isStatic) {
    auto loc = currentLoc();

    if (!check(TokenKind::Identifier)) {
        expectedError("field name");
        return ParseResult<FieldDecl*>::error();
    }

    auto* decl = ctx_.make<FieldDecl>(loc);
    decl->name     = ctx_.intern(current().text());
    decl->vis      = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    decl->isStatic = isStatic;
    advance();

    if (match(TokenKind::Colon)) {
        auto t = parseType();
        if (t.ok()) decl->type = t.get();
    }

    if (match(TokenKind::Assign)) {
        auto expr = parseExpr();
        if (expr.ok()) decl->defaultValue = expr.get();
    }

    return ParseResult<FieldDecl*>(decl);
}

ParseResult<PropDecl*> Parser::parsePropDecl(Visibility vis, bool isStatic) {
    auto loc = currentLoc();

    if (!check(TokenKind::Identifier)) {
        expectedError("property name");
        return ParseResult<PropDecl*>::error();
    }

    auto* decl = ctx_.make<PropDecl>(loc);
    decl->name     = ctx_.intern(current().text());
    decl->vis      = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    decl->isStatic = isStatic;
    advance();

    if (match(TokenKind::Colon)) {
        auto t = parseType();
        if (t.ok()) decl->type = t.get();
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open property body"))
        return ParseResult<PropDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        if (check(TokenKind::Identifier) && current().text() == "get") {
            advance();
            if (match(TokenKind::FatArrow)) {
                auto expr = parseExpr();
                if (expr.ok()) decl->getter = expr.get();
            } else if (check(TokenKind::LBrace)) {
                auto block = parseBlock();
                if (block.ok()) decl->getterBlock = block.get();
            }
        } else if (check(TokenKind::Identifier) && current().text() == "set") {
            advance();
            if (match(TokenKind::LParen)) {
                auto* p = ctx_.make<Param>(currentLoc());
                p->name = ctx_.intern(current().text());
                advance();
                expectConsume(TokenKind::RParen);
                decl->setterParam = p;
            }
            if (match(TokenKind::FatArrow)) {
                auto expr = parseExpr();
                if (expr.ok()) decl->setter = expr.get();
            } else if (check(TokenKind::LBrace)) {
                auto block = parseBlock();
                if (block.ok()) decl->setterBlock = block.get();
            }
        } else {
            advance(); // skip unknown tokens
        }
    }

    expectConsume(TokenKind::RBrace);
    if (!decl->getter && !decl->getterBlock) decl->isReadOnly = true;
    return ParseResult<PropDecl*>(decl);
}

ParseResult<OperatorDecl*> Parser::parseOperatorDecl(Visibility vis) {
    auto loc = currentLoc();
    auto* decl = ctx_.make<OperatorDecl>(loc);
    decl->vis = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;

    // Operator name: +, -, *, [], ()=, str(), etc.
    std::string opName;
    if (check(TokenKind::LBracket)) {
        advance();
        if (match(TokenKind::RBracket)) {
            if (match(TokenKind::Assign)) opName = "[]=";
            else opName = "[]";
        }
    } else if (check(TokenKind::LParen)) {
        advance(); expectConsume(TokenKind::RParen);
        opName = "()";
    } else if (check(TokenKind::Identifier)) {
        // str() or other named ops
        opName = std::string(current().text());
        advance();
        if (match(TokenKind::LParen)) {
            expectConsume(TokenKind::RParen);
            opName += "()";
        }
    } else {
        // Symbolic op: + - * / == < etc.
        opName = std::string(tokenKindName(current().kind()));
        advance();
    }

    decl->opName = ctx_.intern(opName);

    auto params = parseParamList();
    for (auto& p : params) {
        auto* param = ctx_.make<Param>(p.loc);
        param->name = ctx_.intern(p.name);
        param->type = p.type;
        decl->params.push_back(param);
    }

    decl->returnType = parseReturnType();

    if (match(TokenKind::Assign)) {
        auto expr = parseExpr();
        if (expr.ok()) decl->bodyExpr = expr.get();
    } else if (check(TokenKind::LBrace)) {
        auto block = parseBlock();
        if (block.ok()) decl->body = block.get();
    }

    return ParseResult<OperatorDecl*>(decl);
}

// ── Type alias / Distinct / Var / Const / Bitfield ───────────────────────────

ParseResult<TypeAliasDecl*> Parser::parseTypeAliasDecl(Visibility vis) {
    auto loc = currentLoc();
    if (!check(TokenKind::Identifier)) {
        expectedError("type alias name");
        return ParseResult<TypeAliasDecl*>::error();
    }

    auto* decl = ctx_.make<TypeAliasDecl>(loc);
    decl->name = ctx_.intern(current().text());
    decl->vis  = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    advance();

    if (check(TokenKind::Less)) {
        auto gp = parseGenericParams();
        if (gp.ok()) decl->generics = gp.get();
    }

    if (!expectConsume(TokenKind::Assign, "expected '=' in type alias"))
        return ParseResult<TypeAliasDecl*>::error();

    auto t = parseType();
    if (!t.ok()) return ParseResult<TypeAliasDecl*>::error();
    decl->aliasOf = t.get();

    return ParseResult<TypeAliasDecl*>(decl);
}

ParseResult<DistinctDecl*> Parser::parseDistinctDecl(Visibility vis) {
    auto loc = currentLoc();
    // consume 'type' keyword if present
    if (check(TokenKind::KW_type)) advance();

    if (!check(TokenKind::Identifier)) {
        expectedError("distinct type name");
        return ParseResult<DistinctDecl*>::error();
    }

    auto* decl = ctx_.make<DistinctDecl>(loc);
    decl->name = ctx_.intern(current().text());
    decl->vis  = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    advance();

    if (!expectConsume(TokenKind::Assign, "expected '=' in distinct type"))
        return ParseResult<DistinctDecl*>::error();

    auto t = parseType();
    if (!t.ok()) return ParseResult<DistinctDecl*>::error();
    decl->baseType = t.get();

    return ParseResult<DistinctDecl*>(decl);
}

ParseResult<VarDecl*> Parser::parseVarDecl(Visibility vis) {
    auto loc = currentLoc();
    bool isMutable = true;
    bool isLet     = false;

    // Determine mutability from keyword context
    // Called after 'var' or 'let' has been consumed in parseTopLevelDecl
    // We use vis to distinguish (hacky but workable at this level)

    if (!check(TokenKind::Identifier)) {
        expectedError("variable name");
        return ParseResult<VarDecl*>::error();
    }

    auto* decl = ctx_.make<VarDecl>(loc);
    decl->name      = ctx_.intern(current().text());
    decl->vis       = (vis == Visibility::Unspecified) ? Visibility::Priv : vis;
    decl->isMutable = isMutable;
    advance();

    if (match(TokenKind::Colon)) {
        auto t = parseType();
        if (t.ok()) decl->type = t.get();
    }

    if (match(TokenKind::Assign) || match(TokenKind::ColonAssign)) {
        auto expr = parseExpr();
        if (expr.ok()) decl->initializer = expr.get();
    }

    return ParseResult<VarDecl*>(decl);
}

ParseResult<ConstDecl*> Parser::parseConstDecl(Visibility vis) {
    auto loc = currentLoc();
    if (!check(TokenKind::Identifier)) {
        expectedError("constant name");
        return ParseResult<ConstDecl*>::error();
    }

    auto* decl = ctx_.make<ConstDecl>(loc);
    decl->name = ctx_.intern(current().text());
    decl->vis  = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    advance();

    if (match(TokenKind::Colon)) {
        auto t = parseType();
        if (t.ok()) decl->type = t.get();
    }

    if (match(TokenKind::Assign) || match(TokenKind::ColonAssign)) {
        auto expr = parseExpr();
        if (expr.ok()) decl->value = expr.get();
    }

    return ParseResult<ConstDecl*>(decl);
}

ParseResult<BitfieldDecl*> Parser::parseBitfieldDecl(Visibility vis) {
    auto loc = currentLoc();
    // 'bitfield' already consumed; next: 'struct'
    if (check(TokenKind::KW_struct)) advance();

    if (!check(TokenKind::Identifier)) {
        expectedError("bitfield name");
        return ParseResult<BitfieldDecl*>::error();
    }

    auto* decl = ctx_.make<BitfieldDecl>(loc);
    decl->name = ctx_.intern(current().text());
    decl->vis  = (vis == Visibility::Unspecified) ? Visibility::Pub : vis;
    advance();

    if (match(TokenKind::Colon)) {
        auto t = parseType();
        if (t.ok()) decl->baseType = t.get();
    }

    if (!expectConsume(TokenKind::LBrace, "expected '{' to open bitfield body"))
        return ParseResult<BitfieldDecl*>::error();

    while (!check(TokenKind::RBrace) && !atEnd()) {
        BitfieldField bf;
        bf.loc = currentLoc();

        if (check(TokenKind::Identifier)) {
            bf.name = ctx_.intern(current().text());
            advance();
        } else if (check(TokenKind::KW_let) || check(TokenKind::Identifier)) {
            bf.name = "_";
            advance();
        }

        if (match(TokenKind::Colon)) {
            auto bits = parseExpr();
            // We need a literal integer — extract from expr
            if (bits.ok() && bits.get()->is(ASTNodeKind::IntLitExpr)) {
                bf.bits = static_cast<uint32_t>(bits.get()->as<IntLitExpr>()->value);
            }
        }
        decl->fields.push_back(bf);
        match(TokenKind::Comma);
        match(TokenKind::Semicolon);
    }

    expectConsume(TokenKind::RBrace);
    return ParseResult<BitfieldDecl*>(decl);
}

} // namespace vex