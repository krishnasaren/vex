#pragma once
// include/vex/IR/IRBuilder.h

// ============================================================================
// vex/IR/IRBuilder.h  — Convenience builder for constructing IR instructions
// ============================================================================
#include "vex/IR/IRModule.h"
#include "vex/IR/IRInstrKind.h"
#include <string>

namespace vex {

class IRBuilder {
public:
    explicit IRBuilder(IRModule& mod) : mod_(mod) {}

    // Set insertion point
    void setInsertPoint(IRBlock* block)   { block_ = block; }
    void setInsertPoint(IRFunction* fn)   { fn_ = fn; block_ = fn->entry(); }
    IRBlock*    insertBlock()    const    { return block_; }
    IRFunction* insertFunction() const   { return fn_; }

    // ── Constants ──
    IRValue* getInt(int64_t v, uint32_t bits = 64, bool isSigned = true);
    IRValue* getUInt(uint64_t v, uint32_t bits = 64);
    IRValue* getFloat(double v, uint32_t bits = 64);
    IRValue* getBool(bool v);
    IRValue* getNull(IRType* ptrType);
    IRValue* getUndef(IRType* ty);

    // ── Arithmetic ──
    IRInstr* createAdd(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createSub(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createMul(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createDiv(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createMod(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createNeg(IRValue* a,             std::string name = "");
    IRInstr* createFAdd(IRValue* a, IRValue* b,std::string name = "");
    IRInstr* createFSub(IRValue* a, IRValue* b,std::string name = "");
    IRInstr* createFMul(IRValue* a, IRValue* b,std::string name = "");
    IRInstr* createFDiv(IRValue* a, IRValue* b,std::string name = "");

    // ── Bitwise ──
    IRInstr* createAnd(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createOr (IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createXor(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createShl(IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createShr(IRValue* a, IRValue* b, std::string name = "");

    // ── Comparison ──
    IRInstr* createICmp(IRInstrKind pred, IRValue* a, IRValue* b, std::string name = "");
    IRInstr* createFCmp(IRInstrKind pred, IRValue* a, IRValue* b, std::string name = "");

    // ── Memory ──
    IRInstr* createAlloca(IRType* ty, IRValue* count = nullptr, std::string name = "");
    IRInstr* createLoad(IRType* ty, IRValue* ptr,  std::string name = "");
    IRInstr* createStore(IRValue* val, IRValue* ptr);
    IRInstr* createGEP(IRType* ty, IRValue* ptr,
                        std::vector<IRValue*> indices,  std::string name = "");

    // ── Control flow ──
    IRInstr* createBr(IRBlock* dest);
    IRInstr* createCondBr(IRValue* cond, IRBlock* thenB, IRBlock* elseB);
    IRInstr* createRet(IRValue* val);
    IRInstr* createRetVoid();
    IRInstr* createUnreachable();

    // ── Calls ──
    IRInstr* createCall(IRFunction* fn, std::vector<IRValue*> args, std::string name = "");
    IRInstr* createCall(IRValue* fnPtr, IRType* fnType,
                         std::vector<IRValue*> args, std::string name = "");
    IRInstr* createTailCall(IRFunction* fn, std::vector<IRValue*> args);

    // ── Conversions ──
    IRInstr* createZExt(IRValue* v, IRType* to,   std::string name = "");
    IRInstr* createSExt(IRValue* v, IRType* to,   std::string name = "");
    IRInstr* createTrunc(IRValue* v, IRType* to,  std::string name = "");
    IRInstr* createBitcast(IRValue* v, IRType* to,std::string name = "");
    IRInstr* createPhi(IRType* ty, std::string name = "");

    // ── Aggregates ──
    IRInstr* createExtractValue(IRValue* agg, unsigned idx, std::string name = "");
    IRInstr* createInsertValue(IRValue* agg, IRValue* val, unsigned idx, std::string name = "");

private:
    IRModule&   mod_;
    IRFunction* fn_    = nullptr;
    IRBlock*    block_ = nullptr;

    IRInstr* emit(std::unique_ptr<IRInstr> i);
};

} // namespace vex





