#pragma once
// include/vex/CodeGen/StmtEmitter.h



// ============================================================================
// vex/CodeGen/StmtEmitter.h  — Emits LLVM IR for statements
// ============================================================================
#include "vex/AST/Stmt.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"

namespace vex {
class LLVMEmitter;

class StmtEmitter {
public:
    StmtEmitter(LLVMEmitter& parent, llvm::IRBuilder<>& b)
        : parent_(parent), builder_(b) {}

    void emit(StmtNode* s);

    void emitBlock(BlockStmt* s);
    void emitVarDecl(VarDeclStmt* s);
    void emitReturn(ReturnStmt* s);
    void emitIf(IfStmt* s);
    void emitWhile(WhileStmt* s);
    void emitFor(ForStmt* s);
    void emitMatch(MatchStmt* s);
    void emitBreak(BreakStmt* s);
    void emitContinue(ContinueStmt* s);
    void emitDefer(DeferStmt* s);
    void emitYield(YieldStmt* s);
    void emitExprStmt(ExprStmt* s);
    void emitImplicitReturn(llvm::Value* retVal);

    // Loop exit / continue targets
    llvm::BasicBlock* breakTarget    = nullptr;
    llvm::BasicBlock* continueTarget = nullptr;

private:
    LLVMEmitter&      parent_;
    llvm::IRBuilder<>& builder_;
};
} // namespace vex
