#include "vex/Sema/BorrowChecker.h"
namespace vex {
bool BorrowChecker::run(ModuleDecl*) { return true; }
void BorrowChecker::visitFnDecl(FnDecl*) {}
void BorrowChecker::visitBlockStmt(BlockStmt*) {}
void BorrowChecker::visitVarDeclStmt(VarDeclStmt*) {}
void BorrowChecker::visitAssignExpr(AssignExpr*) {}
void BorrowChecker::visitCallExpr(CallExpr*) {}

void BorrowChecker::visitReturnStmt(ReturnStmt*) {}
} // namespace vex
