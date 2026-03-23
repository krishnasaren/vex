#pragma once
// include/vex/Sema/FormatStringChecker.h


// ============================================================================
// vex/Sema/FormatStringChecker.h  — Validates format string arguments
// (for fmt::format, print, println, etc.)
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Expr.h"
#include <vector>

namespace vex {

class FormatStringChecker {
public:
    explicit FormatStringChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    // Check a specific format call (fmt, print, println, etc.)
    bool checkFormatCall(CallExpr* call,
                         std::string_view formatStr,
                         const std::vector<ExprNode*>& args);

private:
    SemaContext& ctx_;

    struct Placeholder {
        bool hasIndex;
        int  index;
        std::string spec;   // format specifier, e.g. ":d", ":.2f"
    };

    std::vector<Placeholder> parsePlaceholders(std::string_view fmt,
                                                SourceLocation loc,
                                                bool& ok);
    bool isFormattable(TypeNode* t) const;
};

} // namespace vex
