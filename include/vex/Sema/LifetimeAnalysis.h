#pragma once
// include/vex/Sema/LifetimeAnalysis.h



// ============================================================================
// vex/Sema/LifetimeAnalysis.h  — Lifetime region computation for borrows
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/Sema/ControlFlowGraph.h"
#include "vex/AST/Decl.h"
#include <unordered_map>
#include <string>

namespace vex {

struct Lifetime {
    uint32_t beginCFGNode;
    uint32_t endCFGNode;
    bool     liveAtReturn = false;
};

class LifetimeAnalysis {
public:
    explicit LifetimeAnalysis(SemaContext& ctx) : ctx_(ctx) {}

    // Compute lifetimes for all variables in a function
    bool analyze(FnDecl* fn);

    const Lifetime* lifetime(std::string_view varName) const;

    // Check that a borrow does not outlive its referent
    bool checkBorrowValid(std::string_view borrow,
                          std::string_view referent,
                          SourceLocation loc);

private:
    SemaContext& ctx_;
    std::unordered_map<std::string, Lifetime> lifetimes_;
};

} // namespace vex
