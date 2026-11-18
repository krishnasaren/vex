#include "vex/Sema/TraitSolver.h"
namespace vex {
bool TraitSolver::checkImpl(ImplDecl*) { return true; }
bool TraitSolver::satisfies(TypeNode*, TraitDecl*, SourceLocation) { return true; }
std::vector<ImplDecl*> TraitSolver::impsFor(TypeNode*) { return {}; }
FnDecl* TraitSolver::lookupMethod(TypeNode*, std::string_view, const std::vector<TypeNode*>&) { return nullptr; }
bool TraitSolver::checkMethodPresent(ImplDecl*, FnDecl*) { return true; }
bool TraitSolver::checkSignatureMatch(FnDecl*, FnDecl*) { return true; }
}
