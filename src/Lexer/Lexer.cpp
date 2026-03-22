// ============================================================================
// vex/Lexer/Lexer.cpp
// Complete VEX language lexer.
// Implements all lexical rules from Chapter 3 of vex_final_rules.vex.
// ============================================================================

#include "vex/Lexer/Lexer.h"
#include "vex/Core/Assert.h"

#include <cstring>
#include <cctype>

namespace vex {

// ── Constructor ───────────────────────────────────────────────────────────────

Lexer::Lexer(const MemoryBuffer& buffer,
             FileID              fileID,
             SourceManager&      srcMgr,
             DiagnosticEngine&   diags)
    : start_(buffer.data())
    , end_(buffer.data() + buffer.size())
    , cur_(buffer.data())
    , fileID_(fileID)
    , srcMgr_(srcMgr)
    , diags_(diags)
{}

// ── Public interface ──────────────────────────────────────────────────────────

std::vector<Token> Lexer::lexAll() {
    std::vector<Token> tokens;
    tokens.reserve(1024);

    while (true) {
        Token tok = nextToken();
        tokens.push_back(tok);
        if (tok.isEof()) break;
    }
    return tokens;
}

Token Lexer::nextToken() {
    skipWhitespace();

    if (atEnd())
        return Token::makeEof(currentLoc());

    const char* tokenStart = cur_;
    char        c          = advance();

    // ── Identifiers and keywords ──────────────────────────────────────────────
    if (isIdentStart(c))
        return lexIdentifierOrKeyword(tokenStart);

    // ── Numeric literals ──────────────────────────────────────────────────────
    if (isDigit(c))
        return lexNumber(tokenStart);

    // ── String literals ───────────────────────────────────────────────────────
    if (c == '"')  return lexString(tokenStart);
    if (c == '`')  return lexMultilineString(tokenStart);
    if (c == '\'') return lexChar(tokenStart);

    // r"..." raw string — handled in lexSymbol via 'r' identifier check
    // f"..." fmt string — handled in lexSymbol via 'f' identifier check

    // ── Symbols and operators ─────────────────────────────────────────────────
    return lexSymbol(tokenStart);
}

// ── Low-level helpers ─────────────────────────────────────────────────────────

char Lexer::advance() {
    VEX_ASSERT(!atEnd(), "advance() called at end of buffer");
    return *cur_++;
}

char Lexer::peek(uint32_t offset) const {
    return peekAt(offset);
}

char Lexer::peekAt(uint32_t offset) const {
    const char* p = cur_ + offset;
    return (p < end_) ? *p : '\0';
}

bool Lexer::match(char expected) {
    if (cur_ < end_ && *cur_ == expected) {
        ++cur_;
        return true;
    }
    return false;
}

bool Lexer::matchStr(std::string_view s) {
    if ((size_t)(end_ - cur_) < s.size()) return false;
    if (std::strncmp(cur_, s.data(), s.size()) != 0) return false;
    cur_ += s.size();
    return true;
}

SourceLocation Lexer::currentLoc() const {
    return srcMgr_.getLocation(fileID_, cur_);
}

SourceLocation Lexer::locAt(const char* ptr) const {
    return srcMgr_.getLocation(fileID_, ptr);
}

Token Lexer::makeToken(TokenKind kind, const char* tokenStart) const {
    std::string_view text(tokenStart, static_cast<size_t>(cur_ - tokenStart));
    return Token(kind, srcMgr_.getLocation(fileID_, tokenStart), text);
}

Token Lexer::makeError(const char* tokenStart, std::string_view msg) {
    SourceLocation loc = locAt(tokenStart);
    diags_.emitSimple(DiagID::LEX_UnknownChar, loc, std::string(msg));
    std::string_view text(tokenStart, static_cast<size_t>(cur_ - tokenStart));
    return Token::makeError(loc, text);
}

// ── Whitespace and comments ───────────────────────────────────────────────────

bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

void Lexer::skipWhitespace() {
    while (!atEnd()) {
        char c = *cur_;

        // Whitespace (spaces, tabs, newlines — Chapter 3: all ignored)
        if (isWhitespace(c)) {
            ++cur_;
            continue;
        }

        // Single-line comment: // or ///
        if (c == '/' && peekAt(1) == '/') {
            skipLineComment();
            continue;
        }

        // Block comment: /* ... */
        if (c == '/' && peekAt(1) == '*') {
            if (!skipBlockComment())
                return; // error was emitted
            continue;
        }

        break;
    }
}

bool Lexer::skipLineComment() {
    // Skip until end of line
    while (!atEnd() && *cur_ != '\n')
        ++cur_;
    return true;
}

bool Lexer::skipBlockComment() {
    const char* start = cur_;
    cur_ += 2; // skip /*

    // Support nested block comments
    int depth = 1;
    while (!atEnd() && depth > 0) {
        if (*cur_ == '/' && peekAt(1) == '*') {
            cur_ += 2;
            ++depth;
        } else if (*cur_ == '*' && peekAt(1) == '/') {
            cur_ += 2;
            --depth;
        } else {
            ++cur_;
        }
    }

    if (depth > 0) {
        diags_.emitSimple(DiagID::LEX_UnterminatedComment,
                          locAt(start),
                          "unterminated block comment");
        return false;
    }
    return true;
}

// ── Identifiers and keywords ──────────────────────────────────────────────────

bool Lexer::isIdentStart(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Lexer::isIdentCont(char c) {
    return isIdentStart(c) || (c >= '0' && c <= '9');
}

Token Lexer::lexIdentifierOrKeyword(const char* tokenStart) {
    // tokenStart[-1] is already consumed by advance() in nextToken()
    // Continue consuming identifier characters
    while (!atEnd() && isIdentCont(*cur_))
        ++cur_;

    std::string_view text(tokenStart, static_cast<size_t>(cur_ - tokenStart));

    // Special case: r"..." raw string
    if (text == "r" && !atEnd() && *cur_ == '"') {
        ++cur_; // consume the opening "
        return lexRawString(tokenStart);
    }

    // Special case: f"..." format string
    if (text == "f" && !atEnd() && *cur_ == '"') {
        ++cur_; // consume the opening "
        return lexFmtString(tokenStart);
    }

    // Keyword lookup — covers all keywords including `mod` (new keyword)
    TokenKind kind = lookupKeyword(text);
    SourceLocation loc = locAt(tokenStart);
    return Token(kind, loc, text);
}

// ── Numeric literals ──────────────────────────────────────────────────────────

bool Lexer::isHexDigit(char c) {
    return isDigit(c) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

void Lexer::lexDecimalDigits() {
    while (!atEnd() && (isDigit(*cur_) || *cur_ == '_'))
        ++cur_;
}

void Lexer::lexHexDigits() {
    while (!atEnd() && (isHexDigit(*cur_) || *cur_ == '_'))
        ++cur_;
}

void Lexer::lexBinaryDigits() {
    while (!atEnd() && (*cur_ == '0' || *cur_ == '1' || *cur_ == '_'))
        ++cur_;
}

void Lexer::lexOctalDigits() {
    while (!atEnd() && ((*cur_ >= '0' && *cur_ <= '7') || *cur_ == '_'))
        ++cur_;
}

IntSuffix Lexer::parseIntSuffix() {
    // Check suffixes: L u u8 u16 u32 u64 i8 i16 i32 i64 ms s us ns
    if (matchStr("u64"))  return IntSuffix::u64;
    if (matchStr("u32"))  return IntSuffix::u32;
    if (matchStr("u16"))  return IntSuffix::u16;
    if (matchStr("i64"))  return IntSuffix::i64;
    if (matchStr("i32"))  return IntSuffix::i32;
    if (matchStr("i16"))  return IntSuffix::i16;
    if (matchStr("i8"))   return IntSuffix::i8;
    if (matchStr("u8"))   return IntSuffix::u8;
    if (matchStr("us"))   return IntSuffix::us;
    if (matchStr("ns"))   return IntSuffix::ns;
    if (matchStr("ms"))   return IntSuffix::ms;
    if (*cur_ == 'u') { ++cur_; return IntSuffix::u; }
    if (*cur_ == 'L') { ++cur_; return IntSuffix::L; }
    if (*cur_ == 's') { ++cur_; return IntSuffix::s; }
    return IntSuffix::None;
}

FloatSuffix Lexer::parseFloatSuffix() {
    if (!atEnd() && *cur_ == 'f') {
        ++cur_;
        return FloatSuffix::f;
    }
    return FloatSuffix::None;
}

Token Lexer::lexNumber(const char* tokenStart) {
    // tokenStart[-1] is the first digit, already consumed
    char first = *(cur_ - 1);

    bool isFloat = false;
    uint8_t base = 10;

    if (first == '0' && !atEnd()) {
        char next = *cur_;
        if (next == 'x' || next == 'X') {
            // Hexadecimal: 0xFF
            ++cur_;
            base = 16;
            lexHexDigits();
        } else if (next == 'b' || next == 'B') {
            // Binary: 0b1010
            ++cur_;
            base = 2;
            lexBinaryDigits();
        } else if (next == 'o' || next == 'O') {
            // Octal: 0o755
            ++cur_;
            base = 8;
            lexOctalDigits();
        } else {
            lexDecimalDigits();
        }
    } else {
        lexDecimalDigits();
    }

    // Check for float: digits . digits (e±digits)?
    if (base == 10 && !atEnd() && *cur_ == '.') {
        char after = peekAt(1);
        // Don't treat 1..10 as float — .. is the range operator
        if (after != '.' && (isDigit(after) || after == 'e' || after == 'E')) {
            isFloat = true;
            ++cur_; // consume '.'
            lexDecimalDigits();
        }
    }

    // Exponent part: e±digits
    if (base == 10 && !atEnd() && (*cur_ == 'e' || *cur_ == 'E')) {
        isFloat = true;
        ++cur_;
        if (!atEnd() && (*cur_ == '+' || *cur_ == '-'))
            ++cur_;
        lexDecimalDigits();
    }

    Token tok = makeToken(isFloat ? TokenKind::FloatLiteral : TokenKind::IntLiteral,
                          tokenStart);

    if (isFloat) {
        tok.setFloatSuffix(parseFloatSuffix());
    } else {
        tok.setIntBase(base);
        tok.setIntSuffix(parseIntSuffix());
    }

    return tok;
}

// ── String literals ───────────────────────────────────────────────────────────

bool Lexer::validateEscapeSeq() {
    if (atEnd()) return false;
    char c = advance();
    switch (c) {
        case 'n': case 't': case 'r': case '\\':
        case '"': case '\'': case '0':
            return true;
        case 'u':
            return validateUnicodeEscape();
        default:
            return false;
    }
}

bool Lexer::validateUnicodeEscape() {
    // Expect \u{XXXXXX}
    if (!match('{')) return false;
    if (!isHexDigit(*cur_)) return false;
    while (!atEnd() && isHexDigit(*cur_)) ++cur_;
    return match('}');
}

Token Lexer::lexString(const char* tokenStart) {
    // tokenStart[-1] is the opening " (already consumed)
    while (!atEnd() && *cur_ != '"' && *cur_ != '\n') {
        if (*cur_ == '\\') {
            ++cur_; // consume backslash
            if (!validateEscapeSeq()) {
                diags_.emitSimple(DiagID::LEX_InvalidEscape,
                                  currentLoc(),
                                  "invalid escape sequence");
            }
        } else {
            ++cur_;
        }
    }

    if (atEnd() || *cur_ == '\n') {
        return makeError(tokenStart, "unterminated string literal");
    }

    ++cur_; // consume closing "
    return makeToken(TokenKind::StringLiteral, tokenStart);
}

Token Lexer::lexRawString(const char* tokenStart) {
    // tokenStart is the 'r', cur_ is just past the opening "
    while (!atEnd() && *cur_ != '"' && *cur_ != '\n')
        ++cur_;

    if (atEnd() || *cur_ == '\n') {
        return makeError(tokenStart, "unterminated raw string literal");
    }

    ++cur_; // consume closing "
    return makeToken(TokenKind::RawString, tokenStart);
}

Token Lexer::lexFmtString(const char* tokenStart) {
    // f"..." — we lex the whole thing as one token here.
    // The parser/sema layer will parse the interpolated expressions.
    // We just need to find the matching closing " while tracking {}.
    int braceDepth = 0;
    while (!atEnd()) {
        char c = *cur_;
        if (c == '{') {
            ++cur_;
            ++braceDepth;
        } else if (c == '}') {
            ++cur_;
            if (braceDepth > 0) --braceDepth;
        } else if (c == '"' && braceDepth == 0) {
            ++cur_; // consume closing "
            break;
        } else if (c == '\\' && braceDepth == 0) {
            ++cur_; // consume backslash
            validateEscapeSeq();
        } else if (c == '\n' && braceDepth == 0) {
            return makeError(tokenStart, "unterminated format string literal");
        } else {
            ++cur_;
        }
    }

    return makeToken(TokenKind::FmtString, tokenStart);
}

Token Lexer::lexMultilineString(const char* tokenStart) {
    // `...` — raw multiline string. No escape processing.
    // Strips common indentation — handled by sema layer.
    while (!atEnd() && *cur_ != '`')
        ++cur_;

    if (atEnd()) {
        return makeError(tokenStart, "unterminated multiline string literal");
    }

    ++cur_; // consume closing `
    return makeToken(TokenKind::MultilineStr, tokenStart);
}

Token Lexer::lexChar(const char* tokenStart) {
    // tokenStart[-1] is the opening ' (already consumed)
    if (atEnd() || *cur_ == '\'') {
        return makeError(tokenStart, "empty char literal");
    }

    if (*cur_ == '\\') {
        ++cur_; // consume backslash
        if (!validateEscapeSeq()) {
            diags_.emitSimple(DiagID::LEX_InvalidEscape,
                              currentLoc(),
                              "invalid escape sequence in char literal");
        }
    } else {
        // Consume UTF-8 codepoint (may be multiple bytes)
        unsigned char byte = static_cast<unsigned char>(*cur_);
        ++cur_;
        // Multi-byte UTF-8: consume remaining bytes
        if (byte >= 0xC0) {
            while (!atEnd() && (static_cast<unsigned char>(*cur_) & 0xC0) == 0x80)
                ++cur_;
        }
    }

    if (atEnd() || *cur_ != '\'') {
        return makeError(tokenStart,
                         "char literal has more than one character — use a string literal instead");
    }

    ++cur_; // consume closing '
    return makeToken(TokenKind::CharLiteral, tokenStart);
}

// ── Symbols and operators ─────────────────────────────────────────────────────

Token Lexer::lexSymbol(const char* tokenStart) {
    char c = *(cur_ - 1); // already consumed by advance() in nextToken()

    switch (c) {

    case '(': return makeToken(TokenKind::LParen,    tokenStart);
    case ')': return makeToken(TokenKind::RParen,    tokenStart);
    case '[': return makeToken(TokenKind::LBracket,  tokenStart);
    case ']': return makeToken(TokenKind::RBracket,  tokenStart);
    case '{': return makeToken(TokenKind::LBrace,    tokenStart);
    case '}': return makeToken(TokenKind::RBrace,    tokenStart);
    case ',': return makeToken(TokenKind::Comma,     tokenStart);
    case ';': return makeToken(TokenKind::Semicolon, tokenStart);
    case '@': return makeToken(TokenKind::At,        tokenStart);
    case '#': return makeToken(TokenKind::Hash,      tokenStart);

    case ':':
        if (match(':')) return makeToken(TokenKind::ColonColon, tokenStart);
        if (match('=')) return makeToken(TokenKind::ColonAssign, tokenStart);
        return makeToken(TokenKind::Colon, tokenStart);

    case '.':
        if (match('.')) {
            if (match('=')) return makeToken(TokenKind::DotDotEq, tokenStart);
            if (match('.')) return makeToken(TokenKind::DotDotDot, tokenStart);
            return makeToken(TokenKind::DotDot, tokenStart);
        }
        return makeToken(TokenKind::Dot, tokenStart);

    case '?':
        if (match('.')) return makeToken(TokenKind::QuestionDot,      tokenStart);
        if (match('?')) return makeToken(TokenKind::QuestionQuestion,  tokenStart);
        return makeToken(TokenKind::Question, tokenStart);

    case '=':
        if (match('=')) return makeToken(TokenKind::Eq,      tokenStart);
        if (match('>')) return makeToken(TokenKind::FatArrow, tokenStart);
        return makeToken(TokenKind::Assign, tokenStart);

    case '!':
        if (match('=')) return makeToken(TokenKind::NotEq, tokenStart);
        return makeToken(TokenKind::Bang, tokenStart);

    case '<':
        if (match('<')) {
            if (match('=')) return makeToken(TokenKind::LShiftAssign, tokenStart);
            return makeToken(TokenKind::LShift, tokenStart);
        }
        if (match('=')) return makeToken(TokenKind::LessEq, tokenStart);
        return makeToken(TokenKind::Less, tokenStart);

    case '>':
        if (match('>')) {
            if (match('>')) return makeToken(TokenKind::URShift, tokenStart);
            if (match('=')) return makeToken(TokenKind::RShiftAssign, tokenStart);
            return makeToken(TokenKind::RShift, tokenStart);
        }
        if (match('=')) return makeToken(TokenKind::GreaterEq, tokenStart);
        return makeToken(TokenKind::Greater, tokenStart);

    case '+':
        if (match('%')) {
            if (match('=')) return makeToken(TokenKind::WrapAddAssign, tokenStart);
            return makeToken(TokenKind::WrapAdd, tokenStart);
        }
        if (match('|')) {
            if (match('=')) return makeToken(TokenKind::SatAddAssign, tokenStart);
            return makeToken(TokenKind::SatAdd, tokenStart);
        }
        if (match('+')) return makeToken(TokenKind::PlusPlus,   tokenStart);
        if (match('=')) return makeToken(TokenKind::PlusAssign,  tokenStart);
        return makeToken(TokenKind::Plus, tokenStart);

    case '-':
        if (match('%')) {
            if (match('=')) return makeToken(TokenKind::WrapSubAssign, tokenStart);
            return makeToken(TokenKind::WrapSub, tokenStart);
        }
        if (match('|')) {
            if (match('=')) return makeToken(TokenKind::SatSubAssign, tokenStart);
            return makeToken(TokenKind::SatSub, tokenStart);
        }
        if (match('-')) return makeToken(TokenKind::MinusMinus,  tokenStart);
        if (match('=')) return makeToken(TokenKind::MinusAssign, tokenStart);
        if (match('>')) return makeToken(TokenKind::Arrow,       tokenStart);
        return makeToken(TokenKind::Minus, tokenStart);

    case '*':
        if (match('%')) {
            if (match('=')) return makeToken(TokenKind::WrapMulAssign, tokenStart);
            return makeToken(TokenKind::WrapMul, tokenStart);
        }
        if (match('|')) {
            if (match('=')) return makeToken(TokenKind::SatMulAssign, tokenStart);
            return makeToken(TokenKind::SatMul, tokenStart);
        }
        if (match('*')) {
            if (match('=')) return makeToken(TokenKind::StarStarAssign, tokenStart);
            return makeToken(TokenKind::StarStar, tokenStart);
        }
        if (match('=')) return makeToken(TokenKind::StarAssign, tokenStart);
        return makeToken(TokenKind::Star, tokenStart);

    case '/':
        // Note: // and /* comments are handled in skipWhitespace.
        // If we reach here, it's a plain / or /= operator.
        if (match('=')) return makeToken(TokenKind::SlashAssign, tokenStart);
        if (match('%')) {
            if (match('=')) return makeToken(TokenKind::WrapSubAssign, tokenStart);
            return makeToken(TokenKind::WrapDiv, tokenStart);
        }
        return makeToken(TokenKind::Slash, tokenStart);

    case '%':
        if (match('=')) return makeToken(TokenKind::PercentAssign, tokenStart);
        return makeToken(TokenKind::Percent, tokenStart);

    case '&':
        if (match('&')) return makeToken(TokenKind::AmpAmp,    tokenStart);
        if (match('=')) return makeToken(TokenKind::AmpAssign, tokenStart);
        return makeToken(TokenKind::Amp, tokenStart);

    case '|':
        if (match('|')) return makeToken(TokenKind::PipePipe,   tokenStart);
        if (match('=')) return makeToken(TokenKind::PipeAssign, tokenStart);
        if (match('>')) return makeToken(TokenKind::PipeArrow,  tokenStart);
        return makeToken(TokenKind::Pipe, tokenStart);

    case '^':
        if (match('=')) return makeToken(TokenKind::CaretAssign, tokenStart);
        return makeToken(TokenKind::Caret, tokenStart);

    case '~':
        return makeToken(TokenKind::Tilde, tokenStart);

    default:
        break;
    }

    // Unknown character
    char msg[64];
    snprintf(msg, sizeof(msg), "unexpected character '%c' (0x%02X)",
             (unsigned char)c, (unsigned char)c);
    return makeError(tokenStart, msg);
}

} // namespace vex