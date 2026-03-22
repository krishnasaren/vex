// src/CodeGen/LLVMEmitter.cpp
// ============================================================================
// vex/CodeGen/LLVMEmitter.cpp
// Translates VexIR → LLVM IR using the LLVM 17 C++ API.
// ============================================================================

#include "vex/CodeGen/LLVMEmitter.h"
#include "vex/Core/Assert.h"
#include "vex/Lexer/TokenKind.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

namespace vex {

// ── Constructor ───────────────────────────────────────────────────────────────

LLVMEmitter::LLVMEmitter(VexContext&        vexCtx,
                          llvm::LLVMContext& llvmCtx,
                          const std::string& moduleName)
    : vexCtx_(vexCtx)
    , llvmCtx_(llvmCtx)
    , builder_(llvmCtx)
{
    llvmMod_ = std::make_unique<llvm::Module>(moduleName, llvmCtx);
    declareRuntimeFunctions();
}

// ── Main entry point ──────────────────────────────────────────────────────────

std::unique_ptr<llvm::Module> LLVMEmitter::emit(IRModule* irMod) {
    VEX_ASSERT(irMod != nullptr, "LLVMEmitter::emit() called with null IRModule");

    // First pass: declare all functions so forward calls work
    for (auto& fn : irMod->functions()) {
        auto* llvmFn = llvm::Function::Create(
            llvm::FunctionType::get(voidType(), {}, false),
            llvm::Function::ExternalLinkage,
            std::string(fn->name()),
            llvmMod_.get());
        fnMap_[std::string(fn->name())] = llvmFn;
    }

    // Second pass: emit bodies
    for (auto& fn : irMod->functions())
        emitFunction(fn.get());

    // Emit globals
    for (auto& g : irMod->globals()) {
        auto* gv = new llvm::GlobalVariable(
            *llvmMod_,
            intType(64),
            !g->isMutable(),
            llvm::GlobalValue::InternalLinkage,
            llvm::ConstantInt::get(intType(64), 0),
            std::string(g->name()));
        (void)gv;
    }

    // Verify the module
    std::string errStr;
    llvm::raw_string_ostream errOS(errStr);
    if (llvm::verifyModule(*llvmMod_, &errOS)) {
        // Non-fatal: the module may have errors during development
        llvm::errs() << "VEX IR verification warning: " << errStr << "\n";
    }

    return std::move(llvmMod_);
}

// ── Function emission ─────────────────────────────────────────────────────────

void LLVMEmitter::emitFunction(IRFunction* fn) {
    auto it = fnMap_.find(std::string(fn->name()));
    VEX_ASSERT(it != fnMap_.end(), "Function not pre-declared");
    llvm::Function* llvmFn = it->second;

    // Rebuild function type with actual params
    std::vector<llvm::Type*> paramTypes(fn->params().size(), intType(64));
    auto* fnType = llvm::FunctionType::get(intType(64), paramTypes, false);

    // Replace the placeholder function
    llvmFn->eraseFromParent();
    llvmFn = llvm::Function::Create(
        fnType,
        llvm::Function::ExternalLinkage,
        std::string(fn->name()),
        llvmMod_.get());
    fnMap_[std::string(fn->name())] = llvmFn;

    // Create LLVM basic blocks for all VexIR blocks first
    blockMap_.clear();
    for (auto& irBB : fn->blocks()) {
        auto* llvmBB = llvm::BasicBlock::Create(
            llvmCtx_,
            std::string(irBB->label()),
            llvmFn);
        blockMap_[irBB.get()] = llvmBB;
    }

    // Map params
    auto llvmArgIt = llvmFn->arg_begin();
    for (auto& param : fn->params()) {
        llvmArgIt->setName(std::string(param->name()));
        valueMap_[param.get()] = &*llvmArgIt;
        ++llvmArgIt;
    }

    // Emit each block
    for (auto& irBB : fn->blocks())
        emitBlock(irBB.get(), llvmFn);
}

void LLVMEmitter::emitBlock(IRBlock* irBB, llvm::Function* /*llvmFn*/) {
    auto* llvmBB = blockMap_[irBB];
    builder_.SetInsertPoint(llvmBB);

    for (auto& instr : irBB->instrs()) {
        auto* val = emitInstr(instr.get());
        if (val) valueMap_[instr.get()] = val;
    }
}

llvm::Value* LLVMEmitter::emitInstr(IRInstr* instr) {
    switch (instr->kind()) {
        case IRInstrKind::Return:       return emitReturn(static_cast<IRReturnInstr*>(instr));
        case IRInstrKind::Br:           return emitBr(static_cast<IRBrInstr*>(instr));
        case IRInstrKind::CondBr:       return emitCondBr(static_cast<IRCondBrInstr*>(instr));
        case IRInstrKind::Alloca:       return emitAlloca(static_cast<IRAllocaInstr*>(instr));
        case IRInstrKind::Store:        return emitStore(static_cast<IRStoreInstr*>(instr));
        case IRInstrKind::Load:         return emitLoad(static_cast<IRLoadInstr*>(instr));
        case IRInstrKind::Call:         return emitCall(static_cast<IRCallInstr*>(instr));
        case IRInstrKind::BinOp:        return emitBinOp(static_cast<IRBinOpInstr*>(instr));
        case IRInstrKind::UnaryOp:      return emitUnaryOp(static_cast<IRUnaryOpInstr*>(instr));
        case IRInstrKind::Cmp:          return emitCmp(static_cast<IRCmpInstr*>(instr));
        case IRInstrKind::Gep:          return emitGep(static_cast<IRGepInstr*>(instr));
        case IRInstrKind::Index:        return emitIndex(static_cast<IRIndexInstr*>(instr));
        case IRInstrKind::Phi:          return emitPhi(static_cast<IRPhiInstr*>(instr));
        case IRInstrKind::IntConst:     return emitIntConst(static_cast<IRIntConstInstr*>(instr));
        case IRInstrKind::FloatConst:   return emitFloatConst(static_cast<IRFloatConstInstr*>(instr));
        case IRInstrKind::BoolConst:    return emitBoolConst(static_cast<IRBoolConstInstr*>(instr));
        case IRInstrKind::StringConst:  return emitStringConst(static_cast<IRStringConstInstr*>(instr));
        case IRInstrKind::Const:        return emitConst(static_cast<IRConstInstr*>(instr));
        case IRInstrKind::Cast:         return emitCast(static_cast<IRCastInstr*>(instr));
        default:
            // Unimplemented instructions return undef
            return llvm::UndefValue::get(intType(64));
    }
}

// ── Terminators ───────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitReturn(IRReturnInstr* i) {
    if (!i->value() || !valueMap_.count(i->value()))
        return builder_.CreateRetVoid();
    return builder_.CreateRet(lookup(i->value()));
}

