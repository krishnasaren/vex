// ============================================================================
// vex/AST/Visibility.cpp
// ============================================================================

#include "vex/AST/Visibility.h"

namespace vex {

std::string_view visibilityName(Visibility vis) {
    switch (vis) {
        case Visibility::Unspecified: return "";
        case Visibility::Pub:         return "pub";
        case Visibility::Priv:        return "priv";
        case Visibility::Internal:    return "internal";
        case Visibility::Prot:        return "prot";
    }
    return "";
}

Visibility defaultVisibility(DeclContext ctx) {
    switch (ctx) {
        // RULE VIS1: Module-level fn/struct/class/enum/trait/const → pub
        //            Module-level var (global) → priv
        case DeclContext::ModuleLevel:
            return Visibility::Pub;

        // RULE VIS1: struct field → pub
        case DeclContext::StructMember:
            return Visibility::Pub;

        // RULE VIS1: class field → priv, class method → pub
        // Note: caller must distinguish field vs method.
        // This returns priv as the more restrictive default for class context.
        case DeclContext::ClassMember:
            return Visibility::Priv;

        // RULE VIS1: enum variant → pub always
        case DeclContext::EnumVariant:
            return Visibility::Pub;

        // RULE VIS1: trait method → pub always
        case DeclContext::TraitMethod:
            return Visibility::Pub;
    }
    return Visibility::Pub;
}

} // namespace vex