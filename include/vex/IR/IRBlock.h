#pragma once
// include/vex/IR/IRBlock.h


// ============================================================================
// vex/IR/IRBlock.h  — A basic block in the VEX IR CFG
// ============================================================================
#include "vex/IR/IRValue.h"
#include "vex/IR/IRInstr.h"
#include <vector>
#include <memory>
#include <string>

namespace vex {

class IRFunction;

class IRBlock : public IRValue {
public:
    explicit IRBlock(IRFunction* parent, std::string label = "")
        : IRValue(IRValueKind::Block, IRType::getVoid()), parent_(parent) {
        setName(std::move(label));
    }

    IRFunction*                               parent()   const { return parent_; }
    const std::vector<std::unique_ptr<IRInstr>>& instrs() const { return instrs_; }

    IRInstr* append(std::unique_ptr<IRInstr> i);
    IRInstr* insert(size_t pos, std::unique_ptr<IRInstr> i);
    void     remove(IRInstr* i);

    IRInstr* terminator() const;
    bool     hasTerminator() const;

    const std::vector<IRBlock*>& predecessors() const { return preds_; }
    const std::vector<IRBlock*>& successors()   const { return succs_; }
    void addPredecessor(IRBlock* b) { preds_.push_back(b); }
    void addSuccessor(IRBlock* b)   { succs_.push_back(b); }

    // Phi nodes for SSA
    std::vector<IRInstr*> phis() const;

    void dump() const;

private:
    IRFunction*                             parent_;
    std::vector<std::unique_ptr<IRInstr>>   instrs_;
    std::vector<IRBlock*>                   preds_;
    std::vector<IRBlock*>                   succs_;
};

} // namespace vex
