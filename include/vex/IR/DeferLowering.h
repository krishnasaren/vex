#pragma once
// include/vex/IR/DeferLowering.h


// ============================================================================
// vex/IR/DeferLowering.h  — Lowers defer statements into cleanup blocks
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class DeferLowering : public IRPass {
public:
    explicit DeferLowering(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "DeferLowering"; }

    bool runOnFunction(IRFunction& fn) override;

private:
    LoweringContext& ctx_;

    void collectDeferPoints(IRFunction& fn,
                             std::vector<IRInstr*>& defers);
    void insertCleanupBeforeRet(IRFunction& fn,
                                 const std::vector<IRInstr*>& defers,
                                 IRInstr* retInstr);
};

} // namespace vex
