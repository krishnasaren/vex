#pragma once
// include/vex/IR/MatchLowering.h


// ============================================================================
// vex/IR/MatchLowering.h  — Lowers high-level Match IR to switch/if chains
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class MatchLowering : public IRPass {
public:
    explicit MatchLowering(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "MatchLowering"; }

    bool runOnFunction(IRFunction& fn) override;

private:
    LoweringContext& ctx_;

    void lowerEnumMatch(IRInstr& matchInstr);
    void lowerIntMatch(IRInstr& matchInstr);
    void lowerStructMatch(IRInstr& matchInstr);
    void emitPatternTest(IRValue* value, IRBlock* okBlock, IRBlock* failBlock);
};

} // namespace vex
