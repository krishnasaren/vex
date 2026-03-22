#pragma once
// ============================================================================
// vex/Lexer/Token.h
// A single token produced by the Lexer.
// Compact — 32 bytes per token. Stores kind, location, and raw text.
// ============================================================================

#include "vex/Lexer/TokenKind.h"
#include "vex/Core/SourceLocation.h"

#include <cstdint>
#include <string_view>

namespace vex {

// ── IntSuffix / FloatSuffix ───────────────────────────────────────────────────
// Numeric literal suffix information stored inside a token.

enum class IntSuffix : uint8_t {
    None,       // default → int (int64 on 64-bit)
    L,          // L   → int64
    u,          // u   → uint
    u8,         // u8  → uint8
    u16,        // u16 → uint16
    u32,        // u32 → uint32
    u64,        // u64 → uint64
    i8,         // i8  → int8
    i16,        // i16 → int16
    i32,        // i32 → int32
    i64,        // i64 → int64
    // Duration suffixes
    ms,         // milliseconds → Duration
    s,          // seconds      → Duration
    us,         // microseconds → Duration
    ns,         // nanoseconds  → Duration
};

enum class FloatSuffix : uint8_t {
    None,   // float64
    f,      // float32
};

// ── Token ─────────────────────────────────────────────────────────────────────

class Token {
public:
    Token() = default;

    Token(TokenKind kind, SourceLocation loc, std::string_view text)
        : kind_(kind), loc_(loc), text_(text) {}

    // ── Accessors ────────────────────────────────────────────────────────────

    TokenKind      kind()     const { return kind_; }
    SourceLocation location() const { return loc_; }

    // Raw source text of this token (points into the MemoryBuffer — zero-copy)
    std::string_view text() const { return text_; }

    // True if this is an error token
    bool isError() const { return kind_ == TokenKind::Error; }
    bool isEof()   const { return kind_ == TokenKind::Eof; }

    // True if the token has the given kind
    bool is(TokenKind k) const { return kind_ == k; }

    // True if the token matches any of the given kinds
    template<typename... Kinds>
    bool isAny(Kinds... kinds) const {
        return ((kind_ == kinds) || ...);
    }

    // True if the token is a keyword
    bool isKeyword() const { return vex::isKeyword(kind_); }

    // True if the token is a literal
    bool isLiteral() const { return vex::isLiteral(kind_); }

    // ── Integer literal extras ────────────────────────────────────────────────

    IntSuffix   intSuffix()   const { return intSuffix_; }
    FloatSuffix floatSuffix() const { return floatSuffix_; }

    void setIntSuffix(IntSuffix s)     { intSuffix_ = s; }
    void setFloatSuffix(FloatSuffix s) { floatSuffix_ = s; }

    // Integer base (2, 8, 10, 16)
    uint8_t intBase() const { return intBase_; }
    void    setIntBase(uint8_t b) { intBase_ = b; }

    // ── Factory helpers ───────────────────────────────────────────────────────

    static Token makeEof(SourceLocation loc) {
        return Token(TokenKind::Eof, loc, {});
    }

    static Token makeError(SourceLocation loc, std::string_view text) {
        return Token(TokenKind::Error, loc, text);
    }

    // Source range of this token: [loc, loc+text.size())
    SourceRange range() const {
        // end location is on the same line for single-line tokens
        SourceLocation end(loc_.fileID(), loc_.line(),
                           loc_.col() + static_cast<uint32_t>(text_.size()));
        return SourceRange(loc_, end);
    }

private:
    TokenKind      kind_        = TokenKind::Eof;
    SourceLocation loc_;
    std::string_view text_;           // view into MemoryBuffer — NOT owned
    IntSuffix      intSuffix_   = IntSuffix::None;
    FloatSuffix    floatSuffix_ = FloatSuffix::None;
    uint8_t        intBase_     = 10;
};

} // namespace vex