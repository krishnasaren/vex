#pragma once
// include/vex/Sema/Sema.h



// ============================================================================
// vex/Sema/Sema.h
// Main semantic analysis driver. Runs all sema passes over the AST.
// ============================================================================

#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"
#include <vector>

namespace vex {

class Sema {
public:
    explicit Sema(VexContext& ctx, ASTContext& astCtx)
        : ctx_(ctx, astCtx) {}

    // Run all semantic passes on a module.
    // Returns false if any errors occurred.
    bool run(ModuleDecl* mod);

    // Individual passes (exposed for testing)
    bool resolveNames(ModuleDecl* mod);
    bool checkTypes(ModuleDecl* mod);
    bool checkBorrows(ModuleDecl* mod);
    bool checkVisibility(ModuleDecl* mod);
    bool checkExhaustiveness(ModuleDecl* mod);
    bool evaluateComptime(ModuleDecl* mod);

    SemaContext& context() { return ctx_; }

private:
    SemaContext ctx_;
};

} // namespace vex
