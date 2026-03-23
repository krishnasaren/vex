#pragma once
// include/vex/Lexer/TokenStream.h


// ============================================================================
// vex/Lexer/TokenStream.h  — Buffered peek-able token stream
// ============================================================================
#include "vex/Lexer/Token.h"
#include <vector>
#include <cassert>

namespace vex {

class TokenStream {
public:
    explicit TokenStream(std::vector<Token> tokens)
        : tokens_(std::move(tokens)), pos_(0) {}

    // Current token
    const Token& peek(size_t lookahead = 0) const {
        size_t idx = pos_ + lookahead;
        if (idx >= tokens_.size()) return tokens_.back(); // EOF
        return tokens_[idx];
    }

    const Token& current() const { return peek(0); }

    // Consume and return current token
    Token consume() {
        if (pos_ < tokens_.size()) return tokens_[pos_++];
        return tokens_.back();
    }

    // Consume if current matches kind
    bool consumeIf(TokenKind k) {
        if (current().kind == k) { consume(); return true; }
        return false;
    }

    bool   atEnd()  const { return pos_ >= tokens_.size() || current().kind == TokenKind::Eof; }
    size_t position() const { return pos_; }
    void   reset(size_t pos) { assert(pos <= tokens_.size()); pos_ = pos; }
    size_t size()   const { return tokens_.size(); }

private:
    std::vector<Token> tokens_;
    size_t             pos_;
};

} // namespace vex
