#include "vex/Sema/ExhaustivenessChecker.h"
namespace vex {
bool ExhaustivenessChecker::run(ModuleDecl*) { return true; }
bool ExhaustivenessChecker::checkMatch(MatchExpr*) { return true; }
bool ExhaustivenessChecker::checkMatchStmt(MatchStmt*) { return true; }
bool ExhaustivenessChecker::isExhaustive(TypeNode*, const std::vector<PatternNode*>&) { return true; }
bool ExhaustivenessChecker::coversEnum(EnumDecl*, const std::vector<PatternNode*>&) { return true; }
bool ExhaustivenessChecker::coversBool(const std::vector<PatternNode*>&) { return true; }
bool ExhaustivenessChecker::coversInt(TypeNode*, const std::vector<PatternNode*>&) { return true; }
bool ExhaustivenessChecker::hasWildcard(const std::vector<PatternNode*>& pats) {
    (void)pats; return false;
}
void ExhaustivenessChecker::reportMissingVariants(MatchExpr*, const std::vector<std::string>&) {}
}
