#pragma once
// include/vex/Module/SelectiveImport.h



// ============================================================================
// vex/Module/SelectiveImport.h  — Handles "import mod::{A, B}" syntax
// ============================================================================
#include "vex/AST/Decl.h"
#include "vex/Sema/SemaContext.h"
#include <vector>
#include <string>

namespace vex {

class SelectiveImport {
public:
    explicit SelectiveImport(SemaContext& ctx) : ctx_(ctx) {}

    // Process a selective import declaration
    bool process(ImportDecl* decl, ModuleDecl* importedMod);

    // Wildcard import (import mod::*)
    bool processWildcard(ImportDecl* decl, ModuleDecl* importedMod);

    // Aliased import (import mod::Foo as Bar)
    bool processAliased(ImportDecl* decl, ModuleDecl* importedMod);

private:
    SemaContext& ctx_;

    Symbol* resolveExport(ModuleDecl* mod, std::string_view name);
    void    importSymbol(Symbol* sym, std::string_view alias);
};
} // namespace vex
