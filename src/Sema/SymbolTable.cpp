// src/Sema/SymbolTable.cpp




#include "vex/Sema/SymbolTable.h"
#include <cassert>

namespace vex {

Symbol* Scope::define(std::unique_ptr<Symbol> sym) {
    auto it = symbols_.find(std::string(sym->name()));
    if (it != symbols_.end()) return nullptr; // already defined
    Symbol* raw = sym.get();
    symbols_.emplace(std::string(sym->name()), std::move(sym));
    return raw;
}

Symbol* Scope::lookupLocal(std::string_view name) const {
    auto it = symbols_.find(std::string(name));
    return it != symbols_.end() ? it->second.get() : nullptr;
}

Symbol* Scope::lookup(std::string_view name) const {
    if (auto* s = lookupLocal(name)) return s;
    return parent_ ? parent_->lookup(name) : nullptr;
}

// ── SymbolTable ──────────────────────────────────────────────────────────────

SymbolTable::SymbolTable() {
    // Create global module scope
    scopes_.push_back(std::make_unique<Scope>(Scope::Kind::Module, nullptr));
    current_ = module_ = scopes_.back().get();
    depth_ = 0;
}

void SymbolTable::pushScope(Scope::Kind kind) {
    scopes_.push_back(std::make_unique<Scope>(kind, current_));
    current_ = scopes_.back().get();
    ++depth_;
}

void SymbolTable::popScope() {
    assert(current_ != module_ && "Cannot pop module scope");
    current_ = current_->parent();
    --depth_;
}

Symbol* SymbolTable::define(SymbolKind kind,
                             std::string_view name,
                             DeclNode* decl,
                             SourceLocation loc) {
    auto sym = std::make_unique<Symbol>(kind, name, decl, loc);
    return current_->define(std::move(sym));
}

Symbol* SymbolTable::lookup(std::string_view name) const {
    return current_ ? current_->lookup(name) : nullptr;
}

Symbol* SymbolTable::lookupModule(std::string_view name) const {
    return module_ ? module_->lookupLocal(name) : nullptr;
}

} // namespace vex
