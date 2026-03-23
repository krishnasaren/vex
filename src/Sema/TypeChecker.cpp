// src/Sema/TypeChecker.cpp



#include "vex/Sema/TypeChecker.h"
#include "vex/Sema/TypeInference.h"
#include "vex/Sema/TypeUnifier.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Stmt.h"

namespace vex {

bool TypeChecker::run(ModuleDecl* mod) {
    TypeInference infer(ctx_);
    for (auto* decl : mod->decls()) {
        if (auto* fn = dyn_cast<FnDecl>(decl))
            infer.inferFunction(fn);
    }
    visit(mod);
    return !ctx_.hasErrors();
}

void TypeChecker::visitFnDecl(FnDecl* n) {
    auto prev = ctx_.currentFn;
    ctx_.currentFn = n;
    checkFunctionBody(n);
    ctx_.currentFn = prev;
}

void TypeChecker::visitStructDecl(StructDecl*) {}
void TypeChecker::visitClassDecl(ClassDecl*) {}
void TypeChecker::visitEnumDecl(EnumDecl*) {}
void TypeChecker::visitImplDecl(ImplDecl* n) { visitChildren(n); }

void TypeChecker::visitVarDeclStmt(VarDeclStmt* n) {
    if (!n->init()) return;
    TypeNode* initTy = inferType(n->init());
    if (!initTy) return;
    if (n->typeAnnotation()) {
        TypeUnifier u(ctx_);
        if (!u.convertible(initTy, n->typeAnnotation())) {
            ctx_.diags().error(n->location(),
                "type mismatch: cannot assign '%0' to '%1'")
                << initTy->toString() << n->typeAnnotation()->toString();
        }
    } else {
        n->setInferredType(initTy);
    }
}

void TypeChecker::visitReturnStmt(ReturnStmt* n) {
    if (!ctx_.currentFn) return;
    TypeNode* retTy = ctx_.currentFn->returnType();
    if (n->value()) {
        TypeNode* valTy = inferType(n->value());
        if (valTy && retTy) {
            TypeUnifier u(ctx_);
            if (!u.convertible(valTy, retTy)) {
                ctx_.diags().error(n->location(),
                    "return type mismatch: got '%0' expected '%1'")
                    << valTy->toString() << retTy->toString();
            }
        }
    }
}

void TypeChecker::visitIfStmt(IfStmt* n) {
    TypeNode* condTy = inferType(n->condition());
    TypeUnifier u(ctx_);
    if (condTy && !u.isBool(condTy)) {
        ctx_.diags().error(n->condition()->location(),
            "if condition must be bool, got '%0'") << condTy->toString();
    }
    visitChildren(n);
}

void TypeChecker::visitWhileStmt(WhileStmt* n) {
    TypeNode* condTy = inferType(n->condition());
    TypeUnifier u(ctx_);
    if (condTy && !u.isBool(condTy)) {
        ctx_.diags().error(n->condition()->location(),
            "while condition must be bool") ;
    }
    visitChildren(n);
}

void TypeChecker::visitForStmt(ForStmt* n) { visitChildren(n); }

TypeNode* TypeChecker::inferType(ExprNode* expr) {
    TypeInference infer(ctx_);
    return infer.infer(expr);
}

TypeNode* TypeChecker::inferBinaryType(BinaryExpr* e) {
    return inferType(e);
}

TypeNode* TypeChecker::inferCallType(CallExpr* e) {
    return inferType(e);
}

TypeNode* TypeChecker::inferMemberType(MemberExpr* e) {
    return inferType(e);
}

bool TypeChecker::isAssignable(TypeNode* from, TypeNode* to) {
    TypeUnifier u(ctx_);
    return u.convertible(from, to);
}

bool TypeChecker::isCompatible(TypeNode* a, TypeNode* b) {
    TypeUnifier u(ctx_);
    return u.equal(a, b);
}

TypeNode* TypeChecker::makeNamedType(std::string_view name) {
    return ctx_.astCtx().makeNamedType(name);
}

TypeNode* TypeChecker::makeNullableType(TypeNode* inner) {
    return ctx_.astCtx().makeNullableType(inner);
}

void TypeChecker::checkFunctionBody(FnDecl* fn) {
    if (fn->body()) visit(fn->body());
}

} // namespace vex
