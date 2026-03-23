#pragma once
// include/vex/IR/IRDumper.h

// ============================================================================
// vex/IR/IRDumper.h  — Textual dump of VEX IR for debugging
// ============================================================================
#include "vex/IR/IRModule.h"
#include <ostream>

namespace vex {

class IRDumper {
public:
    explicit IRDumper(std::ostream& out, bool color = false)
        : out_(out), color_(color) {}

    void dump(const IRModule& mod);
    void dump(const IRFunction& fn);
    void dump(const IRBlock& blk);
    void dump(const IRInstr& instr);
    void dump(const IRValue& val);
    void dump(const IRType& ty);

private:
    std::ostream& out_;
    bool          color_;
    std::unordered_map<const IRValue*, std::string> names_;
    uint32_t      nextId_ = 0;

    std::string nameOf(const IRValue* v);
    std::string typeStr(const IRType* t);
};

} // namespace vex
