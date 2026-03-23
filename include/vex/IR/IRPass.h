#pragma once
// include/vex/IR/IRPass.h

// ============================================================================
// vex/IR/IRPass.h  — Base class for IR transformation and analysis passes
// ============================================================================
#include <string_view>

namespace vex {

class IRModule;
class IRFunction;

enum class IRPassKind : uint8_t {
    ModulePass,    // operates on the whole module
    FunctionPass,  // operates on each function
};

class IRPass {
public:
    virtual ~IRPass() = default;

    virtual IRPassKind  kind() const = 0;
    virtual std::string_view name() const = 0;

    // Run on module (for module passes)
    virtual bool runOnModule(IRModule& mod) { return false; }

    // Run on function (for function passes)
    virtual bool runOnFunction(IRFunction& fn) { return false; }

    // True if this pass modifies IR
    virtual bool modifiesIR() const { return true; }
};

} // namespace vex
