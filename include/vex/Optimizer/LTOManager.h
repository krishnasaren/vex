#pragma once
// include/vex/Optimizer/LTOManager.h


// ============================================================================
// vex/Optimizer/LTOManager.h  — Link-Time Optimization manager
// ============================================================================
#include "vex/Driver/Options.h"
#include "vex/Core/DiagnosticEngine.h"
#include "llvm/IR/Module.h"
#include <vector>
#include <memory>

namespace vex {

class LTOManager {
public:
    LTOManager(DiagnosticEngine& diags, const DriverOptions& opts)
        : diags_(diags), opts_(opts) {}

    // Add a module to LTO set
    void addModule(std::unique_ptr<llvm::Module> mod);

    // Run LTO optimization across all modules; returns merged module
    std::unique_ptr<llvm::Module> run();

    // Write ThinLTO index
    bool writeThinLTOIndex(std::string_view path);

private:
    DiagnosticEngine&    diags_;
    const DriverOptions& opts_;
    std::vector<std::unique_ptr<llvm::Module>> modules_;
};
} // namespace vex
