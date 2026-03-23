#pragma once
// include/vex/Optimizer/DeadCodeElim.h


// vex/Optimizer/DeadCodeElim.h  — VEX IR level DCE pass
#include "vex/IR/IRPass.h"
namespace vex {
class DeadCodeElim : public IRPass {
public:
    IRPassKind kind() const override { return IRPassKind::FunctionPass; }
    std::string_view name() const override { return "DeadCodeElim"; }
    bool runOnFunction(IRFunction& fn) override;
};
} // namespace vex
