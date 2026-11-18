#include "vex/Sema/ConstantFolder.h"
namespace vex {
bool      ConstantFolder::run(ModuleDecl*) { return true; }
bool      ConstantFolder::isConstant(ExprNode*) const { return false; }
ExprNode* ConstantFolder::fold(ExprNode* e) { return e; }
}
