#pragma once
// include/vex/IR/ErrorPropLowering.h



// ============================================================================
// vex/IR/ErrorPropLowering.h  — Lowers ? try-propagation into branches
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class ErrorPropLowering : public IRPass {
public:
    explicit ErrorPropLowering(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "ErrorPropLowering"; }

    bool runOnFunction(IRFunction& fn) override;

private:
    LoweringContext& ctx_;

    void lowerTryInstr(IRInstr& tryInstr,
                        IRBlock* okBlock,
                        IRBlock* errBlock);
};

} // namespace vex
