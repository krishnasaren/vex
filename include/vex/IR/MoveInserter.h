#pragma once
// include/vex/IR/MoveInserter.h

// ============================================================================
// vex/IR/MoveInserter.h  — Makes all ownership moves explicit in the IR
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class MoveInserter : public IRPass {
public:
    explicit MoveInserter(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "MoveInserter"; }

    bool runOnFunction(IRFunction& fn) override;

private:
    LoweringContext& ctx_;

    bool isMovable(IRType* ty) const;
    void insertMoveForArg(IRInstr& call, unsigned argIdx);
};

} // namespace vex
