#pragma once
// include/vex/Lexer/Keywords.h



// ============================================================================
// vex/Lexer/Keywords.h  — Keyword → TokenKind lookup
// ============================================================================
#include "vex/Lexer/TokenKind.h"
#include <string_view>
#include <optional>

namespace vex {

// Returns the TokenKind for the given identifier string if it is a keyword.
std::optional<TokenKind> lookupKeyword(std::string_view ident);

// Returns true if s is any reserved keyword
bool isKeyword(std::string_view s);

} // namespace vex


