// src/CodeGen/LLVMEmitter.cpp — VexIR → LLVM IR translation
#include "vex/CodeGen/LLVMEmitter.h"
#include "vex/IR/VexIR.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Verifier.h"

namespace vex {

LLVMEmitter::LLVMEmitter(llvm::Module& llvmMod,
                          DiagnosticEngine& diags,
                          const DriverOptions& opts)
    : llvmMod_(llvmMod)
    , ctx_(llvmMod.getContext())
    , builder_(llvmMod.getContext())
    , diags_(diags)
    , opts_(opts)
    , typeMap_(llvmMod.getContext())
{}

bool LLVMEmitter::emit(IRModule& vexMod) {
    // Forward-declare all functions
    for (auto& fn : vexMod.functions())
        declareFn(*fn);
    // Declare globals
    for (auto& g : vexMod.globals())
        declareGlobal(*g);
    // Emit bodies
    for (auto& fn : vexMod.functions()) {
        if (!fn->blocks().empty())
            emitFunction(*fn);
    }
    bool bad = llvm::verifyModule(llvmMod_, &llvm::errs());
    return !bad;
}

llvm::Function* LLVMEmitter::declareFn(const IRFunction& fn) {
    if (auto* ex = llvmMod_.getFunction(fn.name())) return ex;

    // Infer return type: if any return instruction carries a value → i64, else void.
    // Special case: main() → i32 (C ABI requirement)
    llvm::Type* retTy = typeMap_.voidTy();
    for (const auto& blk : fn.blocks()) {
        for (const auto& instr : blk->instrs()) {
            if (instr->kind() == IRInstrKind::Return) {
                const auto& r = static_cast<const IRReturnInstr&>(*instr);
                if (r.value()) { retTy = typeMap_.i64Ty(); goto done; }
            }
        }
    }
    if (fn.name() == "main") retTy = typeMap_.i32Ty();
    done:

    std::vector<llvm::Type*> paramTys;
    for (const auto& p : fn.params())
        paramTys.push_back(typeMap_.i64Ty());
    auto* fty = llvm::FunctionType::get(retTy, paramTys, false);
    bool isInternal = !fn.name().empty() && fn.name()[0] == '_'
                   && fn.name().size() > 1 && fn.name()[1] == '_';
    auto lnk = isInternal ? llvm::Function::InternalLinkage
                           : llvm::Function::ExternalLinkage;
    return llvm::Function::Create(fty, lnk, fn.name(), llvmMod_);
}

llvm::GlobalVariable* LLVMEmitter::declareGlobal(const IRGlobal& g) {
    llvm::Constant* init = llvm::Constant::getNullValue(typeMap_.i64Ty());
    return new llvm::GlobalVariable(llvmMod_, typeMap_.i64Ty(),
        !g.isMutable(), llvm::GlobalValue::ExternalLinkage, init, g.debugName());
}

void LLVMEmitter::emitFunction(const IRFunction& fn) {
    auto* llvmFn = llvmMod_.getFunction(fn.name());
    if (!llvmFn) llvmFn = declareFn(fn);

    valueMap_.clear();
    blockMap_.clear();

    // Map params → LLVM args
    size_t idx = 0;
    for (auto& arg : llvmFn->args()) {
        if (idx < fn.params().size())
            valueMap_[fn.params()[idx].get()] = &arg;
        ++idx;
    }
    // Create basic blocks
    for (const auto& blk : fn.blocks()) {
        auto* bb = llvm::BasicBlock::Create(ctx_, blk->label(), llvmFn);
        blockMap_[blk.get()] = bb;
    }
    // Emit instructions per block
    for (const auto& blk : fn.blocks())
        emitBlock(*blk);
}

void LLVMEmitter::emitBlock(const IRBlock& blk) {
    auto* bb = lookupBlock(&blk);
    if (!bb) return;
    builder_.SetInsertPoint(bb);
    for (const auto& instr : blk.instrs()) {
        llvm::Value* v = emitInstr(*instr);
        if (v) valueMap_[instr.get()] = v;
    }
}

llvm::Value* LLVMEmitter::emitInstr(const IRInstr& i) {
    switch (i.kind()) {
    case IRInstrKind::Return:    return emitControl(i);
    case IRInstrKind::Br:        return emitControl(i);
    case IRInstrKind::CondBr:    return emitControl(i);
    case IRInstrKind::Alloca:    return emitMemory(i);
    case IRInstrKind::Load:      return emitMemory(i);
    case IRInstrKind::Store:     return emitMemory(i);
    case IRInstrKind::Gep:       return emitMemory(i);
    case IRInstrKind::Call:      return emitCall(i);
    case IRInstrKind::BinOp:     return emitArith(i);
    case IRInstrKind::UnaryOp:   return emitArith(i);
    case IRInstrKind::Cmp:       return emitCmp(i);
    case IRInstrKind::Cast:      return emitConvert(i);
    case IRInstrKind::Phi:       return emitPhi(i);
    case IRInstrKind::IntConst:
    case IRInstrKind::FloatConst:
    case IRInstrKind::BoolConst:
    case IRInstrKind::StringConst:
    case IRInstrKind::Const:     return emitConst(i);
    case IRInstrKind::Unreachable: return builder_.CreateUnreachable();
    default:                     return nullptr;
    }
}

llvm::Value* LLVMEmitter::emitConst(const IRValue& v) {
    const auto& i = static_cast<const IRInstr&>(v);
    switch (i.kind()) {
    case IRInstrKind::IntConst:
        return llvm::ConstantInt::get(typeMap_.i64Ty(),
            static_cast<const IRIntConstInstr&>(i).value(), true);
    case IRInstrKind::FloatConst:
        return llvm::ConstantFP::get(typeMap_.f64Ty(),
            static_cast<const IRFloatConstInstr&>(i).value());
    case IRInstrKind::BoolConst:
        return llvm::ConstantInt::get(typeMap_.boolTy(),
            static_cast<const IRBoolConstInstr&>(i).value() ? 1 : 0);
    case IRInstrKind::StringConst:
        return builder_.CreateGlobalString(
            static_cast<const IRStringConstInstr&>(i).value(), ".str");
    case IRInstrKind::Const: {
        const auto& cc = static_cast<const IRConstInstr&>(i);
        switch (cc.constKind()) {
        case IRConstKind::Null:  return llvm::ConstantPointerNull::get(typeMap_.ptrTy());
        case IRConstKind::Undef: return llvm::UndefValue::get(typeMap_.i64Ty());
        default:                 return llvm::UndefValue::get(typeMap_.i64Ty());
        }
    }
    default: return llvm::UndefValue::get(typeMap_.i64Ty());
    }
}

llvm::Value* LLVMEmitter::emitArith(const IRInstr& i) {
    if (i.kind() == IRInstrKind::BinOp) {
        const auto& b = static_cast<const IRBinOpInstr&>(i);
        llvm::Value* lhs = lookup(b.lhs());
        llvm::Value* rhs = lookup(b.rhs());
        if (!lhs || !rhs) return llvm::UndefValue::get(typeMap_.i64Ty());
        bool fp = lhs->getType()->isFloatingPointTy();
        switch (b.op()) {
        case TokenKind::Plus:    return fp ? builder_.CreateFAdd(lhs,rhs) : builder_.CreateAdd(lhs,rhs);
        case TokenKind::Minus:   return fp ? builder_.CreateFSub(lhs,rhs) : builder_.CreateSub(lhs,rhs);
        case TokenKind::Star:    return fp ? builder_.CreateFMul(lhs,rhs) : builder_.CreateMul(lhs,rhs);
        case TokenKind::Slash:   return fp ? builder_.CreateFDiv(lhs,rhs) : builder_.CreateSDiv(lhs,rhs);
        case TokenKind::Percent: return builder_.CreateSRem(lhs,rhs);
        case TokenKind::Amp:     return builder_.CreateAnd(lhs,rhs);
        case TokenKind::Pipe:    return builder_.CreateOr(lhs,rhs);
        case TokenKind::Caret:   return builder_.CreateXor(lhs,rhs);
        case TokenKind::LShift:  return builder_.CreateShl(lhs,rhs);
        case TokenKind::RShift:  return builder_.CreateAShr(lhs,rhs);
        case TokenKind::AmpAmp:  return builder_.CreateAnd(lhs,rhs);
        case TokenKind::PipePipe:return builder_.CreateOr(lhs,rhs);
        default: return llvm::UndefValue::get(lhs->getType());
        }
    }
    // UnaryOp
    const auto& u = static_cast<const IRUnaryOpInstr&>(i);
    llvm::Value* v = lookup(u.operand());
    if (!v) return llvm::UndefValue::get(typeMap_.i64Ty());
    switch (u.op()) {
    case TokenKind::Minus: return v->getType()->isFloatingPointTy()
                               ? builder_.CreateFNeg(v) : builder_.CreateNeg(v);
    case TokenKind::Bang:  return builder_.CreateNot(v);
    case TokenKind::Tilde: return builder_.CreateNot(v);
    default:               return v;
    }
}

llvm::Value* LLVMEmitter::emitCmp(const IRInstr& i) {
    const auto& c = static_cast<const IRCmpInstr&>(i);
    llvm::Value* lhs = lookup(c.lhs());
    llvm::Value* rhs = lookup(c.rhs());
    if (!lhs || !rhs) return llvm::ConstantInt::getFalse(ctx_);
    bool fp = lhs->getType()->isFloatingPointTy();
    switch (c.op()) {
    case IRCmpOp::Eq:  return fp ? builder_.CreateFCmpOEQ(lhs,rhs) : builder_.CreateICmpEQ(lhs,rhs);
    case IRCmpOp::NEq: return fp ? builder_.CreateFCmpONE(lhs,rhs) : builder_.CreateICmpNE(lhs,rhs);
    case IRCmpOp::Lt:  return fp ? builder_.CreateFCmpOLT(lhs,rhs) : builder_.CreateICmpSLT(lhs,rhs);
    case IRCmpOp::Lte: return fp ? builder_.CreateFCmpOLE(lhs,rhs) : builder_.CreateICmpSLE(lhs,rhs);
    case IRCmpOp::Gt:  return fp ? builder_.CreateFCmpOGT(lhs,rhs) : builder_.CreateICmpSGT(lhs,rhs);
    case IRCmpOp::Gte: return fp ? builder_.CreateFCmpOGE(lhs,rhs) : builder_.CreateICmpSGE(lhs,rhs);
    default: return llvm::ConstantInt::getFalse(ctx_);
    }
}

llvm::Value* LLVMEmitter::emitMemory(const IRInstr& i) {
    switch (i.kind()) {
    case IRInstrKind::Alloca: {
        const auto& a = static_cast<const IRAllocaInstr&>(i);
        return builder_.CreateAlloca(typeMap_.i64Ty(), nullptr, a.varName());
    }
    case IRInstrKind::Load: {
        llvm::Value* ptr = lookup(static_cast<const IRLoadInstr&>(i).ptr());
        if (!ptr) return llvm::UndefValue::get(typeMap_.i64Ty());
        return builder_.CreateLoad(typeMap_.i64Ty(), ptr, "load");
    }
    case IRInstrKind::Store: {
        const auto& s = static_cast<const IRStoreInstr&>(i);
        llvm::Value* val = lookup(s.val());
        llvm::Value* ptr = lookup(s.ptr());
        if (!val || !ptr) return nullptr;
        return builder_.CreateStore(val, ptr);
    }
    case IRInstrKind::Gep: {
        llvm::Value* base = lookup(static_cast<const IRGepInstr&>(i).base());
        if (!base) return llvm::UndefValue::get(typeMap_.ptrTy());
        llvm::Value* zero = llvm::ConstantInt::get(typeMap_.i32Ty(), 0);
        return builder_.CreateGEP(typeMap_.i8Ty(), base, {zero}, "gep");
    }
    default: return nullptr;
    }
}

llvm::Value* LLVMEmitter::emitControl(const IRInstr& i) {
    switch (i.kind()) {
    case IRInstrKind::Return: {
        const auto& r = static_cast<const IRReturnInstr&>(i);
        if (!r.value()) {
            // If function return type is void, ret void; else ret 0
            llvm::Function* f = builder_.GetInsertBlock()->getParent();
            if (f && !f->getReturnType()->isVoidTy())
                return builder_.CreateRet(llvm::Constant::getNullValue(f->getReturnType()));
            return builder_.CreateRetVoid();
        }
        llvm::Value* v = lookup(r.value());
        if (!v) return builder_.CreateRetVoid();
        llvm::Function* f = builder_.GetInsertBlock()->getParent();
        if (f && v->getType() != f->getReturnType()) {
            if (f->getReturnType()->isVoidTy()) return builder_.CreateRetVoid();
            v = builder_.CreateIntCast(v, f->getReturnType(), true, "retcast");
        }
        return builder_.CreateRet(v);
    }
    case IRInstrKind::Br: {
        llvm::BasicBlock* dest = lookupBlock(static_cast<const IRBrInstr&>(i).target());
        return dest ? builder_.CreateBr(dest) : nullptr;
    }
    case IRInstrKind::CondBr: {
        const auto& cb = static_cast<const IRCondBrInstr&>(i);
        llvm::Value* cond   = lookup(cb.cond());
        llvm::BasicBlock* t = lookupBlock(cb.thenBB());
        llvm::BasicBlock* f = lookupBlock(cb.elseBB());
        if (!cond || !t || !f) return nullptr;
        return builder_.CreateCondBr(cond, t, f);
    }
    default: return nullptr;
    }
}

llvm::Value* LLVMEmitter::emitCall(const IRInstr& i) {
    const auto& c = static_cast<const IRCallInstr&>(i);
    llvm::Function* fn = llvmMod_.getFunction(c.calleeName());
    if (!fn) {
        auto* fty = llvm::FunctionType::get(typeMap_.i64Ty(), {}, true);
        fn = llvm::Function::Create(fty, llvm::Function::ExternalLinkage,
                                     c.calleeName(), llvmMod_);
    }
    std::vector<llvm::Value*> args;
    for (auto* a : c.args()) {
        llvm::Value* v = lookup(a);
        if (v) args.push_back(v);
    }
    auto* fty = fn->getFunctionType();
    if (!fty->isVarArg()) {
        while (args.size() > fty->getNumParams()) args.pop_back();
        while (args.size() < fty->getNumParams())
            args.push_back(llvm::UndefValue::get(fty->getParamType((unsigned)args.size())));
    }
    return builder_.CreateCall(fty, fn, args);
}

llvm::Value* LLVMEmitter::emitConvert(const IRInstr& i) {
    const auto& c = static_cast<const IRCastInstr&>(i);
    llvm::Value* v = lookup(c.val());
    if (!v) return llvm::UndefValue::get(typeMap_.i64Ty());
    llvm::Type* dst = c.targetType() ? typeMap_.map(c.targetType()) : typeMap_.i64Ty();
    if (v->getType() == dst) return v;
    if (v->getType()->isIntegerTy() && dst->isIntegerTy())
        return builder_.CreateIntCast(v, dst, true, "icast");
    if (v->getType()->isFloatingPointTy() && dst->isFloatingPointTy())
        return builder_.CreateFPCast(v, dst, "fcast");
    if (v->getType()->isIntegerTy() && dst->isFloatingPointTy())
        return builder_.CreateSIToFP(v, dst, "i2f");
    if (v->getType()->isFloatingPointTy() && dst->isIntegerTy())
        return builder_.CreateFPToSI(v, dst, "f2i");
    return builder_.CreateBitOrPointerCast(v, dst, "bitcast");
}

llvm::Value* LLVMEmitter::emitPhi(const IRInstr& i) {
    const auto& ph = static_cast<const IRPhiInstr&>(i);
    auto* phi = builder_.CreatePHI(typeMap_.i64Ty(), (unsigned)ph.incoming().size(), "phi");
    for (const auto& [val, blk] : ph.incoming()) {
        llvm::Value* v = lookup(val);
        llvm::BasicBlock* bb = lookupBlock(blk);
        if (v && bb) phi->addIncoming(v, bb);
    }
    return phi;
}

llvm::Value* LLVMEmitter::emitAggregate(const IRInstr&) { return nullptr; }
llvm::Value* LLVMEmitter::emitAtomic(const IRInstr&)    { return nullptr; }

llvm::Value* LLVMEmitter::lookup(const IRValue* v) {
    if (!v) return nullptr;
    auto it = valueMap_.find(v);
    if (it != valueMap_.end()) return it->second;
    if (v->kind() == IRValueKind::Instruction)
        return emitConst(static_cast<const IRInstr&>(*v));
    return nullptr;
}

llvm::BasicBlock* LLVMEmitter::lookupBlock(const IRBlock* b) {
    if (!b) return nullptr;
    auto it = blockMap_.find(b);
    return it != blockMap_.end() ? it->second : nullptr;
}

} // namespace vex
