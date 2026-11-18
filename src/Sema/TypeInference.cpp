#include "vex/Sema/TypeInference.h"
namespace vex {
bool      TypeInference::inferFunction(FnDecl*)        { return true; }
TypeNode* TypeInference::infer(ExprNode*)               { return nullptr; }
bool      TypeInference::unify(TypeNode*, TypeNode*, SourceLocation) { return true; }
TypeNode* TypeInference::freshVar()                     { return nullptr; }
TypeNode* TypeInference::substitute(TypeNode* t)        { return t; }
TypeNode* TypeInference::inferLiteral(IntLitExpr*)      { return nullptr; }
TypeNode* TypeInference::inferBinary(BinaryExpr*)       { return nullptr; }
TypeNode* TypeInference::inferUnary(UnaryExpr*)         { return nullptr; }
TypeNode* TypeInference::inferCall(CallExpr*)           { return nullptr; }
TypeNode* TypeInference::inferMember(MemberExpr*)       { return nullptr; }
TypeNode* TypeInference::inferBlock(BlockExpr*)         { return nullptr; }
} // namespace vex
