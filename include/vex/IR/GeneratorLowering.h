#pragma once
// include/vex/IR/GeneratorLowering.h


// ============================================================================
// vex/IR/GeneratorLowering.h  — Transforms generator functions into state machines
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class GeneratorLowering : public IRPass {
public:
    explicit GeneratorLowering(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "GeneratorLowering"; }

    bool runOnFunction(IRFunction& fn) override;

private:
    LoweringContext& ctx_;

    bool isGeneratorFunction(const IRFunction& fn) const;
    void buildYieldStateStruct(IRFunction& fn);
    void lowerYieldInstr(IRInstr& yieldInstr, IRFunction& fn);
};

} // namespace vex
