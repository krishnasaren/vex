#pragma once
// include/vex/Module/DependencyGraph.h


// ============================================================================
// vex/Module/DependencyGraph.h  — DAG of module dependencies
// ============================================================================
#include "vex/Core/ADT/DAG.h"
#include "vex/AST/Decl.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace vex {

class DependencyGraph {
public:
    using NodeId = uint32_t;

    NodeId addModule(std::string_view path, ModuleDecl* mod);
    void   addEdge(NodeId from, NodeId to);

    NodeId nodeFor(std::string_view path) const;
    ModuleDecl* moduleFor(NodeId id) const;

    // Topological order (leaves first); returns false on cycle
    bool topoSort(std::vector<NodeId>& order);

    // Detect cycles and return one cycle path
    bool hasCycle(std::vector<std::string>& cyclePath);

private:
    DAG<NodeId>                            dag_;
    std::unordered_map<std::string,NodeId> pathToId_;
    std::unordered_map<NodeId,std::string> idToPath_;
    std::unordered_map<NodeId,ModuleDecl*> idToMod_;
    NodeId nextId_ = 0;
};
} // namespace vex
