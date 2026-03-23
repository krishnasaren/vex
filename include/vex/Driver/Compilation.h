#pragma once
// include/vex/Driver/Compilation.h


// ============================================================================
// vex/Driver/Compilation.h  — Represents a complete compilation job
// (possibly multiple source files → single linked output)
// ============================================================================
#include "vex/Driver/CompilationUnit.h"
#include "vex/Driver/Options.h"
#include <vector>
#include <memory>

namespace vex {

class Compilation {
public:
    explicit Compilation(const DriverOptions& opts) : opts_(opts) {}

    void addUnit(std::unique_ptr<CompilationUnit> unit) {
        units_.push_back(std::move(unit));
    }

    const std::vector<std::unique_ptr<CompilationUnit>>& units() const { return units_; }
    const DriverOptions& options() const { return opts_; }

    bool hasErrors() const;
    void setFailed() { failed_ = true; }
    bool failed()    const { return failed_; }

private:
    DriverOptions                                  opts_;
    std::vector<std::unique_ptr<CompilationUnit>>  units_;
    bool                                           failed_ = false;
};

} // namespace vex
