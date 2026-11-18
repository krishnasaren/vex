#include "vex/Sema/NameLookup.h"
namespace vex {

Symbol* NameLookup::lookupUnqualified(std::string_view name, SourceLocation) {
    return ctx_.symbols().lookup(name);
}

Symbol* NameLookup::lookupQualified(const std::vector<std::string_view>& path,
                                     SourceLocation) {
    if (path.empty()) return nullptr;
    return ctx_.symbols().lookup(path.back());
}

Symbol* NameLookup::lookupType(std::string_view name, SourceLocation loc) {
    return lookupUnqualified(name, loc);
}

Symbol* NameLookup::lookupMethod(TypeNode*, std::string_view, SourceLocation) {
    return nullptr;
}

Symbol* NameLookup::lookupModuleMember(Symbol*, std::string_view, SourceLocation) {
    return nullptr;
}

} // namespace vex
