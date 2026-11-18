// src/IR/IRBuilder.cpp
#include "vex/IR/IRBuilder.h"
#include "vex/IR/VexIR.h"
#include <memory>

namespace vex {

IRInstr* IRBuilder::emit(std::unique_ptr<IRInstr> i) {
    auto* ptr = i.get();
    block_->appendInstr(std::move(i));
    return ptr;
}

// ── Arithmetic ────────────────────────────────────────────────────────────────

static TokenKind opFor(const char* op) {
    // Simple map for binary op tokens
    if (op[0]=='+') return TokenKind::Plus;
    if (op[0]=='-') return TokenKind::Minus;
    if (op[0]=='*') return TokenKind::Star;
    if (op[0]=='/') return TokenKind::Slash;
    if (op[0]=='%') return TokenKind::Percent;
    if (op[0]=='&') return TokenKind::Amp;
    if (op[0]=='|') return TokenKind::Pipe;
    if (op[0]=='^') return TokenKind::Caret;
    return TokenKind::Plus;
}

#define MAKE_BINOP(name, tok)                                              \
    IRInstr* IRBuilder::name(IRValue* a, IRValue* b, std::string nm) {    \
        auto i = std::make_unique<IRBinOpInstr>(tok, a, b);               \
        if (!nm.empty()) i->setName(nm);                                   \
        return emit(std::move(i));                                         \
    }

MAKE_BINOP(createAdd,  TokenKind::Plus)
MAKE_BINOP(createSub,  TokenKind::Minus)
MAKE_BINOP(createMul,  TokenKind::Star)
MAKE_BINOP(createDiv,  TokenKind::Slash)
MAKE_BINOP(createMod,  TokenKind::Percent)
MAKE_BINOP(createFAdd, TokenKind::Plus)
MAKE_BINOP(createFSub, TokenKind::Minus)
MAKE_BINOP(createFMul, TokenKind::Star)
MAKE_BINOP(createFDiv, TokenKind::Slash)
MAKE_BINOP(createAnd,  TokenKind::Amp)
MAKE_BINOP(createOr,   TokenKind::Pipe)
MAKE_BINOP(createXor,  TokenKind::Caret)
MAKE_BINOP(createShl,  TokenKind::LShift)
MAKE_BINOP(createShr,  TokenKind::RShift)

IRInstr* IRBuilder::createNeg(IRValue* a, std::string nm) {
    auto i = std::make_unique<IRUnaryOpInstr>(TokenKind::Minus, a, false);
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

// ── Comparisons ───────────────────────────────────────────────────────────────

IRInstr* IRBuilder::createICmp(IRInstrKind pred, IRValue* a, IRValue* b, std::string nm) {
    IRCmpOp op = IRCmpOp::Eq;
    // pred is used as a tag here — just default
    (void)pred;
    auto i = std::make_unique<IRCmpInstr>(op, a, b);
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createFCmp(IRInstrKind pred, IRValue* a, IRValue* b, std::string nm) {
    return createICmp(pred, a, b, nm);
}

// ── Memory ────────────────────────────────────────────────────────────────────

IRInstr* IRBuilder::createAlloca(IRType* ty, IRValue* /*count*/, std::string nm) {
    auto i = std::make_unique<IRAllocaInstr>(nm.empty() ? "tmp" : nm);
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createLoad(IRType* /*ty*/, IRValue* ptr, std::string nm) {
    auto i = std::make_unique<IRLoadInstr>(ptr);
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createStore(IRValue* val, IRValue* ptr) {
    auto i = std::make_unique<IRStoreInstr>(val, ptr);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createGEP(IRType*, IRValue* ptr,
                               std::vector<IRValue*> indices, std::string nm) {
    std::string field = indices.empty() ? "0" : indices[0]->debugName();
    auto i = std::make_unique<IRGepInstr>(ptr, field);
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

// ── Control flow ──────────────────────────────────────────────────────────────

IRInstr* IRBuilder::createBr(IRBlock* dest) {
    auto i = std::make_unique<IRBrInstr>(dest);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createCondBr(IRValue* cond, IRBlock* thenB, IRBlock* elseB) {
    auto i = std::make_unique<IRCondBrInstr>(cond, thenB, elseB);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createRet(IRValue* val) {
    auto i = std::make_unique<IRReturnInstr>(val);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createRetVoid() {
    auto i = std::make_unique<IRReturnInstr>(nullptr);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createUnreachable() {
    auto i = std::make_unique<IRUnreachableInstr>();
    return emit(std::move(i));
}

// ── Calls ─────────────────────────────────────────────────────────────────────

IRInstr* IRBuilder::createCall(IRFunction* fn, std::vector<IRValue*> args, std::string nm) {
    auto i = std::make_unique<IRCallInstr>(fn->name(), std::move(args));
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createCall(IRValue* fnPtr, IRType*, std::vector<IRValue*> args, std::string nm) {
    auto i = std::make_unique<IRCallInstr>(fnPtr, std::move(args));
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createTailCall(IRFunction* fn, std::vector<IRValue*> args) {
    return createCall(fn, std::move(args));
}

// ── Conversions ───────────────────────────────────────────────────────────────

IRInstr* IRBuilder::createZExt(IRValue* v, IRType*, std::string nm) {
    auto i = std::make_unique<IRCastInstr>(v, nullptr);
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createSExt(IRValue* v, IRType*, std::string nm) {
    return createZExt(v, nullptr, nm);
}

IRInstr* IRBuilder::createTrunc(IRValue* v, IRType*, std::string nm) {
    return createZExt(v, nullptr, nm);
}

IRInstr* IRBuilder::createBitcast(IRValue* v, IRType*, std::string nm) {
    return createZExt(v, nullptr, nm);
}

IRInstr* IRBuilder::createPhi(IRType*, std::string nm) {
    auto i = std::make_unique<IRPhiInstr>(std::vector<IRPhiInstr::IncomingPair>{});
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

// ── Aggregates ────────────────────────────────────────────────────────────────

IRInstr* IRBuilder::createExtractValue(IRValue* agg, unsigned idx, std::string nm) {
    auto i = std::make_unique<IRIndexInstr>(agg,
        new IRIntConstInstr(idx, IntSuffix::None, 10));
    if (!nm.empty()) i->setName(nm);
    return emit(std::move(i));
}

IRInstr* IRBuilder::createInsertValue(IRValue* /*agg*/, IRValue* val, unsigned, std::string nm) {
    // Simplified: just return val
    (void)nm;
    return static_cast<IRInstr*>(val);
}

// ── Constants ─────────────────────────────────────────────────────────────────

IRValue* IRBuilder::getInt(int64_t v, uint32_t bits, bool /*isSigned*/) {
    return emit(std::make_unique<IRIntConstInstr>(
        static_cast<uint64_t>(v), IntSuffix::None, 10));
}

IRValue* IRBuilder::getUInt(uint64_t v, uint32_t bits) {
    return emit(std::make_unique<IRIntConstInstr>(v, IntSuffix::None, 10));
}

IRValue* IRBuilder::getFloat(double v, uint32_t bits) {
    FloatSuffix s = (bits == 32) ? FloatSuffix::f : FloatSuffix::None;
    return emit(std::make_unique<IRFloatConstInstr>(v, s));
}

IRValue* IRBuilder::getBool(bool v) {
    return emit(std::make_unique<IRBoolConstInstr>(v));
}

IRValue* IRBuilder::getNull(IRType*) {
    return emit(std::make_unique<IRConstInstr>(IRConstKind::Null));
}

IRValue* IRBuilder::getUndef(IRType*) {
    return emit(std::make_unique<IRConstInstr>(IRConstKind::Undef));
}

} // namespace vex
