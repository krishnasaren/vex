#pragma once
// include/vex/Sema/TypeUnifier.h



// ============================================================================
// vex/Sema/TypeUnifier.h  — Low-level type unification / equality test
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Type.h"

namespace vex {

class TypeUnifier {
public:
    explicit TypeUnifier(SemaContext& ctx) : ctx_(ctx) {}

    // Returns true if a and b are the same structural type
    bool equal(TypeNode* a, TypeNode* b);

    // Returns true if 'from' is implicitly convertible to 'to'
    bool convertible(TypeNode* from, TypeNode* to);

    // Returns the common supertype of a and b (or nullptr)
    TypeNode* join(TypeNode* a, TypeNode* b, SourceLocation loc);

    // True if type is a numeric primitive
    bool isNumeric(TypeNode* t);
    bool isIntegral(TypeNode* t);
    bool isFloat(TypeNode* t);
    bool isString(TypeNode* t);
    bool isBool(TypeNode* t);
    bool isUnit(TypeNode* t);
    bool isNullable(TypeNode* t);
    bool isPointer(TypeNode* t);
    bool isArray(TypeNode* t);
    bool isSlice(TypeNode* t);
    bool isTuple(TypeNode* t);
    bool isFn(TypeNode* t);
    bool isGeneric(TypeNode* t);
    bool isErrorUnion(TypeNode* t);

private:
    SemaContext& ctx_;
    bool equalImpl(TypeNode* a, TypeNode* b, int depth);
};

} // namespace vex
