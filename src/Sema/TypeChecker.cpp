// src/Sema/TypeChecker.cpp
#include "vex/Sema/TypeChecker.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Stmt.h"
#include "vex/AST/Type.h"
#include "vex/Core/DiagnosticIDs.h"
#include <unordered_map>

namespace vex {

bool TypeChecker::run(ModuleDecl* mod) {
    if (!mod) return false;
    for (auto* d : mod->decls) visit(static_cast<DeclNode*>(d));
    return !ctx_.diags().hasErrors();
}

// ── Declarations ──────────────────────────────────────────────────────────────

void TypeChecker::visitFnDecl(FnDecl* n) {
    checkFunctionBody(n);
}

void TypeChecker::visitStructDecl(StructDecl* n) {
    for (auto* m : n->members) visit(static_cast<DeclNode*>(m));
}

void TypeChecker::visitClassDecl(ClassDecl* n) {
    for (auto* m : n->members) visit(static_cast<DeclNode*>(m));
}

void TypeChecker::visitEnumDecl(EnumDecl*) { /* variants are leaf nodes */ }

void TypeChecker::visitImplDecl(ImplDecl* n) {
    for (auto* m : n->members) visit(static_cast<DeclNode*>(m));
}

// ── Statements ────────────────────────────────────────────────────────────────

void TypeChecker::visitVarDeclStmt(VarDeclStmt* n) {
    for (auto& b : n->bindings) {
        TypeNode* initTy = b.initializer ? inferType(b.initializer) : nullptr;
        if (b.type && initTy && !isCompatible(b.type, initTy)) {
            ctx_.diags().emitSimple(DiagID::TYPE_Mismatch, b.loc,
                "type mismatch in variable binding");
        }
    }
}

void TypeChecker::visitReturnStmt(ReturnStmt* n) {
    if (n->value) inferType(n->value);
}

void TypeChecker::visitIfStmt(IfStmt* n) {
    for (auto& br : n->branches) {
        if (br.condition) inferType(br.condition);
        if (br.body)      visit(static_cast<StmtNode*>(br.body));
    }
    if (n->elseBranch) visit(static_cast<StmtNode*>(n->elseBranch));
}

void TypeChecker::visitWhileStmt(WhileStmt* n) {
    if (n->condition) inferType(n->condition);
    if (n->body)      visit(static_cast<StmtNode*>(n->body));
}

void TypeChecker::visitForStmt(ForStmt* n) {
    if (n->iterable) inferType(n->iterable);
    if (n->body)     visit(static_cast<StmtNode*>(n->body));
}

// ── Type inference ────────────────────────────────────────────────────────────

TypeNode* TypeChecker::inferType(ExprNode* e) {
    if (!e) return nullptr;
    switch (e->kind()) {
    case ASTNodeKind::IntLitExpr:    return makeNamedType("i64");
    case ASTNodeKind::FloatLitExpr:  return makeNamedType("f64");
    case ASTNodeKind::StringLitExpr: return makeNamedType("str");
    case ASTNodeKind::BoolLitExpr:   return makeNamedType("bool");
    case ASTNodeKind::CharLitExpr:   return makeNamedType("char");
    case ASTNodeKind::NullLitExpr:   return nullptr;
    case ASTNodeKind::BinaryExpr:    return inferBinaryType(static_cast<BinaryExpr*>(e));
    case ASTNodeKind::CallExpr:      return inferCallType(static_cast<CallExpr*>(e));
    case ASTNodeKind::MemberExpr:    return inferMemberType(static_cast<MemberExpr*>(e));
    case ASTNodeKind::UnaryExpr:     return inferType(static_cast<UnaryExpr*>(e)->operand);
    case ASTNodeKind::AssignExpr: {
        auto* ae = static_cast<AssignExpr*>(e);
        TypeNode* lt = inferType(ae->target);
        TypeNode* rt = inferType(ae->value);
        if (lt && rt && !isCompatible(lt, rt))
            ctx_.diags().emitSimple(DiagID::TYPE_Mismatch, e->location(),
                "type mismatch in assignment");
        return lt;
    }
    case ASTNodeKind::IndexExpr: {
        auto* ie = static_cast<IndexExpr*>(e);
        inferType(ie->base); inferType(ie->index);
        return nullptr;
    }
    case ASTNodeKind::IfExpr: {
        auto* ife = static_cast<IfExpr*>(e);
        inferType(ife->condition);
        TypeNode* tt = inferType(ife->thenBranch);
        TypeNode* et = inferType(ife->elseBranch);
        return tt ? tt : et;
    }
    default: return nullptr;
    }
}

TypeNode* TypeChecker::inferBinaryType(BinaryExpr* e) {
    TypeNode* lt = inferType(e->left);
    TypeNode* rt = inferType(e->right);
    if (lt && rt && !isCompatible(lt, rt))
        ctx_.diags().emitSimple(DiagID::TYPE_Mismatch, e->location(),
            "type mismatch in binary expression");
    // Comparison operators → bool
    switch (e->op) {
    case TokenKind::Eq: case TokenKind::NotEq:
    case TokenKind::Less: case TokenKind::LessEq:
    case TokenKind::Greater: case TokenKind::GreaterEq:
    case TokenKind::AmpAmp: case TokenKind::PipePipe:
        return makeNamedType("bool");
    default: return lt ? lt : rt;
    }
}

TypeNode* TypeChecker::inferCallType(CallExpr* e) {
    inferType(e->callee);
    for (auto& arg : e->args) inferType(arg.value);
    return nullptr; // resolved by overload resolution
}

TypeNode* TypeChecker::inferMemberType(MemberExpr* e) {
    inferType(e->base);
    return nullptr;
}

// ── Compatibility ─────────────────────────────────────────────────────────────

bool TypeChecker::isAssignable(TypeNode* from, TypeNode* to) {
    return isCompatible(from, to);
}

bool TypeChecker::isCompatible(TypeNode* a, TypeNode* b) {
    if (!a || !b) return true;  // unknown = permissive
    if (a == b)   return true;  // same node = same type
    return true; // permissive until full type unification is built
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::unordered_map<std::string, TypeNode*> s_builtinTypeCache;
TypeNode* TypeChecker::makeNamedType(std::string_view name) {
    std::string key(name);
    auto it = s_builtinTypeCache.find(key);
    if (it != s_builtinTypeCache.end()) return it->second;
    auto* n = ctx_.astCtx().make<NamedType>(SourceLocation{}); n->name = name;
    s_builtinTypeCache[key] = n;
    return n;
}

TypeNode* TypeChecker::makeNullableType(TypeNode* inner) {
    return inner; // simplified; real impl wraps in NullableType
}

void TypeChecker::checkFunctionBody(FnDecl* fn) {
    if (!fn->body) return;
    visit(static_cast<StmtNode*>(fn->body));
}

} // namespace vex
