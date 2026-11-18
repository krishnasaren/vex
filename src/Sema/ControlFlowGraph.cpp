#include "vex/Sema/ControlFlowGraph.h"
namespace vex {
std::unique_ptr<ControlFlowGraph> ControlFlowGraph::build(FnDecl*) {
    return std::make_unique<ControlFlowGraph>();
}
void ControlFlowGraph::dfs(std::function<void(CFGNode*)>) const {}
void ControlFlowGraph::computeDominators() {}
bool ControlFlowGraph::dominates(uint32_t, uint32_t) const { return false; }
void ControlFlowGraph::buildFrom(StmtNode*, CFGNode*, CFGNode*, CFGNode*) {}
}
