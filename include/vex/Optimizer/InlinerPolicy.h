#pragma once
// include/vex/Optimizer/InlinerPolicy.h


// ============================================================================
// vex/Optimizer/InlinerPolicy.h  — Decides which calls to inline
// ============================================================================
#include "vex/IR/IRModule.h"

namespace vex {

struct InlinerPolicy {
    unsigned maxInlineSize     = 100;  // IR instructions
    unsigned maxInlineDepth    = 6;
    bool     inlineAlwaysAttr  = true;
    bool     inlineColdFns     = false;
    bool     inlineSingleUse   = true;

    // Returns a cost for inlining fn at call site; <0 = do inline
    int cost(const IRFunction& fn, const IRInstr& call) const;

    static InlinerPolicy defaultDebug();
    static InlinerPolicy defaultRelease();
    static InlinerPolicy aggressive();
};
} // namespace vex