llvm::Value* LLVMEmitter::emitBr(IRBrInstr* i) {
    auto* target = blockMap_[i->target()];
    VEX_ASSERT(target, "IRBrInstr: target block not mapped");
    return builder_.CreateBr(target);
}

llvm::Value* LLVMEmitter::emitCondBr(IRCondBrInstr* i) {
    auto* cond   = lookup(i->cond());
    auto* thenBB = blockMap_[i->thenBB()];
    auto* elseBB = blockMap_[i->elseBB()];
    VEX_ASSERT(thenBB && elseBB, "IRCondBrInstr: block not mapped");
    // Ensure cond is i1
    if (cond->getType() != boolType())
        cond = builder_.CreateICmpNE(cond,
                   llvm::ConstantInt::get(cond->getType(), 0));
    return builder_.CreateCondBr(cond, thenBB, elseBB);
}

// ── Memory ────────────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitAlloca(IRAllocaInstr* i) {
    // Save + restore insertion point to insert allocas in entry block
    auto* currBB = builder_.GetInsertBlock();
    auto* fn     = currBB->getParent();
    auto& entryBB = fn->getEntryBlock();
    builder_.SetInsertPoint(&entryBB, entryBB.begin());
    auto* alloca = builder_.CreateAlloca(intType(64), nullptr,
                                         std::string(i->varName()));
    builder_.SetInsertPoint(currBB);
    return alloca;
}

