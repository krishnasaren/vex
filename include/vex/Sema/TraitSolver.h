#pragma once
// include/vex/Sema/TraitSolver.h


// ============================================================================
// vex/Sema/TraitSolver.h  — Trait / interface satisfaction checking
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"
#include <vector>

namespace vex {

class TraitSolver {
public:
    explicit TraitSolver(SemaContext& ctx) : ctx_(ctx) {}

    // Check that 'impl' satisfies every requirement of its trait
    bool checkImpl(ImplDecl* impl);

    // Check that a concrete type T satisfies trait Tr (for generics)
    bool satisfies(TypeNode* T, TraitDecl* Tr, SourceLocation loc);

    // Find all impls for a type
    std::vector<ImplDecl*> impsFor(TypeNode* T);

    // Lookup a specific trait method on T
    FnDecl* lookupMethod(TypeNode* T, std::string_view method,
                         const std::vector<TypeNode*>& argTypes);

private:
    SemaContext& ctx_;

    bool checkMethodPresent(ImplDecl* impl, FnDecl* required);
    bool checkSignatureMatch(FnDecl* provided, FnDecl* required);
};

} // namespace vex