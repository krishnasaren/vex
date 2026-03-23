#pragma once
// include/vex/IR/IRVerifier.h


// ============================================================================
// vex/IR/IRVerifier.h  — Checks IR invariants after every pass
// ============================================================================
#include "vex/IR/IRModule.h"
#include "vex/Core/DiagnosticEngine.h"

namespace vex {

class IRVerifier {
public:
    explicit IRVerifier(DiagnosticEngine& diags) : diags_(diags) {}

    // Returns true if module is well-formed
    bool verify(const IRModule& mod);
    bool verifyFunction(const IRFunction& fn);
    bool verifyBlock(const IRBlock& blk);
    bool verifyInstr(const IRInstr& instr);

private:
    DiagnosticEngine& diags_;
    bool ok_ = true;

    void error(std::string_view msg, const IRInstr* instr = nullptr);
    bool checkTerminator(const IRBlock& blk);
    bool checkPhiPredecessors(const IRInstr& phi, const IRBlock& blk);
    bool checkOperandTypes(const IRInstr& instr);
};

} // namespace vex