llvm::Value* LLVMEmitter::emitStore(IRStoreInstr* i) {
    auto* ptr = lookup(i->ptr());
    auto* val = lookup(i->val());
    if (!ptr || !val) return nullptr;
    // Cast val to match ptr element type if needed
    return builder_.CreateStore(val, ptr);
}

llvm::Value* LLVMEmitter::emitLoad(IRLoadInstr* i) {
    auto* ptr = lookup(i->ptr());
    if (!ptr) return llvm::UndefValue::get(intType(64));
    return builder_.CreateLoad(intType(64), ptr);
}

// ── Call ──────────────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitCall(IRCallInstr* i) {
    std::vector<llvm::Value*> args;
    for (auto* arg : i->args()) {
        auto* v = valueMap_.count(arg) ? valueMap_[arg]
                                       : llvm::UndefValue::get(intType(64));
        args.push_back(v);
    }

    if (i->hasNamedCallee()) {
        // Look up in runtime or declared functions
        std::string name(i->calleeName());
        llvm::Function* fn = llvmMod_->getFunction(name);
        if (!fn) fn = getRuntimeFn(name);
        if (!fn) {
            // Forward declare as i64(...) so we can still link
            auto* fty = llvm::FunctionType::get(intType(64), {}, true);
            fn = llvm::Function::Create(fty,
                     llvm::Function::ExternalLinkage, name, llvmMod_.get());
        }
        return builder_.CreateCall(fn, args);
    } else {
        auto* calleeVal = lookup(i->calleeVal());
        if (!calleeVal) return llvm::UndefValue::get(intType(64));
        auto* fty = llvm::FunctionType::get(intType(64), {}, true);
        return builder_.CreateCall(fty, calleeVal, args);
    }
}

// ── Arithmetic ────────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitBinOp(IRBinOpInstr* i) {
    auto* lhs = lookup(i->lhs());
    auto* rhs = lookup(i->rhs());
    if (!lhs || !rhs) return llvm::UndefValue::get(intType(64));

    switch (i->op()) {
        case TokenKind::Plus:      return builder_.CreateAdd(lhs, rhs, "add");
        case TokenKind::Minus:     return builder_.CreateSub(lhs, rhs, "sub");
        case TokenKind::Star:      return builder_.CreateMul(lhs, rhs, "mul");
        case TokenKind::Slash:     return builder_.CreateSDiv(lhs, rhs, "div");
        case TokenKind::Percent:   return builder_.CreateSRem(lhs, rhs, "rem");
        case TokenKind::StarStar:  {
            // Emit call to llvm.powi or runtime pow
            auto* powFn = getRuntimeFn("__vex_pow_int");
            if (powFn) return builder_.CreateCall(powFn, {lhs, rhs});
            return builder_.CreateMul(lhs, rhs); // fallback
        }
        case TokenKind::Amp:       return builder_.CreateAnd(lhs, rhs, "and");
        case TokenKind::Pipe:      return builder_.CreateOr(lhs, rhs, "or");
        case TokenKind::Caret:     return builder_.CreateXor(lhs, rhs, "xor");
        case TokenKind::LShift:    return builder_.CreateShl(lhs, rhs, "shl");
        case TokenKind::RShift:    return builder_.CreateAShr(lhs, rhs, "ashr");
        case TokenKind::URShift:   return builder_.CreateLShr(lhs, rhs, "lshr");
        case TokenKind::AmpAmp:
        case TokenKind::KW_and:    return builder_.CreateAnd(lhs, rhs, "land");
        case TokenKind::PipePipe:
        case TokenKind::KW_or:     return builder_.CreateOr(lhs, rhs, "lor");
        // Wrapping arithmetic
        case TokenKind::WrapAdd:   return builder_.CreateAdd(lhs, rhs, "wrapadd",
                                                              /*NUW*/true, /*NSW*/false);
        case TokenKind::WrapSub:   return builder_.CreateSub(lhs, rhs, "wrapsub",
                                                              true, false);
        case TokenKind::WrapMul:   return builder_.CreateMul(lhs, rhs, "wrapmul",
                                                              true, false);
        // Saturating: emit runtime calls
        case TokenKind::SatAdd: {
            auto* fn = getRuntimeFn("__vex_sat_add_i64");
            if (fn) return builder_.CreateCall(fn, {lhs, rhs});
            return builder_.CreateAdd(lhs, rhs);
        }
        case TokenKind::SatSub: {
            auto* fn = getRuntimeFn("__vex_sat_sub_i64");
            if (fn) return builder_.CreateCall(fn, {lhs, rhs});
            return builder_.CreateSub(lhs, rhs);
        }
        case TokenKind::SatMul: {
            auto* fn = getRuntimeFn("__vex_sat_mul_i64");
            if (fn) return builder_.CreateCall(fn, {lhs, rhs});
            return builder_.CreateMul(lhs, rhs);
        }
        default:
            return llvm::UndefValue::get(intType(64));
    }
}

