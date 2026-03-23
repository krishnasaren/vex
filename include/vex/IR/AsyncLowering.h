#pragma once
// include/vex/IR/AsyncLowering.h


// ============================================================================
// vex/IR/AsyncLowering.h  — Transforms async/await into coroutine state machines
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class AsyncLowering : public IRPass {
public:
    explicit AsyncLowering(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "AsyncLowering"; }

    bool runOnFunction(IRFunction& fn) override;

private:
    LoweringContext& ctx_;

    bool isAsyncFunction(const IRFunction& fn) const;
    void buildStateStruct(IRFunction& fn);
    void lowerAwaitInstr(IRInstr& awaitInstr, IRFunction& fn);
    void splitAtSuspendPoints(IRFunction& fn);
};

} // namespace vex
