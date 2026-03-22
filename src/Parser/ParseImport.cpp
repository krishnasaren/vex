// src/Parser/ParseImport.cpp
// ============================================================================
// vex/Parser/ParseImport.cpp
// Import statement parsing (all three forms).
// ============================================================================

#include "vex/Parser/Parser.h"

namespace vex {

ParseResult<ImportDecl*> Parser::parseImport() {
    auto loc = currentLoc();
    auto* decl = ctx_.make<ImportDecl>(loc);

    // Guard: :: not valid outside import
    inImportContext_ = true;

    // Parse module path: std::io  or  mylib::shapes
    auto segments = parseImportPath();
    if (segments.empty()) {
        inImportContext_ = false;
        return ParseResult<ImportDecl*>::error();
    }

    for (auto& s : segments)
        decl->pathSegments.push_back(ctx_.intern(s));

    // Default alias = last segment
    decl->alias = decl->pathSegments.back();

    // Form 1 with alias: import std::io as myio
    if (match(TokenKind::KW_as)) {
        if (!check(TokenKind::Identifier)) {
            expectedError("alias name after 'as'");
            inImportContext_ = false;
            return ParseResult<ImportDecl*>::error();
        }
        // Disallow 'mod' as alias
        if (check(TokenKind::KW_mod)) {
            diags_.report(DiagID::SEMA_ColonColonOutsideImport, currentLoc(),
                          "'mod' is a reserved keyword and cannot be used as an import alias");
            advance();
            inImportContext_ = false;
            return ParseResult<ImportDecl*>::error();
        }
        decl->alias = ctx_.intern(current().text());
        decl->hasExplicitAlias = true;
        advance();
    }
    // Form 2/3: selective import { name1, name2 }
    else if (check(TokenKind::LBrace)) {
        auto items = parseSelectiveImportList();

        if (!items.empty() && items[0].isWildcard) {
            decl->isOpenImport = true;
        } else {
            for (auto& item : items) {
                SelectiveImportItem si;
                si.originalName = ctx_.intern(item.originalName);
                si.localName    = ctx_.intern(item.localName);
                si.isWildcard   = item.isWildcard;
                si.loc          = loc;
                decl->selectiveItems.push_back(si);
            }
        }
    }

    inImportContext_ = false;
    return ParseResult<ImportDecl*>(decl);
}

} // namespace vex