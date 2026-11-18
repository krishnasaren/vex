// src/Sema/NameResolver.cpp
#include "vex/Sema/NameResolver.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Stmt.h"
#include "vex/Core/DiagnosticIDs.h"

namespace vex {

// ── Helpers ───────────────────────────────────────────────────────────────────

void NameResolver::defineBuiltinType(std::string_view name) {
    ctx_.symbols().define(SymbolKind::Builtin, name, nullptr, {});
}

void NameResolver::defineBuiltins() {
    // Built-in constructors
    for (auto n : {"Ok", "Err", "Some", "None", "true", "false"})
        ctx_.symbols().define(SymbolKind::Builtin, n, nullptr, {});
    for (auto n : {"i8","i16","i32","i64","i128","u8","u16","u32","u64","u128",
                   "f32","f64","bool","str","char","void","never",
                   "uint","int","usize","isize","byte","rune","any","string"})
        defineBuiltinType(n);
}

// ── Entry points ──────────────────────────────────────────────────────────────

bool NameResolver::run(ModuleDecl* mod) {
    collectingPhase_ = true;
    defineBuiltins();
    collectDecls(mod);
    collectingPhase_ = false;
    resolveUses(mod);
    return !ctx_.diags().hasErrors();
}

void NameResolver::collectDecls(ModuleDecl* mod) {
    if (!mod) return;
    visit(static_cast<DeclNode*>(mod));
}

void NameResolver::resolveUses(ModuleDecl* mod) {
    if (!mod) return;
    visit(static_cast<DeclNode*>(mod));
}

// ── Declarations (via ASTVisitor overrides) ───────────────────────────────────

void NameResolver::visitModuleDecl(ModuleDecl* n) {
    if (collectingPhase_) {
        for (auto* d : n->decls) visit(static_cast<DeclNode*>(d));
    } else {
        for (auto* i : n->imports) visit(static_cast<DeclNode*>(i));
        for (auto* d : n->decls)   visit(static_cast<DeclNode*>(d));
    }
}

void NameResolver::visitImportDecl(ImportDecl*) { /* module system resolves */ }

void NameResolver::visitFnDecl(FnDecl* n) {
    if (collectingPhase_) {
        ctx_.symbols().define(SymbolKind::Function, n->name, n, n->location());
        return;
    }
    ctx_.symbols().pushScope(Scope::Kind::Function);
    for (auto* p : n->params)
        ctx_.symbols().define(SymbolKind::Parameter, p->name, p, p->location());
    if (n->body) visit(static_cast<StmtNode*>(n->body));
    ctx_.symbols().popScope();
}

void NameResolver::visitStructDecl(StructDecl* n) {
    if (collectingPhase_) {
        ctx_.symbols().define(SymbolKind::Struct, n->name, n, n->location());
        return;
    }
    ctx_.symbols().pushScope(Scope::Kind::Struct);
    for (auto* m : n->members) visit(static_cast<DeclNode*>(m));
    ctx_.symbols().popScope();
}

void NameResolver::visitClassDecl(ClassDecl* n) {
    if (collectingPhase_) {
        ctx_.symbols().define(SymbolKind::Class, n->name, n, n->location());
        return;
    }
    ctx_.symbols().pushScope(Scope::Kind::Struct);
    for (auto* m : n->members) visit(static_cast<DeclNode*>(m));
    ctx_.symbols().popScope();
}

void NameResolver::visitEnumDecl(EnumDecl* n) {
    if (collectingPhase_) {
        ctx_.symbols().define(SymbolKind::Enum, n->name, n, n->location());
        for (auto* v : n->variants)
            ctx_.symbols().define(SymbolKind::EnumVariant, v->name, v, v->location());
    }
}

void NameResolver::visitTraitDecl(TraitDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Trait, n->name, n, n->location());
}

void NameResolver::visitInterfaceDecl(InterfaceDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Interface, n->name, n, n->location());
}

void NameResolver::visitImplDecl(ImplDecl* n) {
    if (!collectingPhase_) {
        ctx_.symbols().pushScope(Scope::Kind::Struct);
        for (auto* m : n->members) visit(static_cast<DeclNode*>(m));
        ctx_.symbols().popScope();
    }
}

void NameResolver::visitVarDecl(VarDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Variable, n->name, n, n->location());
}

void NameResolver::visitConstDecl(ConstDecl* n) {
    if (collectingPhase_)
        ctx_.symbols().define(SymbolKind::Constant, n->name, n, n->location());
    else if (n->value)
        visit(static_cast<ExprNode*>(n->value));
}

// ── Statements ────────────────────────────────────────────────────────────────

void NameResolver::visitBlockStmt(BlockStmt* n) {
    if (collectingPhase_) return;
    ctx_.symbols().pushScope(Scope::Kind::Block);
    for (auto* s : n->stmts)       visit(static_cast<StmtNode*>(s));
    if (n->trailingExpr)            visit(static_cast<ExprNode*>(n->trailingExpr));
    ctx_.symbols().popScope();
}

void NameResolver::visitVarDeclStmt(VarDeclStmt* n) {
    if (collectingPhase_) return;
    for (auto& b : n->bindings) {
        if (b.initializer) visit(static_cast<ExprNode*>(b.initializer));
        if (!b.isDiscard)
            ctx_.symbols().define(SymbolKind::Variable, b.name, nullptr, b.loc);
    }
}

void NameResolver::visitForStmt(ForStmt* n) {
    if (collectingPhase_) return;
    ctx_.symbols().pushScope(Scope::Kind::Loop);
    if (!n->valueVar.empty())
        ctx_.symbols().define(SymbolKind::Variable, n->valueVar, nullptr, n->location());
    if (n->iterable)  visit(static_cast<ExprNode*>(n->iterable));
    if (n->body)      visit(static_cast<StmtNode*>(n->body));
    ctx_.symbols().popScope();
}

// ── Expressions ───────────────────────────────────────────────────────────────

void NameResolver::visitLambdaExpr(LambdaExpr* n) {
    if (collectingPhase_) return;
    ctx_.symbols().pushScope(Scope::Kind::Function);
    for (const auto& p : n->params)
        ctx_.symbols().define(SymbolKind::Parameter, p.name, nullptr, p.loc);
    if (n->bodyExpr)   visit(static_cast<ExprNode*>(n->bodyExpr));
    if (n->bodyBlock)  visit(static_cast<StmtNode*>(n->bodyBlock));
    ctx_.symbols().popScope();
}

void NameResolver::visitIdentExpr(IdentExpr* n) {
    if (collectingPhase_) return;
    Symbol* sym = ctx_.symbols().lookup(n->name);
    if (!sym) {
        ctx_.diags().emitSimple(
            DiagID::SEMA_UndefinedName, n->location(),
            "use of undeclared identifier '" + std::string(n->name) + "'");
    } // sym found — resolve in TypeChecker pass
}

} // namespace vex
