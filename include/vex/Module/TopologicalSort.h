#pragma once
// include/vex/Module/TopologicalSort.h



// vex/Module/TopologicalSort.h  — Kahn's algorithm for module compile order
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace vex {

class TopologicalSort {
public:
    void addNode(std::string_view node);
    void addEdge(std::string_view from, std::string_view to); // from depends on to

    // Returns sorted list (dependencies first); false = cycle
    bool sort(std::vector<std::string>& out);
    bool hasCycle() const { return cycle_; }

private:
    std::unordered_map<std::string,std::vector<std::string>> edges_; // node → deps
    std::unordered_map<std::string,int> inDegree_;
    bool cycle_ = false;
};
} // namespace vex
