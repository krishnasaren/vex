#include "vex/Module/ModuleCache.h"
#include "vex/AST/Decl.h"
namespace vex {

ModuleDecl* ModuleCache::lookup(std::string_view path) const {
    auto it = cache_.find(std::string(path));
    return (it != cache_.end()) ? it->second : nullptr;
}

void ModuleCache::insert(std::string_view path, ModuleDecl* mod) {
    cache_[std::string(path)] = mod;
}

void ModuleCache::clear() {
    cache_.clear();
}

} // namespace vex
