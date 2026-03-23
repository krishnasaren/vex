#pragma once
// include/vex/Lexer/UnicodeCharSets.h


// ============================================================================
// vex/Lexer/UnicodeCharSets.h  — Unicode character class tests for identifiers
// ============================================================================
#include <cstdint>

namespace vex {
namespace unicode {

// Returns true if codepoint is a valid identifier start (XID_Start)
bool isIdentStart(uint32_t cp);

// Returns true if codepoint is a valid identifier continuation (XID_Continue)
bool isIdentContinue(uint32_t cp);

// Returns true if cp is whitespace
bool isWhitespace(uint32_t cp);

// Decode one UTF-8 code point from ptr; advances ptr; returns 0 on error
uint32_t decodeUTF8(const char*& ptr, const char* end);

// How many bytes does a codepoint need in UTF-8
int utf8Len(uint32_t cp);

} // namespace unicode
} // namespace vex