llvm::Value* LLVMEmitter::emitUnaryOp(IRUnaryOpInstr* i) {
    auto* operand = lookup(i->operand());
    if (!operand) return llvm::UndefValue::get(intType(64));

    switch (i->op()) {
        case TokenKind::Minus:
            return builder_.CreateNeg(operand, "neg");
        case TokenKind::Bang:
        case TokenKind::KW_not:
            return builder_.CreateNot(operand, "not");
        case TokenKind::Tilde:
            return builder_.CreateNot(operand, "bitnot");
        case TokenKind::PlusPlus: {
            auto* one  = llvm::ConstantInt::get(intType(64), 1);
            auto* newv = builder_.CreateAdd(operand, one, "inc");
            // For postfix: return old value; for prefix: return new value
            return i->isPostfix() ? operand : newv;
        }
        case TokenKind::MinusMinus: {
            auto* one  = llvm::ConstantInt::get(intType(64), 1);
            auto* newv = builder_.CreateSub(operand, one, "dec");
            return i->isPostfix() ? operand : newv;
        }
        default:
            return operand;
    }
}

// ── Comparison ────────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitCmp(IRCmpInstr* i) {
    auto* lhs = lookup(i->lhs());
    auto* rhs = lookup(i->rhs());
    if (!lhs || !rhs) return llvm::ConstantInt::getFalse(llvmCtx_);

    switch (i->op()) {
        case IRCmpOp::Eq:  return builder_.CreateICmpEQ(lhs, rhs, "eq");
        case IRCmpOp::NEq: return builder_.CreateICmpNE(lhs, rhs, "ne");
        case IRCmpOp::Lt:  return builder_.CreateICmpSLT(lhs, rhs, "lt");
        case IRCmpOp::Lte: return builder_.CreateICmpSLE(lhs, rhs, "le");
        case IRCmpOp::Gt:  return builder_.CreateICmpSGT(lhs, rhs, "gt");
        case IRCmpOp::Gte: return builder_.CreateICmpSGE(lhs, rhs, "ge");
    }
    return llvm::ConstantInt::getFalse(llvmCtx_);
}

// ── Access ────────────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitGep(IRGepInstr* i) {
    auto* base = lookup(i->base());
    if (!base) return llvm::UndefValue::get(intType(64));
    // Simple GEP: treat base as pointer to struct, field offset = 0 for now
    // Full implementation requires type information from sema
    return base;
}

llvm::Value* LLVMEmitter::emitIndex(IRIndexInstr* i) {
    auto* base = lookup(i->base());
    auto* idx  = lookup(i->idx());
    if (!base || !idx) return llvm::UndefValue::get(intType(64));
    // Emit bounds-checked index via runtime call
    auto* fn = getRuntimeFn("__vex_list_get");
    if (fn) return builder_.CreateCall(fn, {base, idx});
    return builder_.CreateAdd(base, idx); // fallback placeholder
}

// ── SSA ───────────────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitPhi(IRPhiInstr* i) {
    auto* phi = builder_.CreatePHI(intType(64),
                                    static_cast<unsigned>(i->incoming().size()),
                                    "phi");
    for (auto& [irVal, irBB] : i->incoming()) {
        auto* llvmVal = valueMap_.count(irVal) ? valueMap_[irVal]
                                               : llvm::UndefValue::get(intType(64));
        auto* llvmBB  = blockMap_.count(irBB) ? blockMap_[irBB] : nullptr;
        if (llvmBB)
            phi->addIncoming(llvmVal, llvmBB);
    }
    return phi;
}

