#pragma once
// include/vex/Sema/TypeConstraint.h


// ============================================================================
// vex/Sema/TypeConstraint.h  — Constraint set for generic type parameter solving
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Type.h"
#include <vector>

namespace vex {

enum class ConstraintKind : uint8_t {
    Equal,       // T == U
    Subtype,     // T <: U
    Implements,  // T : Trait
    HasMember,   // T has field/method .name
};

struct Constraint {
    ConstraintKind kind;
    TypeNode*      lhs;
    TypeNode*      rhs;      // or trait node
    std::string    member;   // for HasMember
    SourceLocation loc;
};

class TypeConstraintSolver {
public:
    explicit TypeConstraintSolver(SemaContext& ctx) : ctx_(ctx) {}

    void addEqual(TypeNode* a, TypeNode* b, SourceLocation loc);
    void addSubtype(TypeNode* sub, TypeNode* super, SourceLocation loc);
    void addImplements(TypeNode* t, TypeNode* trait, SourceLocation loc);
    void addHasMember(TypeNode* t, std::string_view member, SourceLocation loc);

    // Solve all constraints; returns false if unsatisfiable
    bool solve();

    const std::vector<Constraint>& constraints() const { return constraints_; }

private:
    SemaContext&            ctx_;
    std::vector<Constraint> constraints_;

    bool solveEqual(const Constraint& c);
    bool solveImplements(const Constraint& c);
};

} // namespace vex
