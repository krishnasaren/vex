#pragma once
// ============================================================================
// vex/AST/ASTDumper.h
// Prints the AST in a human-readable tree format to stdout.
// Used by: vexc --dump-ast
// ============================================================================

#include "vex/AST/ASTVisitor.h"
#include "vex/Core/SourceManager.h"
#include <cstdio>
#include <string>

namespace vex {

class ASTDumper : public ASTVisitor {
public:
    explicit ASTDumper(const SourceManager& srcMgr,
                       FILE* out = stdout,
                       bool  useColor = true)
        : srcMgr_(srcMgr), out_(out), useColor_(useColor) {}

    // Dump a module (top-level entry point)
    void dump(ModuleDecl* module);

    // Override all visit methods
    void visitModuleDecl(ModuleDecl* node) override;
    void visitImportDecl(ImportDecl* node) override;
    void visitFnDecl(FnDecl* node) override;
    void visitStructDecl(StructDecl* node) override;
    void visitClassDecl(ClassDecl* node) override;
    void visitEnumDecl(EnumDecl* node) override;
    void visitTraitDecl(TraitDecl* node) override;
    void visitInterfaceDecl(InterfaceDecl* node) override;
    void visitImplDecl(ImplDecl* node) override;
    void visitFieldDecl(FieldDecl* node) override;
    void visitVarDecl(VarDecl* node) override;
    void visitConstDecl(ConstDecl* node) override;
    void visitBlockStmt(BlockStmt* node) override;
    void visitVarDeclStmt(VarDeclStmt* node) override;
    void visitExprStmt(ExprStmt* node) override;
    void visitIfStmt(IfStmt* node) override;
    void visitWhileStmt(WhileStmt* node) override;
    void visitForStmt(ForStmt* node) override;
    void visitReturnStmt(ReturnStmt* node) override;
    void visitMatchStmt(MatchStmt* node) override;
    void visitIntLitExpr(IntLitExpr* node) override;
    void visitFloatLitExpr(FloatLitExpr* node) override;
    void visitStringLitExpr(StringLitExpr* node) override;
    void visitBoolLitExpr(BoolLitExpr* node) override;
    void visitNullLitExpr(NullLitExpr* node) override;
    void visitIdentExpr(IdentExpr* node) override;
    void visitBinaryExpr(BinaryExpr* node) override;
    void visitUnaryExpr(UnaryExpr* node) override;
    void visitCallExpr(CallExpr* node) override;
    void visitMemberExpr(MemberExpr* node) override;
    void visitModulePathExpr(ModulePathExpr* node) override;
    void visitAssignExpr(AssignExpr* node) override;
    void visitIfExpr(IfExpr* node) override;
    void visitLambdaExpr(LambdaExpr* node) override;
    void visitNamedType(NamedType* node) override;
    void visitNullableType(NullableType* node) override;
    void visitSliceType(SliceType* node) override;

private:
    const SourceManager& srcMgr_;
    FILE*                out_;
    bool                 useColor_;
    int                  indent_ = 0;

    void printIndent();
    void printLoc(SourceLocation loc);
    void printNode(const char* name, SourceLocation loc = SourceLocation::invalid());

    struct IndentGuard {
        int& indent;
        IndentGuard(int& i) : indent(i) { indent += 2; }
        ~IndentGuard() { indent -= 2; }
    };

    IndentGuard indented() { return IndentGuard(indent_); }

    const char* blue()  const { return useColor_ ? "\033[1;34m" : ""; }
    const char* green() const { return useColor_ ? "\033[1;32m" : ""; }
    const char* cyan()  const { return useColor_ ? "\033[1;36m" : ""; }
    const char* reset() const { return useColor_ ? "\033[0m"    : ""; }
};

} // namespace vex