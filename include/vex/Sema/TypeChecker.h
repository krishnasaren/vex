#pragma once
// include/vex/Sema/TypeChecker.h


// ============================================================================
// vex/Sema/TypeChecker.h
// Type checking pass: infers and verifies types on every expression and decl.
// ============================================================================

#include "vex/Sema/SemaContext.h"
#include "vex/AST/ASTVisitor.h"

namespace vex {

class TypeChecker : public ASTVisitor {
public:
    explicit TypeChecker(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    void visitFnDecl(FnDecl* n)           override;
    void visitStructDecl(StructDecl* n)   override;
    void visitClassDecl(ClassDecl* n)     override;
    void visitEnumDecl(EnumDecl* n)       override;
    void visitImplDecl(ImplDecl* n)       override;
    void visitVarDeclStmt(VarDeclStmt* n) override;
    void visitReturnStmt(ReturnStmt* n)   override;
    void visitIfStmt(IfStmt* n)           override;
    void visitWhileStmt(WhileStmt* n)     override;
    void visitForStmt(ForStmt* n)         override;

    // Expression type inference
    TypeNode* inferType(ExprNode* expr);
    TypeNode* inferBinaryType(BinaryExpr* e);
    TypeNode* inferCallType(CallExpr* e);
    TypeNode* inferMemberType(MemberExpr* e);

    // Type compatibility check
    bool isAssignable(TypeNode* from, TypeNode* to);
    bool isCompatible(TypeNode* a, TypeNode* b);

private:
    SemaContext& ctx_;

    TypeNode* makeNamedType(std::string_view name);
    TypeNode* makeNullableType(TypeNode* inner);
    void      checkFunctionBody(FnDecl* fn);
};

} // namespace vex
