#include "vex/Sema/OverflowChecker.h"
namespace vex {
bool OverflowChecker::run(ModuleDecl*) { return true; }
bool OverflowChecker::checkExpr(ExprNode*) { return true; }
bool OverflowChecker::checkBinary(BinaryExpr*) { return true; }
bool OverflowChecker::checkUnary(UnaryExpr*) { return true; }
bool OverflowChecker::checkLiteral(IntLitExpr*) { return true; }
bool OverflowChecker::checkCast(CastExpr*) { return true; }
}
