#pragma once
// include/vex/Sema/OrphanChecker.h



// ============================================================================
// vex/Sema/OrphanChecker.h  — Orphan rule: impls must be in the same module
// as either the trait or the type.
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"

namespace vex {

class OrphanChecker {
public:
    explicit OrphanChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);
    bool checkImpl(ImplDecl* impl, ModuleDecl* currentMod);

private:
    SemaContext& ctx_;
    bool isLocal(TypeNode* t,  ModuleDecl* mod);
    bool isLocal(TraitDecl* tr, ModuleDecl* mod);
};

} // namespace vex
