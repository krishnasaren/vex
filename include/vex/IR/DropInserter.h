#pragma once
// include/vex/IR/DropInserter.h



// ============================================================================
// vex/IR/DropInserter.h  — Inserts Drop calls for owned values going out of scope
// ============================================================================
#include "vex/IR/IRPass.h"
#include "vex/IR/LoweringContext.h"

namespace vex {

class DropInserter : public IRPass {
public:
    explicit DropInserter(LoweringContext& ctx) : ctx_(ctx) {}

    IRPassKind   kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "DropInserter"; }

    bool runOnFunction(IRFunction& fn) override;

private:
    LoweringContext& ctx_;

    bool needsDrop(IRType* ty) const;
    void insertDropBefore(IRInstr& pos, IRValue* val);
};

} // namespace vex
