#include "vex/Sema/MoveChecker.h"
namespace vex {
bool MoveChecker::run(ModuleDecl*) { return true; }
void MoveChecker::visitFnDecl(FnDecl*) {}
void MoveChecker::visitBlockStmt(BlockStmt*) {}
void MoveChecker::visitVarDeclStmt(VarDeclStmt*) {}
void MoveChecker::visitAssignExpr(AssignExpr*) {}
void MoveChecker::visitCallExpr(CallExpr*) {}
void MoveChecker::visitReturnStmt(ReturnStmt*) {}
}
