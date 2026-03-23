#pragma once
// include/vex/Lexer/LiteralParser.h


// ============================================================================
// vex/Lexer/LiteralParser.h  — Base class / utilities for literal parsing
// ============================================================================
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/SourceLocation.h"
#include <string>
#include <string_view>

namespace vex {

// Shared utilities for numeric, string, and char literal parsers
struct LiteralParser {
    static bool isHexDigit(char c);
    static bool isOctDigit(char c);
    static bool isBinDigit(char c);
    static int  hexVal(char c);

    // Decode a Unicode escape \u{HHHH}; returns false and emits diag on error
    static bool decodeUnicodeEscape(std::string_view text,
                                     size_t& pos,
                                     uint32_t& codepoint,
                                     DiagnosticEngine& diags,
                                     SourceLocation loc);

    // Encode a Unicode codepoint as UTF-8
    static std::string encodeUTF8(uint32_t codepoint);
};

} // namespace vex
