#pragma once
// include/vex/Sema/LivenessAnalysis.h


// ============================================================================
// vex/Sema/LivenessAnalysis.h  — Data-flow liveness for variables
// ============================================================================
#include "vex/Sema/ControlFlowGraph.h"
#include <unordered_set>
#include <string>

namespace vex {

struct LivenessInfo {
    std::unordered_set<std::string> liveIn;
    std::unordered_set<std::string> liveOut;
    std::unordered_set<std::string> def;
    std::unordered_set<std::string> use;
};

class LivenessAnalysis {
public:
    // Run backward data-flow liveness on the CFG
    static void run(ControlFlowGraph& cfg,
                    std::vector<LivenessInfo>& out);

private:
    static void computeDefUse(CFGNode* node, LivenessInfo& info);
};

} // namespace vex