// ── Constants ─────────────────────────────────────────────────────────────────

llvm::Value* LLVMEmitter::emitIntConst(IRIntConstInstr* i) {
    // Choose LLVM integer type based on suffix
    llvm::Type* ty;
    switch (i->suffix()) {
        case IntSuffix::u8:  case IntSuffix::i8:   ty = intType(8);   break;
        case IntSuffix::u16: case IntSuffix::i16:  ty = intType(16);  break;
        case IntSuffix::u32: case IntSuffix::i32:  ty = intType(32);  break;
        default:                                    ty = intType(64);  break;
    }
    return llvm::ConstantInt::get(ty, i->value(), /*isSigned=*/true);
}

llvm::Value* LLVMEmitter::emitFloatConst(IRFloatConstInstr* i) {
    llvm::Type* ty = (i->suffix() == FloatSuffix::f)
                     ? floatType(true) : floatType(false);
    return llvm::ConstantFP::get(ty, i->value());
}

llvm::Value* LLVMEmitter::emitBoolConst(IRBoolConstInstr* i) {
    return llvm::ConstantInt::get(boolType(), i->value() ? 1 : 0);
}

llvm::Value* LLVMEmitter::emitStringConst(IRStringConstInstr* i) {
    // Create a global constant string and return a fat pointer struct
    auto* strConst = builder_.CreateGlobalStringPtr(
        std::string(i->value()), ".str");
    auto* lenVal   = llvm::ConstantInt::get(intType(64), i->value().size());

    // Build { i8*, i64 } struct
    auto* strTy = strType();
    auto* agg   = llvm::UndefValue::get(strTy);
    auto* withPtr = builder_.CreateInsertValue(agg,  strConst, {0});
    auto* withLen = builder_.CreateInsertValue(withPtr, lenVal,  {1});
    return withLen;
}

llvm::Value* LLVMEmitter::emitConst(IRConstInstr* i) {
    switch (i->constKind()) {
        case IRConstKind::Null:  return llvm::ConstantPointerNull::get(
                                     llvm::PointerType::get(llvmCtx_, 0));
        case IRConstKind::Void:  return nullptr;
        case IRConstKind::Undef: return llvm::UndefValue::get(intType(64));
    }
    return nullptr;
}

llvm::Value* LLVMEmitter::emitCast(IRCastInstr* i) {
    auto* val = lookup(i->val());
    if (!val) return llvm::UndefValue::get(intType(64));
    // Type-directed cast; full implementation requires sema type info
    return builder_.CreateBitOrPointerCast(val, intType(64), "cast");
}

// ── Type helpers ──────────────────────────────────────────────────────────────

llvm::Type* LLVMEmitter::intType(unsigned bits) {
    return llvm::IntegerType::get(llvmCtx_, bits);
}

llvm::Type* LLVMEmitter::floatType(bool is32bit) {
    return is32bit ? llvm::Type::getFloatTy(llvmCtx_)
                   : llvm::Type::getDoubleTy(llvmCtx_);
}

llvm::Type* LLVMEmitter::boolType() {
    return llvm::IntegerType::get(llvmCtx_, 1);
}

llvm::Type* LLVMEmitter::voidType() {
    return llvm::Type::getVoidTy(llvmCtx_);
}

llvm::Type* LLVMEmitter::ptrType() {
    return llvm::PointerType::get(llvmCtx_, 0); // opaque ptr
}

llvm::Type* LLVMEmitter::strType() {
    // { i8*, i64 }
    return llvm::StructType::get(llvmCtx_, {ptrType(), intType(64)});
}

llvm::Type* LLVMEmitter::listType() {
    // { i8*, i64, i64 }
    return llvm::StructType::get(llvmCtx_,
                                  {ptrType(), intType(64), intType(64)});
}

