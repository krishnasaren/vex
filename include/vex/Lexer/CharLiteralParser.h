#pragma once
// include/vex/Lexer/CharLiteralParser.h


// ============================================================================
// vex/Lexer/CharLiteralParser.h  — Parses single-character literals 'c'
// ============================================================================
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/SourceLocation.h"
#include <cstdint>
#include <string_view>

namespace vex {

struct CharLiteralValue {
    uint32_t codepoint;
    bool     isByte;    // b'x' — must be ASCII
};

class CharLiteralParser {
public:
    explicit CharLiteralParser(DiagnosticEngine& diags) : diags_(diags) {}

    CharLiteralValue parse(std::string_view spelling, SourceLocation loc);

private:
    DiagnosticEngine& diags_;
};

} // namespace vex
