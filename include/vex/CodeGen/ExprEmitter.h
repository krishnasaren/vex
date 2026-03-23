#pragma once
// include/vex/CodeGen/ExprEmitter.h


// ============================================================================
// vex/CodeGen/ExprEmitter.h  — Emits LLVM IR for expressions
// ============================================================================
#include "vex/AST/Expr.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

namespace vex {
class LLVMEmitter;

class ExprEmitter {
public:
    ExprEmitter(LLVMEmitter& parent, llvm::IRBuilder<>& b)
        : parent_(parent), builder_(b) {}

    llvm::Value* emit(ExprNode* e);

    llvm::Value* emitLiteral(LiteralExpr* e);
    llvm::Value* emitIdent(IdentExpr* e);
    llvm::Value* emitBinary(BinaryExpr* e);
    llvm::Value* emitUnary(UnaryExpr* e);
    llvm::Value* emitCall(CallExpr* e);
    llvm::Value* emitMember(MemberExpr* e);
    llvm::Value* emitIndex(IndexExpr* e);
    llvm::Value* emitIf(IfExpr* e);
    llvm::Value* emitMatch(MatchExpr* e);
    llvm::Value* emitClosure(ClosureExpr* e);
    llvm::Value* emitBlock(BlockExpr* e);
    llvm::Value* emitCast(CastExpr* e);
    llvm::Value* emitRange(RangeExpr* e);
    llvm::Value* emitTuple(TupleExpr* e);
    llvm::Value* emitArray(ArrayExpr* e);
    llvm::Value* emitAwait(AwaitExpr* e);
    llvm::Value* emitYield(YieldExpr* e);
    llvm::Value* emitTry(TryExpr* e);
    llvm::Value* emitStructLit(StructLitExpr* e);
    llvm::Value* emitFormatStr(FmtStrExpr* e);
    llvm::Value* emitAssign(AssignExpr* e);
    llvm::Value* emitCompoundAssign(CompoundAssignExpr* e);

private:
    LLVMEmitter&      parent_;
    llvm::IRBuilder<>& builder_;
};
} // namespace vex
