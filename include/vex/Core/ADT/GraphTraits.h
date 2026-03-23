#pragma once
// include/vex/Core/ADT/GraphTraits.h


// ============================================================================
// vex/Core/ADT/GraphTraits.h  — LLVM-style graph traits for algorithms
// ============================================================================
namespace vex {

template<typename GraphT>
struct GraphTraits {
    // Specialize for concrete graph types:
    // using NodeRef = ...;
    // static NodeRef getEntryNode(GraphT* g);
    // static auto    child_begin(NodeRef n);
    // static auto    child_end(NodeRef n);
};

} // namespace vex
