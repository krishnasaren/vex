// src/Sema/Sema.cpp



#include "vex/Sema/Sema.h"
#include "vex/Sema/NameResolver.h"
#include "vex/Sema/TypeChecker.h"
#include "vex/Sema/BorrowChecker.h"
#include "vex/Sema/VisibilityChecker.h"
#include "vex/Sema/ExhaustivenessChecker.h"
#include "vex/Sema/ComptimeEvaluator.h"
#include "vex/Sema/DeriveExpander.h"
#include "vex/Sema/DecoratorProcessor.h"
#include "vex/Sema/MoveChecker.h"
#include "vex/Sema/OrphanChecker.h"
#include "vex/Sema/OverflowChecker.h"
#include "vex/Sema/FormatStringChecker.h"
#include "vex/Sema/ImplResolver.h"

namespace vex {

bool Sema::run(ModuleDecl* mod) {
    if (!mod) return false;

    // Phase 1: expand derives and decorators first
    {
        DeriveExpander derive(ctx_);
        if (!derive.run(mod)) return false;
    }
    {
        DecoratorProcessor deco(ctx_);
        if (!deco.run(mod)) return false;
    }

    // Phase 2: name resolution
    if (!resolveNames(mod)) return false;

    // Phase 3: type checking + inference
    if (!checkTypes(mod)) return false;

    // Phase 4: exhaustiveness
    if (!checkExhaustiveness(mod)) return false;

    // Phase 5: borrow / ownership / move checking
    if (!checkBorrows(mod)) return false;

    // Phase 6: visibility
    if (!checkVisibility(mod)) return false;

    // Phase 7: comptime evaluation
    if (!evaluateComptime(mod)) return false;

    // Phase 8: overflow detection
    {
        OverflowChecker oc(ctx_);
        if (!oc.run(mod)) return false;
    }

    // Phase 9: format string validation
    {
        FormatStringChecker fsc(ctx_);
        if (!fsc.run(mod)) return false;
    }

    // Phase 10: orphan rule
    {
        OrphanChecker oc(ctx_);
        if (!oc.run(mod)) return false;
    }

    return !ctx_.hasErrors();
}

bool Sema::resolveNames(ModuleDecl* mod) {
    NameResolver nr(ctx_);
    return nr.run(mod);
}

bool Sema::checkTypes(ModuleDecl* mod) {
    TypeChecker tc(ctx_);
    return tc.run(mod);
}

bool Sema::checkBorrows(ModuleDecl* mod) {
    BorrowChecker bc(ctx_);
    MoveChecker   mc(ctx_);
    return bc.run(mod) && mc.run(mod);
}

bool Sema::checkVisibility(ModuleDecl* mod) {
    VisibilityChecker vc(ctx_);
    return vc.run(mod);
}

bool Sema::checkExhaustiveness(ModuleDecl* mod) {
    ExhaustivenessChecker ec(ctx_);
    return ec.run(mod);
}

bool Sema::evaluateComptime(ModuleDecl* mod) {
    ComptimeEvaluator ce(ctx_);
    return ce.run(mod);
}

} // namespace vex

