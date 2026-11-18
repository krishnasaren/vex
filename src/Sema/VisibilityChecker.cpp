#include "vex/Sema/VisibilityChecker.h"
namespace vex {

bool VisibilityChecker::run(ModuleDecl* mod) {
    if (!mod) return true;
    for (auto* d : mod->decls) visit(static_cast<DeclNode*>(d));
    return !ctx_.diags().hasErrors();
}

void VisibilityChecker::visitIdentExpr(IdentExpr*)   {}
void VisibilityChecker::visitMemberExpr(MemberExpr*) {}
void VisibilityChecker::visitCallExpr(CallExpr*)     {}

bool VisibilityChecker::isAccessible(Symbol*, SourceLocation) { return true; }
void VisibilityChecker::reportNotAccessible(Symbol*, SourceLocation) {}

} // namespace vex
