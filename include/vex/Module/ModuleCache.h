#pragma once
// include/vex/Module/ModuleCache.h



// vex/Module/ModuleCache.h  — Caches compiled module ASTs to avoid re-parsing
#include "vex/AST/Decl.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace vex {

class ModuleCache {
public:
    void   insert(std::string_view path, ModuleDecl* mod);
    ModuleDecl* lookup(std::string_view path) const;
    bool   contains(std::string_view path) const;
    void   clear();
    size_t size() const { return cache_.size(); }

private:
    std::unordered_map<std::string, ModuleDecl*> cache_;
};
} // namespace vex
