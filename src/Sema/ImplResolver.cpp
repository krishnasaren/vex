#include "vex/Sema/ImplResolver.h"
namespace vex {
void ImplResolver::registerImpl(ImplDecl*) {}
FnDecl* ImplResolver::resolveMethod(TypeNode*, std::string_view,
    const std::vector<TypeNode*>&, SourceLocation) { return nullptr; }
FnDecl* ImplResolver::resolveTraitMethod(TypeNode*, TraitDecl*, std::string_view,
    const std::vector<TypeNode*>&, SourceLocation) { return nullptr; }
bool ImplResolver::methodMatches(FnDecl*, const std::vector<TypeNode*>&) { return false; }
}
