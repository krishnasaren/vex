#pragma once
// include/vex/Sema/SymbolTable.h





// ============================================================================
// vex/Sema/SymbolTable.h
// Scoped symbol table: maps names to Symbol* at each scope level.
// ============================================================================

#include "vex/Sema/Symbol.h"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace vex {

class Scope {
public:
    enum class Kind : uint8_t {
        Module,
        Function,
        Block,
        Struct,
        Class,
        Enum,
        Trait,
        Interface,
        Loop,
        Lambda,
    };

    explicit Scope(Kind kind, Scope* parent = nullptr)
        : kind_(kind), parent_(parent) {}

    Kind    kind()   const { return kind_; }
    Scope*  parent() const { return parent_; }

    // Define a symbol in this scope. Returns nullptr if already defined.
    Symbol* define(std::unique_ptr<Symbol> sym);

    // Look up in this scope only (no parent traversal).
    Symbol* lookupLocal(std::string_view name) const;

    // Look up in this scope and all ancestors.
    Symbol* lookup(std::string_view name) const;

    const std::unordered_map<std::string, std::unique_ptr<Symbol>>& symbols() const {
        return symbols_;
    }

private:
    Kind   kind_;
    Scope* parent_;
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols_;
};

// ── SymbolTable ───────────────────────────────────────────────────────────────
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable() = default;

    // Scope management
    void   pushScope(Scope::Kind kind);
    void   popScope();
    Scope* currentScope() const { return current_; }

    // Define symbol in current scope
    Symbol* define(SymbolKind kind, std::string_view name,
                   DeclNode* decl, SourceLocation loc);

    // Lookup from current scope upward
    Symbol* lookup(std::string_view name) const;

    // Lookup in module (top-level) scope
    Symbol* lookupModule(std::string_view name) const;

    // Total scope depth
    size_t depth() const { return depth_; }

private:
    std::vector<std::unique_ptr<Scope>> scopes_;
    Scope* current_ = nullptr;
    Scope* module_  = nullptr;
    size_t depth_   = 0;
};

} // namespace vex
