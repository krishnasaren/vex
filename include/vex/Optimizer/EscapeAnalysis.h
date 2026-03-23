#pragma once
// include/vex/Optimizer/EscapeAnalysis.h


// ============================================================================
// vex/Optimizer/EscapeAnalysis.h  — Determines which allocations escape heap
// ============================================================================
#include "vex/IR/IRPass.h"
#include <unordered_set>

namespace vex {

class EscapeAnalysis : public IRPass {
public:
    IRPassKind kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "EscapeAnalysis"; }
    bool modifiesIR() const override { return false; }

    bool runOnFunction(IRFunction& fn) override;

    bool escapes(const IRValue* alloca) const;

private:
    std::unordered_set<const IRValue*> escaped_;

    bool valueEscapes(const IRValue* v, int depth) const;
};
} // namespace vex
