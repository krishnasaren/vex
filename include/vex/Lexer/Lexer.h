#pragma once
// ============================================================================
// vex/Lexer/Lexer.h
// The VEX language lexer (tokenizer).
// Converts raw source text into a stream of Token objects.
//
// Rules from Chapter 3 (Lexical Rules) of vex_final_rules.vex:
//   - Identifiers: [a-zA-Z_][a-zA-Z0-9_]*
//   - Keywords: all listed in Chapter 3
//   - :: only valid in import statements (flagged by parser, not lexer)
//   - Statements terminated by }, ; or logical line completeness
//   - Comments: // single-line  /* multi-line */  /// doc comment
//   - Whitespace: spaces, tabs, newlines — all ignored between tokens
//   - Newlines do NOT act as statement terminators
// ============================================================================

#include "vex/Lexer/Token.h"
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/MemoryBuffer.h"
#include "vex/Core/SourceManager.h"

#include <vector>

namespace vex {

// ── Lexer ─────────────────────────────────────────────────────────────────────

class Lexer {
public:
    // Create a Lexer for a source buffer.
    // srcMgr and diags must outlive the Lexer.
    Lexer(const MemoryBuffer& buffer,
          FileID              fileID,
          SourceManager&      srcMgr,
          DiagnosticEngine&   diags);

    // Lex all tokens from the buffer into a vector.
    // The last token is always Eof.
    std::vector<Token> lexAll();

    // Lex one token and advance. Returns Eof at end of file.
    Token nextToken();

    // Peek at the current character without advancing.
    char peek(uint32_t offset = 0) const;

    // Current byte offset into the buffer.
    uint32_t currentOffset() const {
        return static_cast<uint32_t>(cur_ - start_);
    }

private:
    // Source
    const char*       start_;   // start of buffer
    const char*       end_;     // one past end of buffer
    const char*       cur_;     // current read position
    FileID            fileID_;
    SourceManager&    srcMgr_;
    DiagnosticEngine& diags_;

    // ── Low-level character helpers ──────────────────────────────────────────

    bool atEnd() const { return cur_ >= end_; }
    char advance();
    char peekAt(uint32_t offset) const;
    bool match(char expected);
    bool matchStr(std::string_view s);

    SourceLocation currentLoc() const;
    SourceLocation locAt(const char* ptr) const;

    // ── Token builders ────────────────────────────────────────────────────────

    Token makeToken(TokenKind kind, const char* tokenStart) const;
    Token makeError(const char* tokenStart, std::string_view msg);

    // ── Whitespace & comments ─────────────────────────────────────────────────

    void skipWhitespace();
    bool skipLineComment();    // // ... and /// ...
    bool skipBlockComment();   // /* ... */

    // ── Lexing methods — one per token category ───────────────────────────────

    Token lexIdentifierOrKeyword(const char* tokenStart);
    Token lexNumber(const char* tokenStart);
    Token lexString(const char* tokenStart);
    Token lexRawString(const char* tokenStart);
    Token lexFmtString(const char* tokenStart);
    Token lexMultilineString(const char* tokenStart);
    Token lexChar(const char* tokenStart);
    Token lexSymbol(const char* tokenStart);

    // ── Numeric literal helpers ───────────────────────────────────────────────

    void    lexDecimalDigits();
    void    lexHexDigits();
    void    lexBinaryDigits();
    void    lexOctalDigits();
    IntSuffix    parseIntSuffix();
    FloatSuffix  parseFloatSuffix();

    // ── String literal helpers ────────────────────────────────────────────────

    // Scan and validate escape sequence. Returns false on error.
    bool validateEscapeSeq();
    bool validateUnicodeEscape();

    // ── Identifier/keyword length in bytes (ASCII + UTF-8 safe) ──────────────
    static bool isIdentStart(char c);
    static bool isIdentCont(char c);
    static bool isDigit(char c) { return c >= '0' && c <= '9'; }
    static bool isHexDigit(char c);
    static bool isWhitespace(char c);
};

} // namespace vex