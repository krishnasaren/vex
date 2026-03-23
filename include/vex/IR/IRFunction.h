#pragma once
// include/vex/IR/IRFunction.h


// ============================================================================
// vex/IR/IRFunction.h  — A function in VEX IR
// ============================================================================
#include "vex/IR/IRValue.h"
#include "vex/IR/IRBlock.h"
#include <vector>
#include <memory>
#include <string>

namespace vex {

class IRModule;

struct IRArgument : public IRValue {
    uint32_t index;
    IRArgument(IRType* ty, uint32_t idx)
        : IRValue(IRValueKind::Argument, ty), index(idx) {}
};

class IRFunction : public IRValue {
public:
    IRFunction(std::string name, IRType* fnType, IRModule* parent)
        : IRValue(IRValueKind::Function, fnType), parent_(parent) {
        setName(std::move(name));
    }

    IRModule*  module()  const { return parent_; }
    IRType*    fnType()  const { return type(); }
    IRType*    retType() const;

    const std::vector<std::unique_ptr<IRArgument>>& args()   const { return args_; }
    const std::vector<std::unique_ptr<IRBlock>>&    blocks() const { return blocks_; }

    IRArgument* arg(size_t i) const { return args_[i].get(); }
    IRBlock*    entry()       const { return blocks_.empty() ? nullptr : blocks_[0].get(); }

    IRBlock* addBlock(std::string label = "");
    void     addArg(IRType* ty);

    bool isDeclaration() const { return blocks_.empty(); }
    bool isExternal()    const { return external_; }
    void setExternal(bool v)   { external_ = v; }

    const std::string& linkage() const { return linkage_; }
    void setLinkage(std::string l)     { linkage_ = std::move(l); }

    bool isTailCallOptimizable() const { return tailCallOpt_; }
    void setTailCallOpt(bool v)        { tailCallOpt_ = v; }

    void dump() const;

private:
    IRModule*                              parent_;
    std::vector<std::unique_ptr<IRArgument>> args_;
    std::vector<std::unique_ptr<IRBlock>>    blocks_;
    bool                                   external_    = false;
    bool                                   tailCallOpt_ = false;
    std::string                            linkage_     = "external";
};

} // namespace vex
