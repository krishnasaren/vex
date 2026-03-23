#pragma once
// include/vex/IR/IRValue.h


// ============================================================================
// vex/IR/IRValue.h  — Base class for all IR values (instr, arg, const, block)
// ============================================================================
#include "vex/IR/IRType.h"
#include <string>
#include <vector>
#include <cstdint>

namespace vex {

class IRInstr;
class IRBlock;

enum class IRValueKind : uint8_t {
    Instruction,
    Argument,
    Constant,
    GlobalVariable,
    Function,
    Block,
    Undef,
};

class IRValue {
public:
    IRValue(IRValueKind kind, IRType* type)
        : kind_(kind), type_(type) {}
    virtual ~IRValue() = default;

    IRValueKind kind() const { return kind_; }
    IRType*     type() const { return type_; }

    const std::string& name() const { return name_; }
    void setName(std::string n)     { name_ = std::move(n); }

    // Use-def chain
    void addUse(IRInstr* user) { uses_.push_back(user); }
    void removeUse(IRInstr* user);
    const std::vector<IRInstr*>& uses() const { return uses_; }
    uint32_t useCount() const { return (uint32_t)uses_.size(); }

    bool isConstant() const { return kind_ == IRValueKind::Constant; }
    bool isUndef()    const { return kind_ == IRValueKind::Undef; }

private:
    IRValueKind         kind_;
    IRType*             type_;
    std::string         name_;
    std::vector<IRInstr*> uses_;
};

} // namespace vex
