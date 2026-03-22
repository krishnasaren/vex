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
        case DeclContext::ModuleLevel:  return Visibility::Pub;
        case DeclContext::StructMember: return Visibility::Pub;
        case DeclContext::ClassMember:  return Visibility::Priv;
        case DeclContext::EnumVariant:  return Visibility::Pub;
        case DeclContext::TraitMethod:  return Visibility::Pub;
    }
    return Visibility::Pub;
}

} // namespace vex