#include "vex/Sema/OrphanChecker.h"
namespace vex {
bool OrphanChecker::run(ModuleDecl*) { return true; }
bool OrphanChecker::checkImpl(ImplDecl*, ModuleDecl*) { return true; }
bool OrphanChecker::isLocal(TypeNode*, ModuleDecl*) { return true; }
bool OrphanChecker::isLocal(TraitDecl*, ModuleDecl*) { return true; }
}
