#include "vex/IR/IRPassManager.h"
#include "vex/IR/IRPass.h"
#include "vex/IR/VexIR.h"

namespace vex {

bool IRPassManager::run(IRModule& mod) {
    bool changed = false;
    for (auto& p : passes_) {
        if (p->kind() == IRPassKind::ModulePass) {
            changed |= p->runOnModule(mod);
        } else {
            for (auto& fn : mod.functions())
                changed |= p->runOnFunction(*fn);
        }
    }
    return changed;
}

} // namespace vex
