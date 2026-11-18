#include "vex/Sema/LifetimeAnalysis.h"
namespace vex {
bool LifetimeAnalysis::analyze(FnDecl*) { return true; }
const Lifetime* LifetimeAnalysis::lifetime(std::string_view) const { return nullptr; }
}
