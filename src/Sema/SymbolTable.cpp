// src/Sema/SymbolTable.cpp
#include "vex/Sema/SymbolTable.h"

namespace vex {

// ── Scope ─────────────────────────────────────────────────────────────────────

Symbol* Scope::define(std::unique_ptr<Symbol> sym) {
    auto name = std::string(sym->name());
    auto [it, inserted] = symbols_.emplace(name, std::move(sym));
    return inserted ? it->second.get() : nullptr;
}

Symbol* Scope::lookupLocal(std::string_view name) const {
    auto it = symbols_.find(std::string(name));
    return (it != symbols_.end()) ? it->second.get() : nullptr;
}

Symbol* Scope::lookup(std::string_view name) const {
    if (auto* sym = lookupLocal(name)) return sym;
    if (parent_) return parent_->lookup(name);
    return nullptr;
}

// ── SymbolTable ───────────────────────────────────────────────────────────────

SymbolTable::SymbolTable() {
    auto moduleScope = std::make_unique<Scope>(Scope::Kind::Module, nullptr);
    module_  = moduleScope.get();
    current_ = module_;
    scopes_.push_back(std::move(moduleScope));
    depth_ = 1;
}

void SymbolTable::pushScope(Scope::Kind kind) {
    auto scope = std::make_unique<Scope>(kind, current_);
    current_ = scope.get();
    scopes_.push_back(std::move(scope));
    ++depth_;
}

void SymbolTable::popScope() {
    if (scopes_.size() <= 1) return;
    scopes_.pop_back();
    current_ = scopes_.back().get();
    --depth_;
}

Symbol* SymbolTable::define(SymbolKind kind, std::string_view name,
                             DeclNode* decl, SourceLocation loc) {
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
