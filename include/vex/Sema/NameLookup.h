#pragma once
// include/vex/Sema/NameLookup.h



// ============================================================================
// vex/Sema/NameLookup.h  — Utilities for qualified and unqualified name lookup
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/Sema/Symbol.h"
#include "vex/Core/SourceLocation.h"
#include <vector>
#include <string_view>

namespace vex {

class NameLookup {
public:
    explicit NameLookup(SemaContext& ctx) : ctx_(ctx) {}

    // Unqualified lookup: searches current scope chain
    Symbol* lookupUnqualified(std::string_view name, SourceLocation loc);

    // Qualified lookup: resolves a::b::c paths
    Symbol* lookupQualified(const std::vector<std::string_view>& path,
                             SourceLocation loc);

    // Type lookup (only returns type symbols)
    Symbol* lookupType(std::string_view name, SourceLocation loc);

    // Method lookup on a specific type
    Symbol* lookupMethod(TypeNode* receiver,
                         std::string_view method,
                         SourceLocation loc);

    // Module member lookup
    Symbol* lookupModuleMember(Symbol* modSym,
                               std::string_view member,
                               SourceLocation loc);

private:
    SemaContext& ctx_;
};

} // namespace vex



