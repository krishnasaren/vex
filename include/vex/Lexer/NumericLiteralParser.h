#pragma once
// include/vex/Lexer/NumericLiteralParser.h


// ============================================================================
// vex/Lexer/NumericLiteralParser.h  — Parses int/float literal spellings
// ============================================================================
#include "vex/Core/DiagnosticEngine.h"
#include "vex/Core/SourceLocation.h"
#include <cstdint>
#include <string_view>

namespace vex {

struct NumericLiteralValue {
    enum class Kind { Integer, Float };
    Kind    kind = Kind::Integer;
    int64_t  intVal  = 0;
    double   floatVal= 0.0;
    // Explicit suffix: i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 (empty = infer)
    std::string suffix;
    bool     isNeg   = false;
};

class NumericLiteralParser {
public:
    explicit NumericLiteralParser(DiagnosticEngine& diags) : diags_(diags) {}

    // Parse a numeric literal spelled as 'spelling' at 'loc'
    NumericLiteralValue parse(std::string_view spelling, SourceLocation loc);

private:
    DiagnosticEngine& diags_;

    NumericLiteralValue parseDecimal(std::string_view s, SourceLocation loc);
    NumericLiteralValue parseHex(std::string_view s, SourceLocation loc);
    NumericLiteralValue parseOct(std::string_view s, SourceLocation loc);
    NumericLiteralValue parseBin(std::string_view s, SourceLocation loc);
    NumericLiteralValue parseFloat(std::string_view s, SourceLocation loc);

    std::string extractSuffix(std::string_view& s);
};

} // namespace vex

