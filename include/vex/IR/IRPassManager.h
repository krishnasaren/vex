#pragma once
// include/vex/IR/IRPassManager.h



// ============================================================================
// vex/IR/IRPassManager.h  — Manages and runs a sequence of IR passes
// ============================================================================
#include "vex/IR/IRPass.h"
#include <vector>
#include <memory>

namespace vex {

class IRModule;

class IRPassManager {
public:
    void addPass(std::unique_ptr<IRPass> p) { passes_.push_back(std::move(p)); }

    // Run all passes on module; returns true if any pass modified IR
    bool run(IRModule& mod);

    void clear() { passes_.clear(); }
    size_t numPasses() const { return passes_.size(); }

private:
    std::vector<std::unique_ptr<IRPass>> passes_;
};

} // namespace vex
