#pragma once
// include/vex/Sema/Symbol.h

#pragma once
// ============================================================================
// vex/Sema/Symbol.h
// A symbol is a resolved declaration in the symbol table.
// ============================================================================

#include "vex/AST/ASTNode.h"
#include "vex/AST/Visibility.h"
#include "vex/Core/SourceLocation.h"
#include <string>
#include <string_view>

namespace vex {

class TypeNode;
class DeclNode;

// ── SymbolKind ────────────────────────────────────────────────────────────────
enum class SymbolKind : uint8_t {
    Variable,
    Parameter,
    Function,
    Struct,
    Class,
    Enum,
    EnumVariant,
    Trait,
    Interface,
    TypeAlias,
    Distinct,
    Constant,
    Module,
    Builtin,
};

std::string_view symbolKindName(SymbolKind k);

// ── Symbol ────────────────────────────────────────────────────────────────────
class Symbol {
public:
    Symbol(SymbolKind kind,
           std::string_view name,
           DeclNode* decl,
           SourceLocation loc)
        : kind_(kind), name_(name), decl_(decl), loc_(loc) {}

    SymbolKind       kind()       const { return kind_; }
    std::string_view name()       const { return name_; }
    DeclNode*        decl()       const { return decl_; }
    SourceLocation   location()   const { return loc_; }
    Visibility       visibility() const { return vis_; }
    TypeNode*        type()       const { return type_; }

    void setVisibility(Visibility v) { vis_  = v; }
    void setType(TypeNode* t)        { type_ = t; }

    bool isType() const {
        return kind_ == SymbolKind::Struct  ||
               kind_ == SymbolKind::Class   ||
               kind_ == SymbolKind::Enum    ||
               kind_ == SymbolKind::Trait   ||
               kind_ == SymbolKind::Interface ||
               kind_ == SymbolKind::TypeAlias ||
               kind_ == SymbolKind::Distinct;
    }
    bool isCallable() const {
        return kind_ == SymbolKind::Function;
    }

private:
    SymbolKind     kind_;
    std::string    name_;
    DeclNode*      decl_  = nullptr;
    TypeNode*      type_  = nullptr;
    SourceLocation loc_;
    Visibility     vis_   = Visibility::Pub;
};

} // namespace vex
