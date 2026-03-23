#pragma once
// include/vex/Parser/RecoveryStrategy.h


// ============================================================================
// vex/Parser/RecoveryStrategy.h  — Error recovery helpers for the parser
// ============================================================================
#include "vex/Parser/ParseContext.h"
#include "vex/Lexer/TokenKind.h"
#include <initializer_list>

namespace vex {

class RecoveryStrategy {
public:
    explicit RecoveryStrategy(ParseContext& ctx) : ctx_(ctx) {}

    // Skip tokens until one in 'syncSet' or EOF
    void skipUntil(std::initializer_list<TokenKind> syncSet);

    // Skip a balanced pair, e.g. { } or ( )
    void skipBalanced(TokenKind open, TokenKind close);

    // Skip to next statement boundary (semicolon, }, EOF)
    void skipToNextStatement();

    // Skip to next top-level declaration
    void skipToNextDecl();

private:
    ParseContext& ctx_;
};

} // namespace vex
