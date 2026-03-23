#pragma once
// include/vex/Core/ADT/DAG.h


// ============================================================================
// vex/Core/ADT/DAG.h  — Generic Directed Acyclic Graph
// ============================================================================
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace vex {

template<typename NodeId = uint32_t>
class DAG {
public:
    void   addNode(NodeId n)            { nodes_.insert(n); }
    bool   hasNode(NodeId n)    const   { return nodes_.count(n); }
    void   addEdge(NodeId from, NodeId to) {
        addNode(from); addNode(to);
        edges_[from].push_back(to);
        revEdges_[to].push_back(from);
    }
    const std::vector<NodeId>& successors(NodeId n)   const {
        static const std::vector<NodeId> empty;
        auto it = edges_.find(n); return it != edges_.end() ? it->second : empty;
    }
    const std::vector<NodeId>& predecessors(NodeId n) const {
        static const std::vector<NodeId> empty;
        auto it = revEdges_.find(n); return it != revEdges_.end() ? it->second : empty;
    }

    // Kahn's algorithm topological sort; returns false on cycle
    bool topoSort(std::vector<NodeId>& out) const;

    // DFS post-order
    void dfsPostOrder(NodeId root, std::function<void(NodeId)> fn) const;

private:
    std::unordered_set<NodeId>                    nodes_;
    std::unordered_map<NodeId,std::vector<NodeId>> edges_;
    std::unordered_map<NodeId,std::vector<NodeId>> revEdges_;
};

} // namespace vex
