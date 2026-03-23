#pragma once
// include/vex/Sema/OwnershipRegion.h



// ============================================================================
// vex/Sema/OwnershipRegion.h  — Ownership regions (similar to Rust NLL regions)
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"
#include <unordered_set>
#include <string>

namespace vex {

struct OwnershipState {
    bool owned    = false;
    bool moved    = false;
    bool borrowed = false;
    bool mutBorrowed = false;
    bool dropped  = false;
};

class OwnershipRegion {
public:
    explicit OwnershipRegion(SemaContext& ctx) : ctx_(ctx) {}

    bool analyze(FnDecl* fn);

    OwnershipState stateOf(std::string_view varName) const;

    void markMoved(std::string_view name);
    void markDropped(std::string_view name);
    void markBorrowed(std::string_view name, bool mut);
    void releaseBorrow(std::string_view name);

private:
    SemaContext& ctx_;
    std::unordered_map<std::string, OwnershipState> states_;
};

} // namespace vex
