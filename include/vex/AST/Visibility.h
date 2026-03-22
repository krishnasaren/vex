#pragma once
// ============================================================================
// vex/AST/Visibility.h
// The four visibility modifiers from Chapter 4 of vex_final_rules.vex.
// ============================================================================

#include <cstdint>
#include <string_view>

namespace vex {

// RULE VIS2: The four visibility modifiers.
// Plus an Unspecified value for "use the default for this context."

enum class Visibility : uint8_t {
    Unspecified,  // use default for context
    Pub,          // any code in any module
    Priv,         // only this file
    Internal,     // all files in same project directory
    Prot,         // this class AND subclasses only (classes only)
};

std::string_view visibilityName(Visibility vis);

// Returns the default visibility for a given declaration context.
// Rules from Chapter 4 RULE VIS1.
enum class DeclContext : uint8_t {
    ModuleLevel,    // top-level in a .vex file
    StructMember,   // field or method inside a struct
    ClassMember,    // field or method inside a class
    EnumVariant,    // variant inside an enum
    TraitMethod,    // method inside a trait
};

Visibility defaultVisibility(DeclContext ctx);

} // namespace vex