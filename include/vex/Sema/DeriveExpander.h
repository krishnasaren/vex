#pragma once
// include/vex/Sema/DeriveExpander.h


// ============================================================================
// vex/Sema/DeriveExpander.h  — Expands @derive(Trait, ...) decorators
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"
#include <string_view>

namespace vex {

class DeriveExpander {
public:
    explicit DeriveExpander(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    // Expand a single derive on a declaration
    bool expand(Decl* decl, std::string_view traitName);

private:
    SemaContext& ctx_;

    bool deriveDebug(StructDecl* s);
    bool deriveDebug(EnumDecl* e);
    bool deriveClone(StructDecl* s);
    bool deriveClone(EnumDecl* e);
    bool deriveEq(StructDecl* s);
    bool deriveEq(EnumDecl* e);
    bool deriveHash(StructDecl* s);
    bool deriveDefault(StructDecl* s);
    bool deriveIter(StructDecl* s);
    bool deriveDisplay(StructDecl* s);
    bool deriveDisplay(EnumDecl* e);

    // Inject an impl block back into the AST
    void injectImpl(ImplDecl* impl, Decl* forDecl);
};

} // namespace vex