llvm::Type* LLVMEmitter::mapType(TypeNode* /*type*/) {
    return intType(64); // placeholder
}

// ── Runtime function declarations ─────────────────────────────────────────────

void LLVMEmitter::declareRuntimeFunctions() {
    // vex_rt memory
    {
        auto* fty = llvm::FunctionType::get(ptrType(),
                    {intType(64), intType(64)}, false);
        runtimeFns_["vex_alloc"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "vex_alloc", llvmMod_.get());
    }
    {
        auto* fty = llvm::FunctionType::get(voidType(), {ptrType()}, false);
        runtimeFns_["vex_free"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "vex_free", llvmMod_.get());
    }
    // vex_rt panic
    {
        auto* fty = llvm::FunctionType::get(voidType(),
                    {ptrType(), ptrType(), intType(32)}, false);
        auto* fn  = llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                            "vex_panic", llvmMod_.get());
        fn->addFnAttr(llvm::Attribute::NoReturn);
        runtimeFns_["vex_panic"] = fn;
    }
    // vex_rt io
    {
        // VexStr = { i8*, i64 }
        auto* strTy = strType();
        auto* fty   = llvm::FunctionType::get(voidType(), {strTy}, false);
        runtimeFns_["vex_println"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "vex_println", llvmMod_.get());
        runtimeFns_["vex_print"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "vex_print", llvmMod_.get());
    }
    // vex_rt ARC
    {
        auto* fty = llvm::FunctionType::get(ptrType(),
                    {ptrType(), intType(64), intType(64), ptrType()}, false);
        runtimeFns_["vex_arc_alloc"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "vex_arc_alloc", llvmMod_.get());
    }
    {
        auto* fty = llvm::FunctionType::get(ptrType(), {ptrType()}, false);
        runtimeFns_["vex_arc_clone"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "vex_arc_clone", llvmMod_.get());
    }
    {
        auto* fty = llvm::FunctionType::get(voidType(), {ptrType()}, false);
        runtimeFns_["vex_arc_drop"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "vex_arc_drop", llvmMod_.get());
    }
    // Internal helpers
    {
        auto* fty = llvm::FunctionType::get(intType(64),
                    {intType(64), intType(64)}, false);
        runtimeFns_["__vex_pow_int"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__vex_pow_int", llvmMod_.get());
        runtimeFns_["__vex_sat_add_i64"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__vex_sat_add_i64", llvmMod_.get());
        runtimeFns_["__vex_sat_sub_i64"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__vex_sat_sub_i64", llvmMod_.get());
        runtimeFns_["__vex_sat_mul_i64"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__vex_sat_mul_i64", llvmMod_.get());
    }
    // List access
    {
        auto* fty = llvm::FunctionType::get(intType(64),
                    {ptrType(), intType(64)}, false);
        runtimeFns_["__vex_list_get"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__vex_list_get", llvmMod_.get());
    }
    // Pattern matching helper
    {
        auto* fty = llvm::FunctionType::get(boolType(), {ptrType()}, false);
        runtimeFns_["__pattern_match"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__pattern_match", llvmMod_.get());
    }
    // Result helpers
    {
        auto* fty = llvm::FunctionType::get(boolType(), {intType(64)}, false);
        runtimeFns_["__result_is_ok"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__result_is_ok", llvmMod_.get());
    }
    {
        auto* fty = llvm::FunctionType::get(intType(64), {intType(64)}, false);
        runtimeFns_["__result_ok"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__result_ok", llvmMod_.get());
        runtimeFns_["__result_err"] =
            llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                   "__result_err", llvmMod_.get());
    }
}

llvm::Function* LLVMEmitter::getRuntimeFn(const std::string& name) {
    auto it = runtimeFns_.find(name);
    if (it != runtimeFns_.end()) return it->second;
    return llvmMod_->getFunction(name);
}

llvm::Value* LLVMEmitter::lookup(IRValue* val) {
    if (!val) return nullptr;
    auto it = valueMap_.find(val);
    if (it != valueMap_.end()) return it->second;
    // Might be a constant we can reconstruct
    return llvm::UndefValue::get(intType(64));
}

} // namespace vex