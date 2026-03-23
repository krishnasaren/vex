// src/Sema/Symbol.cpp



#include "vex/Sema/Symbol.h"
namespace vex {

std::string_view symbolKindName(SymbolKind k) {
    switch (k) {
    case SymbolKind::Variable:    return "variable";
    case SymbolKind::Parameter:   return "parameter";
    case SymbolKind::Function:    return "function";
    case SymbolKind::Struct:      return "struct";
    case SymbolKind::Class:       return "class";
    case SymbolKind::Enum:        return "enum";
    case SymbolKind::EnumVariant: return "enum variant";
    case SymbolKind::Trait:       return "trait";
    case SymbolKind::Interface:   return "interface";
    case SymbolKind::TypeAlias:   return "type alias";
    case SymbolKind::Distinct:    return "distinct type";
    case SymbolKind::Constant:    return "constant";
    case SymbolKind::Module:      return "module";
    case SymbolKind::Builtin:     return "builtin";
    }
    return "unknown";
}

} // namespace vex

