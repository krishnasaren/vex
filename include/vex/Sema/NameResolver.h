#pragma once
// include/vex/Sema/NameResolver.h



#pragma once
// ============================================================================
// vex/Sema/NameResolver.h
// Pass 1 of sema: collects declarations and resolves identifier references.
// ============================================================================

#include "vex/Sema/SemaContext.h"
#include "vex/AST/ASTVisitor.h"

namespace vex {

class NameResolver : public ASTVisitor {
public:
    explicit NameResolver(SemaContext& ctx) : ctx_(ctx) {}

    bool run(ModuleDecl* mod);

    // Collect declarations (pass 1 — before type checking)
    void collectDecls(ModuleDecl* mod);

    // Resolve uses (pass 2 — after collecting)
    void resolveUses(ModuleDecl* mod);

    // ASTVisitor overrides
    void visitModuleDecl(ModuleDecl* n)   override;
    void visitFnDecl(FnDecl* n)           override;
    void visitStructDecl(StructDecl* n)   override;
    void visitClassDecl(ClassDecl* n)     override;
    void visitEnumDecl(EnumDecl* n)       override;
    void visitTraitDecl(TraitDecl* n)     override;
    void visitInterfaceDecl(InterfaceDecl* n) override;
    void visitImplDecl(ImplDecl* n)       override;
    void visitVarDecl(VarDecl* n)         override;
    void visitConstDecl(ConstDecl* n)     override;
    void visitImportDecl(ImportDecl* n)   override;
    void visitVarDeclStmt(VarDeclStmt* n) override;
    void visitIdentExpr(IdentExpr* n)     override;
    void visitBlockStmt(BlockStmt* n)     override;
    void visitForStmt(ForStmt* n)         override;

private:
    SemaContext& ctx_;
    bool         collectingPhase_ = true;

    void defineBuiltins();
    void defineBuiltinType(std::string_view name);
};

} // namespace vex

