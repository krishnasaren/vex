#pragma once
// include/vex/Sema/ControlFlowGraph.h


// ============================================================================
// vex/Sema/ControlFlowGraph.h  — CFG construction for a function body.
// Used by borrow checker, liveness analysis, and def-use analysis.
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Stmt.h"
#include <vector>
#include <memory>

namespace vex {

struct CFGNode {
    uint32_t              id;
    std::vector<StmtNode*> stmts;
    std::vector<uint32_t>  successors;
    std::vector<uint32_t>  predecessors;
    bool                   isEntry = false;
    bool                   isExit  = false;
};

class ControlFlowGraph {
public:
    // Build CFG for function body
    static std::unique_ptr<ControlFlowGraph> build(FnDecl* fn);

    const std::vector<std::unique_ptr<CFGNode>>& nodes() const { return nodes_; }
    CFGNode* entry() const { return entry_; }
    CFGNode* exit()  const { return exit_; }
    CFGNode* node(uint32_t id) const { return nodes_[id].get(); }

    // DFS traversal
    void dfs(std::function<void(CFGNode*)> fn) const;

    // Compute dominators
    void computeDominators();
    bool dominates(uint32_t a, uint32_t b) const;

private:
    std::vector<std::unique_ptr<CFGNode>> nodes_;
    CFGNode* entry_ = nullptr;
    CFGNode* exit_  = nullptr;
    std::vector<uint32_t> idom_; // immediate dominator for each node

    CFGNode* newNode();
    void     buildFrom(StmtNode* s, CFGNode* current, CFGNode* breakTarget,
                       CFGNode* continueTarget);
};

} // namespace vex
