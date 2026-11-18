#include "vex/Sema/LivenessAnalysis.h"
namespace vex {
void LivenessAnalysis::run(ControlFlowGraph&, std::vector<LivenessInfo>&) {}
void LivenessAnalysis::computeDefUse(CFGNode*, LivenessInfo&) {}
}
