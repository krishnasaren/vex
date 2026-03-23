#pragma once
// include/vex/Parser/ParseContext.h

#pragma once
// ============================================================================
// vex/Parser/ParseContext.h  — Shared context passed through all parse methods
// ============================================================================
#include "vex/Lexer/TokenStream.h"
#include "vex/Core/DiagnosticEngine.h"
#include "vex/AST/ASTContext.h"
#include <string_view>

namespace vex {

class ParseContext {
public:
    ParseContext(TokenStream& tokens,
                 ASTContext& astCtx,
                 DiagnosticEngine& diags)
        : tokens_(tokens), astCtx_(astCtx), diags_(diags) {}

    TokenStream&      tokens()  { return tokens_; }
    ASTContext&        astCtx()  { return astCtx_; }
    DiagnosticEngine&  diags()   { return diags_; }

    // Convenience helpers
    const Token& peek(size_t la = 0) const { return tokens_.peek(la); }
    Token        consume()                 { return tokens_.consume(); }
    bool         consumeIf(TokenKind k)    { return tokens_.consumeIf(k); }
    bool         atEnd()            const  { return tokens_.atEnd(); }
    SourceLocation currentLoc()     const  { return tokens_.current().loc; }

    // Expect: consume if matches, otherwise emit error and return false
    bool expect(TokenKind k);
    bool expectIdent(std::string_view* outName = nullptr);

    // Synchronize after error to next statement boundary
    void synchronize();

    bool hasErrors() const { return diags_.hasErrors(); }

private:
    TokenStream&      tokens_;
    ASTContext&        astCtx_;
    DiagnosticEngine&  diags_;
};

} // namespace vex


