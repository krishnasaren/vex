#pragma once
// ============================================================================
// vex/CodeGen/LLVMEmitter.h
// Translates VexIR into LLVM IR using the LLVM C++ API.
// ============================================================================
/*
#include "vex/IR/VexIR.h"
#include "vex/Core/VexContext.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/Target/TargetMachine.h>

#include <memory>
#include <unordered_map>
#include <string>

namespace vex {

class LLVMEmitter {
public:
    LLVMEmitter(VexContext& vexCtx,
                llvm::LLVMContext& llvmCtx,
                const std::string& moduleName);

    // Emit the full VexIR module into an LLVM module
    std::unique_ptr<llvm::Module> emit(IRModule* irMod);

    // Get the resulting LLVM module (after emit())
    llvm::Module* llvmModule() const { return llvmMod_.get(); }

private:
    VexContext&         vexCtx_;
    llvm::LLVMContext&  llvmCtx_;
    llvm::IRBuilder<>   builder_;
    std::unique_ptr<llvm::Module> llvmMod_;

    // Maps VexIR values to LLVM values
    std::unordered_map<IRValue*, llvm::Value*> valueMap_;
    // Maps VexIR blocks to LLVM basic blocks
    std::unordered_map<IRBlock*, llvm::BasicBlock*> blockMap_;
    // Maps function names to LLVM functions
    std::unordered_map<std::string, llvm::Function*> fnMap_;

    // ── Emission helpers ──────────────────────────────────────────────────────
    void        emitFunction(IRFunction* fn);
    void        emitBlock(IRBlock* irBB, llvm::Function* llvmFn);
    llvm::Value* emitInstr(IRInstr* instr);

    llvm::Value* emitReturn(IRReturnInstr* i);
    llvm::Value* emitBr(IRBrInstr* i);
    llvm::Value* emitCondBr(IRCondBrInstr* i);
    llvm::Value* emitAlloca(IRAllocaInstr* i);
    llvm::Value* emitStore(IRStoreInstr* i);
    llvm::Value* emitLoad(IRLoadInstr* i);
    llvm::Value* emitCall(IRCallInstr* i);
    llvm::Value* emitBinOp(IRBinOpInstr* i);
    llvm::Value* emitUnaryOp(IRUnaryOpInstr* i);
    llvm::Value* emitCmp(IRCmpInstr* i);
    llvm::Value* emitGep(IRGepInstr* i);
    llvm::Value* emitIndex(IRIndexInstr* i);
    llvm::Value* emitPhi(IRPhiInstr* i);
    llvm::Value* emitIntConst(IRIntConstInstr* i);
    llvm::Value* emitFloatConst(IRFloatConstInstr* i);
    llvm::Value* emitBoolConst(IRBoolConstInstr* i);
    llvm::Value* emitStringConst(IRStringConstInstr* i);
    llvm::Value* emitConst(IRConstInstr* i);
    llvm::Value* emitCast(IRCastInstr* i);

    // ── Type mapping ─────────────────────────────────────────────────────────
    llvm::Type* mapType(TypeNode* type);
    llvm::Type* intType(unsigned bits);
    llvm::Type* floatType(bool is32bit);
    llvm::Type* boolType();
    llvm::Type* voidType();
    llvm::Type* ptrType();
    llvm::Type* strType();   // { i8*, i64 } fat pointer
    llvm::Type* listType();  // { i8*, i64, i64 } fat pointer

    // ── Runtime function declarations ─────────────────────────────────────────
    void declareRuntimeFunctions();
    llvm::Function* getRuntimeFn(const std::string& name);
    std::unordered_map<std::string, llvm::Function*> runtimeFns_;

    // Lookup a mapped LLVM value (asserts if not found)
    llvm::Value* lookup(IRValue* val);
};

} // namespace vex

*/
// ============================================================================
// vex/CodeGen/LLVMEmitter.h  — Emits LLVM IR from VEX IR
// ============================================================================
#include "vex/IR/IRModule.h"
#include "vex/CodeGen/LLVMTypeMapper.h"
#include "vex/CodeGen/LLVMDebugInfo.h"
#include "vex/CodeGen/ABI.h"
#include "vex/Driver/Options.h"
#include "vex/Core/DiagnosticEngine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include <unordered_map>
#include <memory>

namespace vex {

class LLVMEmitter {
public:
    LLVMEmitter(llvm::Module& llvmMod,
                DiagnosticEngine& diags,
                const DriverOptions& opts);

    bool emit(IRModule& vexMod);

private:
    llvm::Module&        llvmMod_;
    llvm::LLVMContext&   ctx_;
    llvm::IRBuilder<>    builder_;
    DiagnosticEngine&    diags_;
    const DriverOptions& opts_;
    LLVMTypeMapper       typeMap_;
    std::unique_ptr<LLVMDebugInfoEmitter> debugInfo_;
    std::unique_ptr<ABIInfo>             abi_;

    // VEX IR value → LLVM value mapping
    std::unordered_map<const IRValue*, llvm::Value*> valueMap_;
    // VEX IR block → LLVM basic block mapping
    std::unordered_map<const IRBlock*, llvm::BasicBlock*> blockMap_;

    void emitFunction(const IRFunction& fn);
    void emitBlock(const IRBlock& blk);
    llvm::Value* emitInstr(const IRInstr& instr);
    llvm::Value* emitConst(const IRValue& v);

    llvm::Value* emitArith(const IRInstr& i);
    llvm::Value* emitCmp(const IRInstr& i);
    llvm::Value* emitMemory(const IRInstr& i);
    llvm::Value* emitControl(const IRInstr& i);
    llvm::Value* emitCall(const IRInstr& i);
    llvm::Value* emitConvert(const IRInstr& i);
    llvm::Value* emitAggregate(const IRInstr& i);
    llvm::Value* emitAtomic(const IRInstr& i);
    llvm::Value* emitPhi(const IRInstr& i);

    llvm::Value*     lookup(const IRValue* v);
    llvm::BasicBlock* lookupBlock(const IRBlock* b);
    llvm::Function*   declareFn(const IRFunction& fn);
    llvm::GlobalVariable* declareGlobal(const IRGlobal& g);
};

} // namespace vex