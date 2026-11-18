#include "vex/Sema/FormatStringChecker.h"
namespace vex {

bool FormatStringChecker::run(ModuleDecl* mod) {
    if (!mod) return true;
    
    return !ctx_.diags().hasErrors();
}

bool FormatStringChecker::checkFormatCall(CallExpr*, std::string_view,
                                           const std::vector<ExprNode*>&) {
    return true;
}

std::vector<FormatStringChecker::Placeholder>
FormatStringChecker::parsePlaceholders(std::string_view fmt, SourceLocation, bool& ok) {
    std::vector<Placeholder> result;
    ok = true;
    for (size_t i = 0; i < fmt.size(); ) {
        if (fmt[i] == '{') {
            size_t end = fmt.find('}', i);
            if (end == std::string_view::npos) { ok = false; break; }
            Placeholder p;
            p.hasIndex = false; p.index = -1;
            std::string_view inner = fmt.substr(i+1, end-i-1);
            if (!inner.empty() && inner[0] >= '0' && inner[0] <= '9') {
                p.hasIndex = true; p.index = inner[0] - '0';
            }
            result.push_back(p);
            i = end + 1;
        } else { ++i; }
    }
    return result;
}

bool FormatStringChecker::isFormattable(TypeNode*) const { return true; }

} // namespace vex
