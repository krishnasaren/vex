#pragma once
// include/vex/Sema/MROResolver.h

// ============================================================================
// vex/Sema/MROResolver.h  — Method Resolution Order for class inheritance
// Implements C3 linearization for multi-class hierarchies.
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Decl.h"
#include <vector>

namespace vex {

class MROResolver {
public:
    explicit MROResolver(SemaContext& ctx) : ctx_(ctx) {}

    // Compute MRO for a class; returns false on cycle
    bool computeMRO(ClassDecl* cls);

    // Get previously computed MRO (most-derived first)
    const std::vector<ClassDecl*>& getMRO(ClassDecl* cls);

    // Lookup first method in MRO order
    FnDecl* lookupMethod(ClassDecl* cls, std::string_view name);

    // Lookup field in MRO order
    FieldDecl* lookupField(ClassDecl* cls, std::string_view name);

private:
    SemaContext& ctx_;
    std::unordered_map<ClassDecl*, std::vector<ClassDecl*>> cache_;

    bool c3Merge(std::vector<std::vector<ClassDecl*>>& lists,
                 std::vector<ClassDecl*>& result);
};

} // namespace vex
