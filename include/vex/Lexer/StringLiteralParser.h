#pragma once
// include/vex/Lexer/StringLiteralParser.h
EOF

// ============================================================================
// vex/Lexer/StringLiteralParser.h  — Unescapes string / byte literals
// ============================================================================
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/SourceLocation.h"
#include <string>
#include <string_view>

namespace vex {

struct StringLiteralValue {
    std::string value;   // Unescaped UTF-8 content
    bool        isByte;  // b"..." byte string
    bool        isRaw;   // r#"..."# raw string
    bool        isFmt;   // f"..." format string (no unescaping needed yet)
};

class StringLiteralParser {
public:
    explicit StringLiteralParser(DiagnosticEngine& diags) : diags_(diags) {}

    StringLiteralValue parse(std::string_view spelling, SourceLocation loc);

private:
    DiagnosticEngine& diags_;

    bool processEscape(std::string_view src, size_t& i,
                       std::string& out, SourceLocation loc);
};

} // namespace vex
