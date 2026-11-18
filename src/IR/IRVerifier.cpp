#include "vex/IR/IRVerifier.h"
namespace vex {
bool IRVerifier::verify(const IRModule& mod) {
    for (auto& fn : mod.functions())
        if (!verifyFunction(*fn)) return false;
    return ok_;
}
bool IRVerifier::verifyFunction(const IRFunction& fn) {
    for (auto& blk : fn.blocks())
        if (!verifyBlock(*blk)) return false;
    return ok_;
}
bool IRVerifier::verifyBlock(const IRBlock& blk) {
    for (auto& instr : blk.instrs())
        if (!verifyInstr(*instr)) return false;
    return ok_;
}
bool IRVerifier::verifyInstr(const IRInstr&) { return ok_; }
void IRVerifier::error(std::string_view msg, const IRInstr*) {
    fprintf(stderr, "IR verification error: %.*s\n", (int)msg.size(), msg.data());
    ok_ = false;
}
bool IRVerifier::checkTerminator(const IRBlock& blk) { return blk.hasTerminator(); }
bool IRVerifier::checkPhiPredecessors(const IRInstr&, const IRBlock&) { return true; }
bool IRVerifier::checkOperandTypes(const IRInstr&) { return true; }
}
