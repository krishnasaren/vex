#pragma once
// include/vex/IR/IRInstr.h

// ============================================================================
// vex/IR/IRInstr.h  — A single VEX IR instruction
// ============================================================================
#include "vex/IR/IRValue.h"
#include "vex/IR/IRInstrKind.h"
#include "vex/Core/SourceLocation.h"
#include <vector>

namespace vex {

class IRBlock;

class IRInstr : public IRValue {
public:
    IRInstr(IRInstrKind opcode, IRType* type, IRBlock* parent = nullptr)
        : IRValue(IRValueKind::Instruction, type), opcode_(opcode), parent_(parent) {}

    IRInstrKind             opcode()    const { return opcode_; }
    IRBlock*                parent()    const { return parent_; }
    SourceLocation          location()  const { return loc_; }
    const std::vector<IRValue*>& operands() const { return operands_; }
    IRValue*                operand(size_t i) const { return operands_[i]; }
    size_t                  numOperands() const { return operands_.size(); }

    void setParent(IRBlock* b)         { parent_ = b; }
    void setLocation(SourceLocation l) { loc_    = l; }
    void addOperand(IRValue* v)        { operands_.push_back(v); v->addUse(this); }
    void setOperand(size_t i, IRValue* v) { operands_[i] = v; }

    bool isTerminator() const;
    bool isMemoryOp()   const;
    bool isCallInstr()  const {
        return opcode_ == IRInstrKind::Call ||
               opcode_ == IRInstrKind::TailCall ||
               opcode_ == IRInstrKind::IndirectCall;
    }

    void dump() const;

private:
    IRInstrKind          opcode_;
    IRBlock*             parent_ = nullptr;
    SourceLocation       loc_;
    std::vector<IRValue*> operands_;
};

} // namespace vex
