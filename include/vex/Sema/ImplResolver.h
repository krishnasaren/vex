#pragma once
// include/vex/Sema/ImplResolver.h


// ============================================================================
// vex/Sema/ImplResolver.h  — Resolves which impl block provides a method call
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Expr.h"

namespace vex {

class ImplResolver {
public:
    explicit ImplResolver(SemaContext& ctx) : ctx_(ctx) {}

    // Register an impl block
    void registerImpl(ImplDecl* impl);

    // Resolve a method call on receiver type T
    FnDecl* resolveMethod(TypeNode* T, std::string_view method,
                          const std::vector<TypeNode*>& argTypes,
                          SourceLocation loc);

    // Resolve a trait method via trait bound
    FnDecl* resolveTraitMethod(TypeNode* T, TraitDecl* trait,
                               std::string_view method,
                               const std::vector<TypeNode*>& argTypes,
                               SourceLocation loc);

private:
    SemaContext& ctx_;
    // type-name → list of impl blocks
    std::unordered_map<std::string, std::vector<ImplDecl*>> implMap_;

    bool methodMatches(FnDecl* fn, const std::vector<TypeNode*>& argTypes);
};

} // namespace vex
