#include "vex/Sema/OwnershipRegion.h"
namespace vex {
bool OwnershipRegion::analyze(FnDecl*) { return true; }
OwnershipState OwnershipRegion::stateOf(std::string_view name) const {
    auto it = states_.find(std::string(name));
    return (it != states_.end()) ? it->second : OwnershipState{};
}
void OwnershipRegion::markMoved(std::string_view name) { states_[std::string(name)].moved = true; }
void OwnershipRegion::markDropped(std::string_view name) { states_[std::string(name)].dropped = true; }
void OwnershipRegion::markBorrowed(std::string_view name, bool mut) {
    auto& s = states_[std::string(name)];
    if (mut) s.mutBorrowed = true; else s.borrowed = true;
}
void OwnershipRegion::releaseBorrow(std::string_view name) {
    auto& s = states_[std::string(name)];
    s.borrowed = false; s.mutBorrowed = false;
}
}
