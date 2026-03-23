#pragma once
// include/vex/Lexer/Preprocessor.h

// ============================================================================
// vex/Lexer/Preprocessor.h  — Handles #if/@comptime conditional compilation
// and module-level attributes before parsing
// ============================================================================
#include "vex/Lexer/TokenStream.h"
#include "vex/Core/DiagnosticEngine.h"
#include <string>
#include <unordered_map>

namespace vex {

class Preprocessor {
public:
    explicit Preprocessor(DiagnosticEngine& diags) : diags_(diags) {}

    // Process token stream; filters conditional compilation tokens
    TokenStream process(TokenStream& input);

    // Define a feature flag
    void define(std::string_view flag, bool value = true);
    bool isDefined(std::string_view flag) const;

private:
    DiagnosticEngine& diags_;
    std::unordered_map<std::string,bool> flags_;

    bool evaluateCondition(TokenStream& ts);
};

} // namespace vex
