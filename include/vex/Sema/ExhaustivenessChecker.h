#pragma once
// include/vex/Sema/ExhaustivenessChecker.h


// ============================================================================
// vex/Sema/ExhaustivenessChecker.h
// Ensures match/switch expressions cover all possible patterns.
// ============================================================================
#include "vex/Sema/SemaContext.h"
#include "vex/AST/Stmt.h"
#include "vex/AST/Expr.h"
#include <vector>

namespace vex {

class ExhaustivenessChecker {
public:
    explicit ExhaustivenessChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    // Check a single match expression
    bool checkMatch(MatchExpr* m);
    bool checkMatchStmt(MatchStmt* m);

    // Check if a pattern list is exhaustive for the given type
    bool isExhaustive(TypeNode* ty, const std::vector<PatternNode*>& patterns);

private:
    SemaContext& ctx_;

    bool coversEnum(EnumDecl* e, const std::vector<PatternNode*>& patterns);
    bool coversBool(const std::vector<PatternNode*>& patterns);
    bool coversInt(TypeNode* ty, const std::vector<PatternNode*>& patterns);
    bool hasWildcard(const std::vector<PatternNode*>& patterns);
    void reportMissingVariants(MatchExpr* m, const std::vector<std::string>& missing);
};

} // namespace vex
