// src/IR/VexIR.cpp
// ============================================================================
// vex/IR/VexIR.cpp
// Core VexIR data structures.
// ============================================================================

#include "vex/IR/VexIR.h"
#include "vex/Core/Assert.h"
#include <sstream>
#include <algorithm>

namespace vex {

// ── IRValue ───────────────────────────────────────────────────────────────────

IRValue::IRValue(IRValueKind kind, std::string_view name)
    : kind_(kind), name_(name), id_(nextId_++)
{}

uint64_t IRValue::nextId_ = 0;

std::string IRValue::debugName() const {
    if (!name_.empty()) return "%" + name_;
    return "%" + std::to_string(id_);
}

// ── IRBlock ───────────────────────────────────────────────────────────────────

IRBlock::IRBlock(std::string_view label, IRFunction* parent)
    : label_(label), parent_(parent)
{}

void IRBlock::appendInstr(std::unique_ptr<IRInstr> instr) {
    instr->setParent(this);
    instrs_.push_back(std::move(instr));
}

bool IRBlock::hasTerminator() const {
    if (instrs_.empty()) return false;
    auto kind = instrs_.back()->kind();
    return kind == IRInstrKind::Return  ||
           kind == IRInstrKind::Br      ||
           kind == IRInstrKind::CondBr  ||
           kind == IRInstrKind::Unreachable;
}

void IRBlock::dump(std::ostream& os) const {
    os << label_ << ":\n";
    for (auto& instr : instrs_)
        instr->dump(os);
}

// ── IRFunction ───────────────────────────────────────────────────────────────

IRFunction::IRFunction(std::string_view name, IRModule* parent)
    : name_(name), parent_(parent)
{}

IRBlock* IRFunction::createBlock(std::string_view label) {
    auto block = std::make_unique<IRBlock>(label, this);
    auto* ptr  = block.get();
    blocks_.push_back(std::move(block));
    return ptr;
}

IRValue* IRFunction::addParam(std::string_view name) {
    auto param = std::make_unique<IRParam>(name);
    auto* ptr  = param.get();
    params_.push_back(std::move(param));
    return ptr;
}

IRBlock* IRFunction::entryBlock() const {
    VEX_ASSERT(!blocks_.empty(), "IRFunction has no blocks");
    return blocks_.front().get();
}

void IRFunction::dump(std::ostream& os) const {
    os << "fn " << name_ << "(";
    for (size_t i = 0; i < params_.size(); ++i) {
        if (i) os << ", ";
        os << params_[i]->debugName();
    }
    os << ") {\n";
    for (auto& block : blocks_)
        block->dump(os);
    os << "}\n\n";
}

// ── IRGlobal ──────────────────────────────────────────────────────────────────

IRGlobal::IRGlobal(std::string_view name, bool mutable_)
    : IRValue(IRValueKind::Global, name), mutable_(mutable_)
{}

// ── IRModule ──────────────────────────────────────────────────────────────────

IRModule::IRModule(std::string_view name) : name_(name) {}

IRFunction* IRModule::createFunction(std::string_view name) {
    auto fn  = std::make_unique<IRFunction>(name, this);
    auto* ptr = fn.get();
    functions_.push_back(std::move(fn));
    fnMap_[std::string(name)] = ptr;
    return ptr;
}

IRGlobal* IRModule::createGlobal(std::string_view name, bool mutable_) {
    auto g   = std::make_unique<IRGlobal>(name, mutable_);
    auto* ptr = g.get();
    globals_.push_back(std::move(g));
    return ptr;
}

void IRModule::declareType(std::string_view name) {
    typeNames_.emplace_back(name);
}

IRFunction* IRModule::findFunction(std::string_view name) const {
    auto it = fnMap_.find(std::string(name));
    return (it != fnMap_.end()) ? it->second : nullptr;
}

void IRModule::dump(std::ostream& os) const {
    os << "; VexIR Module: " << name_ << "\n\n";
    for (auto& g : globals_)
        os << "global " << g->name() << "\n";
    if (!globals_.empty()) os << "\n";
    for (auto& fn : functions_)
        fn->dump(os);
}

// ── Instruction implementations ───────────────────────────────────────────────

void IRInstr::setParent(IRBlock* b) { parent_ = b; }

// IRReturnInstr
void IRReturnInstr::dump(std::ostream& os) const {
    os << "  ret ";
    if (value_) os << value_->debugName();
    else        os << "void";
    os << "\n";
}

// IRBrInstr
void IRBrInstr::dump(std::ostream& os) const {
    os << "  br " << target_->label() << "\n";
}

// IRCondBrInstr
void IRCondBrInstr::dump(std::ostream& os) const {
    os << "  condbr " << cond_->debugName()
       << ", " << thenBB_->label()
       << ", " << elseBB_->label() << "\n";
}

// IRAllocaInstr
void IRAllocaInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = alloca " << name_ << "\n";
}

