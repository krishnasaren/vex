#include "vex/Module/ModuleLoader.h"
#include "vex/Module/ModuleCache.h"
namespace vex {

ModuleDecl* ModuleLoader::load(std::string_view importPath,
                                std::string_view /*fromFile*/,
                                SourceLocation /*loc*/) {
    if (auto* cached = cache_.lookup(importPath)) return cached;
    return nullptr;
}

ModuleDecl* ModuleLoader::loadFile(std::string_view /*filePath*/) {
    return nullptr;
}

ModuleDecl* ModuleLoader::parseVexSource(const std::string&) {
    return nullptr;
}

ModuleDecl* ModuleLoader::loadVxl(const std::string&) {
    return nullptr;
}

} // namespace vex
