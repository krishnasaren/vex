#pragma once
// include/vex/Sema/OverloadResolver.h



// ============================================================================
// vex/Sema/OverloadResolver.h
// Resolves overloaded function calls to the best matching candidate.
// ============================================================================

#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Expr.h"
#include <vector>

namespace vex {

struct OverloadCandidate {
    FnDecl*             fn;
    int                 score;   // higher = better match
    bool                viable;
};

class OverloadResolver {
public:
    explicit OverloadResolver(SemaContext& ctx) : ctx_(ctx) {}

    // Resolve a call expression to the best candidate.
    // Returns nullptr on failure (diagnostic emitted).
    FnDecl* resolve(std::string_view name,
                    const std::vector<TypeNode*>& argTypes,
                    const std::vector<FnDecl*>& candidates,
                    SourceLocation callLoc);

private:
    SemaContext& ctx_;

    int scoreCandidate(FnDecl* fn, const std::vector<TypeNode*>& argTypes);
    bool isConvertible(TypeNode* from, TypeNode* to);
};

} // namespace vex
