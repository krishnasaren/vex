// src/Sema/NameResolver.cpp




#include "vex/Sema/NameResolver.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Stmt.h"
#include "vex/Core/DiagnosticEngine.h"

namespace vex {

bool NameResolver::run(ModuleDecl* mod) {
    collectingPhase_ = true;
    defineBuiltins();
    collectDecls(mod);
    collectingPhase_ = false;
    resolveUses(mod);
    return !ctx_.hasErrors();
}

void NameResolver::defineBuiltins() {
    for (auto name : {"i8","i16","i32","i64","i128",
                      "u8","u16","u32","u64","u128",
                      "f32","f64","bool","str","char",
                      "void","never","uint","int","usize","isize"})
        defineBuiltinType(name);
}

void NameResolver::defineBuiltinType(std::string_view name) {
    ctx_.symbols().define(SymbolKind::Builtin, name, nullptr, SourceLocation{});
}

void NameResolver::collectDecls(ModuleDecl* mod) {
    visit(mod);
}

void NameResolver::resolveUses(ModuleDecl* mod) {
    visit(mod);
}

void NameResolver::visitModuleDecl(ModuleDecl* n) {
    ctx_.symbols().pushScope(Scope::Kind::Module);
    visitChildren(n);
    ctx_.symbols().popScope();
}

void NameResolver::visitFnDecl(FnDecl* n) {
    if (collectingPhase_) {
        ctx_.symbols().define(SymbolKind::Function, n->name(), n, n->location());
        return;
    }
    ctx_.symbols().pushScope(Scope::Kind::Function);
    for (auto* p : n->params())
        ctx_.symbols().define(SymbolKind::Parameter, p->name(), p, p->location());
    if (n->body()) visit(n->body());
    ctx_.symbols().popScope();
}

void NameResolver::visitStructDecl(StructDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Struct, n->name(), n, n->location());
}

void NameResolver::visitClassDecl(ClassDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Class, n->name(), n, n->location());
}

void NameResolver::visitEnumDecl(EnumDecl* n) {
    if (collectingPhase_) {
        ctx_.symbols().define(SymbolKind::Enum, n->name(), n, n->location());
        for (auto* v : n->variants())
            ctx_.symbols().define(SymbolKind::EnumVariant, v->name(), v, v->location());
    }
}

void NameResolver::visitTraitDecl(TraitDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Trait, n->name(), n, n->location());
}

void NameResolver::visitInterfaceDecl(InterfaceDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Interface, n->name(), n, n->location());
}

void NameResolver::visitImplDecl(ImplDecl* n) {
    if (!collectingPhase_) {
        ctx_.symbols().pushScope(Scope::Kind::Struct);
        visitChildren(n);
        ctx_.symbols().popScope();
    }
}

void NameResolver::visitVarDecl(VarDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Variable, n->name(), n, n->location());
}

void NameResolver::visitConstDecl(ConstDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Constant, n->name(), n, n->location());
}

void NameResolver::visitImportDecl(ImportDecl* n) {
    // Import resolution is handled by the module system; skip here
}

void NameResolver::visitVarDeclStmt(VarDeclStmt* n) {
    if (!collectingPhase_) {
        if (n->init()) visit(n->init());
        ctx_.symbols().define(SymbolKind::Variable, n->name(), n, n->location());
    }
}

void NameResolver::visitIdentExpr(IdentExpr* n) {
    if (collectingPhase_) return;
    Symbol* sym = ctx_.symbols().lookup(n->name());
    if (!sym) {
        ctx_.diags().error(n->location(),
            "use of undeclared identifier '%0'") << n->name();
    } else {
        n->setSymbol(sym);
    }
}

void NameResolver::visitBlockStmt(BlockStmt* n) {
    if (collectingPhase_) return;
    ctx_.symbols().pushScope(Scope::Kind::Block);
    visitChildren(n);
    ctx_.symbols().popScope();
}

void NameResolver::visitForStmt(ForStmt* n) {
    if (collectingPhase_) return;
    ctx_.symbols().pushScope(Scope::Kind::Loop);
    ctx_.symbols().define(SymbolKind::Variable, n->bindingName(), n, n->location());
    visit(n->body());
    ctx_.symbols().popScope();
}

} // namespace vex