// IRStoreInstr
void IRStoreInstr::dump(std::ostream& os) const {
    os << "  store " << val_->debugName()
       << ", " << ptr_->debugName() << "\n";
}

// IRLoadInstr
void IRLoadInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = load " << ptr_->debugName() << "\n";
}

// IRCallInstr (callee as string)
IRCallInstr::IRCallInstr(std::string_view callee,
                         std::vector<IRValue*> args)
    : IRInstr(IRInstrKind::Call), calleeName_(callee), args_(std::move(args))
{}

// IRCallInstr (callee as value)
IRCallInstr::IRCallInstr(IRValue* callee,
                         std::vector<IRValue*> args)
    : IRInstr(IRInstrKind::Call), calleeVal_(callee), args_(std::move(args))
{}

void IRCallInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = call ";
    if (calleeVal_)  os << calleeVal_->debugName();
    else             os << calleeName_;
    os << "(";
    for (size_t i = 0; i < args_.size(); ++i) {
        if (i) os << ", ";
        os << args_[i]->debugName();
    }
    os << ")\n";
}

// IRBinOpInstr
void IRBinOpInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = binop "
       << (int)op_ << " "
       << lhs_->debugName() << ", "
       << rhs_->debugName() << "\n";
}

// IRUnaryOpInstr
void IRUnaryOpInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = unary "
       << (int)op_ << " " << operand_->debugName();
    if (postfix_) os << " postfix";
    os << "\n";
}

// IRCmpInstr
void IRCmpInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = cmp "
       << (int)op_ << " "
       << lhs_->debugName() << ", "
       << rhs_->debugName() << "\n";
}

// IRGepInstr
void IRGepInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = gep "
       << base_->debugName() << "." << field_ << "\n";
}

// IRIndexInstr
void IRIndexInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = index "
       << base_->debugName() << "[" << idx_->debugName() << "]\n";
}

// IRPhiInstr
void IRPhiInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = phi [";
    for (size_t i = 0; i < incoming_.size(); ++i) {
        if (i) os << ", ";
        os << incoming_[i].first->debugName()
           << " <- " << incoming_[i].second->label();
    }
    os << "]\n";
}

// IRIntConstInstr
void IRIntConstInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = int " << value_ << "\n";
}

// IRFloatConstInstr
void IRFloatConstInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = float " << value_ << "\n";
}

// IRBoolConstInstr
void IRBoolConstInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = bool " << (value_ ? "true" : "false") << "\n";
}

// IRStringConstInstr
void IRStringConstInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = str \"" << value_ << "\"\n";
}

// IRConstInstr
void IRConstInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = const ";
    switch (kind_) {
        case IRConstKind::Null: os << "null"; break;
        case IRConstKind::Void: os << "void"; break;
        case IRConstKind::Undef: os << "undef"; break;
    }
    os << "\n";
}

// IRCastInstr
void IRCastInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = cast " << val_->debugName() << "\n";
}

// IRRangeInstr
void IRRangeInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = range "
       << lo_->debugName()
       << (inclusive_ ? "..=" : "..")
       << hi_->debugName() << "\n";
}

// IRNullCoalesceInstr
void IRNullCoalesceInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = nullcoalesce "
       << lhs_->debugName() << " ?? " << rhs_->debugName() << "\n";
}

// IRTupleInstr
void IRTupleInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = tuple (";
    for (size_t i = 0; i < elems_.size(); ++i) {
        if (i) os << ", ";
        os << elems_[i]->debugName();
    }
    os << ")\n";
}

// IRListInstr
void IRListInstr::dump(std::ostream& os) const {
    os << "  " << debugName() << " = list [";
    for (size_t i = 0; i < elems_.size(); ++i) {
        if (i) os << ", ";
        os << elems_[i]->debugName();
    }
    os << "]\n";
}

} // namespace vex