#pragma once
// include/vex/IR/ClosureLowering.h

// ============================================================================
// vex/IR/ClosureLowering.h  — Converts closures into structs + call methods
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class ClosureLowering : public IRPass {
public:
    explicit ClosureLowering(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::ModulePass; }
    std::string_view name() const override { return "ClosureLowering"; }

    bool runOnModule(IRModule& mod) override;

private:
    LoweringContext& ctx_;

    IRType*     buildCaptureStruct(IRFunction& closureFn,
                                    const std::vector<IRValue*>& captures);
    IRFunction* buildThunk(IRFunction& closureFn, IRType* captureStructTy);
};

} // namespace vex
