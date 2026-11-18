#include "vex/Sema/OverloadResolver.h"
#include "vex/Core/DiagnosticIDs.h"
namespace vex {

FnDecl* OverloadResolver::resolve(std::string_view name,
                                   const std::vector<TypeNode*>& argTypes,
                                   const std::vector<FnDecl*>& candidates,
                                   SourceLocation callLoc) {
    if (candidates.empty()) {
        ctx_.diags().emitSimple(DiagID::TYPE_NoMatchingOverload, callLoc,
            "no matching function '" + std::string(name) + "'");
        return nullptr;
    }
    // Score each candidate; pick the best one
    FnDecl* best  = nullptr;
    int     bestS = -1;
    int     ties  = 0;
    for (auto* fn : candidates) {
        int s = scoreCandidate(fn, argTypes);
        if (s > bestS) { best = fn; bestS = s; ties = 1; }
        else if (s == bestS && s >= 0) { ++ties; }
    }
    if (ties > 1) {
        ctx_.diags().emitSimple(DiagID::TYPE_AmbiguousOverload, callLoc,
            "ambiguous overload for '" + std::string(name) + "'");
        return nullptr;
    }
    if (!best || bestS < 0) {
        ctx_.diags().emitSimple(DiagID::TYPE_NoMatchingOverload, callLoc,
            "no matching overload for '" + std::string(name) + "'");
        return nullptr;
    }
    return best;
}

int OverloadResolver::scoreCandidate(FnDecl* fn,
                                      const std::vector<TypeNode*>& argTypes) {
    if (fn->params.size() != argTypes.size()) return -1;
    int score = 100;
    for (size_t i = 0; i < argTypes.size(); ++i) {
        if (!argTypes[i]) continue;  // unknown type — skip check
        if (!isConvertible(argTypes[i], fn->params[i]->type)) return -1;
    }
    return score;
}

bool OverloadResolver::isConvertible(TypeNode* /*from*/, TypeNode* /*to*/) {
    return true; // permissive until unification is complete
}

} // namespace vex
