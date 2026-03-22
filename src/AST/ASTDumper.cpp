// ============================================================================
// vex/AST/ASTDumper.cpp
// ============================================================================

#include "vex/AST/ASTDumper.h"
#include "vex/Lexer/TokenKind.h"
#include <cstring>

namespace vex {

void ASTDumper::printIndent() {
    for (int i = 0; i < indent_; ++i)
        fputc(' ', out_);
}

void ASTDumper::printLoc(SourceLocation loc) {
    if (!loc.isValid()) return;
    auto path = srcMgr_.getFilePath(loc);
    // Print just filename
    auto slash = path.rfind('/');
    if (slash == std::string_view::npos) slash = path.rfind('\\');
    auto name = (slash != std::string_view::npos) ? path.substr(slash+1) : path;
    fprintf(out_, " %s<%.*s:%u:%u>%s",
            cyan(), (int)name.size(), name.data(),
            loc.line(), loc.col(), reset());
}

void ASTDumper::printNode(const char* name, SourceLocation loc) {
    printIndent();
    fprintf(out_, "%s%s%s", blue(), name, reset());
    printLoc(loc);
    fputc('\n', out_);
}

void ASTDumper::dump(ModuleDecl* module) {
    visitModuleDecl(module);
}

void ASTDumper::visitModuleDecl(ModuleDecl* node) {
    printNode("ModuleDecl", node->location());
    fprintf(out_, "  name='%.*s'\n", (int)node->name.size(), node->name.data());
    auto g = indented();
    for (auto* imp : node->imports) dispatch(imp);
    for (auto* d   : node->decls)   dispatch(d);
}

void ASTDumper::visitImportDecl(ImportDecl* node) {
    printIndent();
    fprintf(out_, "%sImportDecl%s  path='", blue(), reset());
    for (size_t i = 0; i < node->pathSegments.size(); ++i) {
        if (i) fprintf(out_, "::");
        auto& s = node->pathSegments[i];
        fprintf(out_, "%.*s", (int)s.size(), s.data());
    }
    fprintf(out_, "'");
    if (!node->alias.empty())
        fprintf(out_, " as='%.*s'", (int)node->alias.size(), node->alias.data());
    if (node->isOpenImport)
        fprintf(out_, " {*}");
    else if (!node->selectiveItems.empty()) {
        fprintf(out_, " {");
        for (size_t i = 0; i < node->selectiveItems.size(); ++i) {
            if (i) fprintf(out_, ", ");
            auto& it = node->selectiveItems[i];
            fprintf(out_, "%.*s", (int)it.originalName.size(), it.originalName.data());
            if (it.localName != it.originalName)
                fprintf(out_, " as %.*s", (int)it.localName.size(), it.localName.data());
        }
        fprintf(out_, "}");
    }
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitFnDecl(FnDecl* node) {
    printIndent();
    fprintf(out_, "%sFnDecl%s  name='%.*s'",
            blue(), reset(),
            (int)node->name.size(), node->name.data());
    if (node->isAsync)     fprintf(out_, " async");
    if (node->isGenerator) fprintf(out_, " generator");
    if (node->isStatic)    fprintf(out_, " static");
    if (node->isAbstract)  fprintf(out_, " abstract");
    printLoc(node->location());
    fputc('\n', out_);
    if (node->body) {
        auto g = indented();
        dispatch(node->body);
    }
}

void ASTDumper::visitStructDecl(StructDecl* node) {
    printNode("StructDecl", node->location());
    fprintf(out_, "  name='%.*s'\n", (int)node->name.size(), node->name.data());
    auto g = indented();
    for (auto* m : node->members) dispatch(m);
}

void ASTDumper::visitClassDecl(ClassDecl* node) {
    printNode("ClassDecl", node->location());
    fprintf(out_, "  name='%.*s'\n", (int)node->name.size(), node->name.data());
    auto g = indented();
    for (auto* m : node->members) dispatch(m);
}

void ASTDumper::visitEnumDecl(EnumDecl* node) {
    printNode("EnumDecl", node->location());
    fprintf(out_, "  name='%.*s'\n", (int)node->name.size(), node->name.data());
    auto g = indented();
    for (auto* v : node->variants) {
        printIndent();
        fprintf(out_, "%sEnumVariant%s  '%.*s'\n",
                blue(), reset(),
                (int)v->name.size(), v->name.data());
    }
}

void ASTDumper::visitTraitDecl(TraitDecl* node) {
    printNode("TraitDecl", node->location());
    fprintf(out_, "  name='%.*s'\n", (int)node->name.size(), node->name.data());
}

void ASTDumper::visitInterfaceDecl(InterfaceDecl* node) {
    printNode("InterfaceDecl", node->location());
    fprintf(out_, "  name='%.*s'\n", (int)node->name.size(), node->name.data());
}

void ASTDumper::visitImplDecl(ImplDecl* node) {
    printNode("ImplDecl", node->location());
    auto g = indented();
    for (auto* m : node->members) dispatch(m);
}

void ASTDumper::visitFieldDecl(FieldDecl* node) {
    printIndent();
    fprintf(out_, "%sFieldDecl%s  name='%.*s'",
            blue(), reset(),
            (int)node->name.size(), node->name.data());
    if (node->isStatic) fprintf(out_, " static");
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitVarDecl(VarDecl* node) {
    printIndent();
    fprintf(out_, "%sVarDecl%s  name='%.*s' %s",
            blue(), reset(),
            (int)node->name.size(), node->name.data(),
            node->isMutable ? "var" : "let");
    printLoc(node->location());
    fputc('\n', out_);
    if (node->initializer) {
        auto g = indented();
        dispatch(node->initializer);
    }
}

void ASTDumper::visitConstDecl(ConstDecl* node) {
    printIndent();
    fprintf(out_, "%sConstDecl%s  name='%.*s'",
            blue(), reset(),
            (int)node->name.size(), node->name.data());
    printLoc(node->location());
    fputc('\n', out_);
    if (node->value) {
        auto g = indented();
        dispatch(node->value);
    }
}

void ASTDumper::visitBlockStmt(BlockStmt* node) {
    printNode("BlockStmt", node->location());
    auto g = indented();
    for (auto* s : node->stmts) dispatch(s);
    if (node->trailingExpr) dispatch(node->trailingExpr);
}

void ASTDumper::visitVarDeclStmt(VarDeclStmt* node) {
    printNode("VarDeclStmt", node->location());
    auto g = indented();
    for (auto& b : node->bindings) {
        printIndent();
        fprintf(out_, "Binding '%.*s' %s\n",
                (int)b.name.size(), b.name.data(),
                node->isMutable ? "var" : "let");
        if (b.initializer) {
            auto g2 = indented();
            dispatch(b.initializer);
        }
    }
}

void ASTDumper::visitExprStmt(ExprStmt* node) {
    printNode("ExprStmt", node->location());
    auto g = indented();
    dispatch(node->expr);
}

void ASTDumper::visitIfStmt(IfStmt* node) {
    printNode("IfStmt", node->location());
    auto g = indented();
    for (auto& br : node->branches) {
        printIndent();
        fprintf(out_, "Branch\n");
        auto g2 = indented();
        if (br.condition) dispatch(br.condition);
        if (br.body)      dispatch(br.body);
    }
    if (node->elseBranch) {
        printIndent();
        fprintf(out_, "ElseBranch\n");
        auto g2 = indented();
        dispatch(node->elseBranch);
    }
}

void ASTDumper::visitWhileStmt(WhileStmt* node) {
    printNode("WhileStmt", node->location());
    auto g = indented();
    if (node->condition) dispatch(node->condition);
    if (node->body)      dispatch(node->body);
}

void ASTDumper::visitForStmt(ForStmt* node) {
    printIndent();
    fprintf(out_, "%sForStmt%s  var='%.*s'",
            blue(), reset(),
            (int)node->valueVar.size(), node->valueVar.data());
    if (!node->indexVar.empty())
        fprintf(out_, " index='%.*s'", (int)node->indexVar.size(), node->indexVar.data());
    printLoc(node->location());
    fputc('\n', out_);
    auto g = indented();
    if (node->iterable) dispatch(node->iterable);
    if (node->body)     dispatch(node->body);
}

void ASTDumper::visitReturnStmt(ReturnStmt* node) {
    printNode("ReturnStmt", node->location());
    if (node->value) {
        auto g = indented();
        dispatch(node->value);
    }
}

void ASTDumper::visitMatchStmt(MatchStmt* node) {
    printNode("MatchStmt", node->location());
    auto g = indented();
    if (node->subject) dispatch(node->subject);
    for (auto* arm : node->arms) {
        printIndent();
        fprintf(out_, "MatchArm\n");
    }
}

void ASTDumper::visitIntLitExpr(IntLitExpr* node) {
    printIndent();
    fprintf(out_, "%sIntLit%s  %llu", green(), reset(),
            (unsigned long long)node->value);
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitFloatLitExpr(FloatLitExpr* node) {
    printIndent();
    fprintf(out_, "%sFloatLit%s  %g", green(), reset(), node->value);
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitStringLitExpr(StringLitExpr* node) {
    printIndent();
    fprintf(out_, "%sStringLit%s  \"%.*s\"",
            green(), reset(),
            (int)node->value.size(), node->value.data());
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitBoolLitExpr(BoolLitExpr* node) {
    printIndent();
    fprintf(out_, "%sBoolLit%s  %s", green(), reset(),
            node->value ? "true" : "false");
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitNullLitExpr(NullLitExpr* node) {
    printNode("NullLit", node->location());
}

void ASTDumper::visitIdentExpr(IdentExpr* node) {
    printIndent();
    fprintf(out_, "%sIdent%s  '%.*s'",
            green(), reset(),
            (int)node->name.size(), node->name.data());
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitBinaryExpr(BinaryExpr* node) {
    printIndent();
    auto opName = tokenKindName(node->op);
    fprintf(out_, "%sBinaryExpr%s  op='%.*s'",
            green(), reset(),
            (int)opName.size(), opName.data());
    printLoc(node->location());
    fputc('\n', out_);
    auto g = indented();
    dispatch(node->left);
    dispatch(node->right);
}

void ASTDumper::visitUnaryExpr(UnaryExpr* node) {
    printIndent();
    auto opName = tokenKindName(node->op);
    fprintf(out_, "%sUnaryExpr%s  op='%.*s' %s",
            green(), reset(),
            (int)opName.size(), opName.data(),
            node->isPostfix ? "postfix" : "prefix");
    printLoc(node->location());
    fputc('\n', out_);
    auto g = indented();
    dispatch(node->operand);
}

void ASTDumper::visitCallExpr(CallExpr* node) {
    printNode("CallExpr", node->location());
    auto g = indented();
    dispatch(node->callee);
    for (auto& arg : node->args) {
        printIndent();
        if (!arg.name.empty())
            fprintf(out_, "Arg '%.*s':\n",
                    (int)arg.name.size(), arg.name.data());
        else
            fprintf(out_, "Arg:\n");
        auto g2 = indented();
        dispatch(arg.value);
    }
}

void ASTDumper::visitMemberExpr(MemberExpr* node) {
    printIndent();
    fprintf(out_, "%sMemberExpr%s  .%.*s",
            green(), reset(),
            (int)node->member.size(), node->member.data());
    printLoc(node->location());
    fputc('\n', out_);
    auto g = indented();
    dispatch(node->base);
}

void ASTDumper::visitModulePathExpr(ModulePathExpr* node) {
    printIndent();
    fprintf(out_, "%sModulePath%s  ", green(), reset());
    for (size_t i = 0; i < node->segments.size(); ++i) {
        if (i) fprintf(out_, "::");
        auto& s = node->segments[i];
        fprintf(out_, "%.*s", (int)s.size(), s.data());
    }
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitAssignExpr(AssignExpr* node) {
    printIndent();
    auto opName = tokenKindName(node->op);
    fprintf(out_, "%sAssignExpr%s  op='%.*s'",
            green(), reset(),
            (int)opName.size(), opName.data());
    printLoc(node->location());
    fputc('\n', out_);
    auto g = indented();
    dispatch(node->target);
    dispatch(node->value);
}

void ASTDumper::visitIfExpr(IfExpr* node) {
    printNode("IfExpr", node->location());
    auto g = indented();
    if (node->condition)  dispatch(node->condition);
    if (node->thenBranch) dispatch(node->thenBranch);
    if (node->elseBranch) dispatch(node->elseBranch);
}

void ASTDumper::visitLambdaExpr(LambdaExpr* node) {
    printIndent();
    fprintf(out_, "%sLambdaExpr%s", green(), reset());
    if (node->isCopyCapture) fprintf(out_, " copy");
    if (node->isMoveCapture) fprintf(out_, " move");
    if (node->isAsync)       fprintf(out_, " async");
    printLoc(node->location());
    fputc('\n', out_);
    auto g = indented();
    if (node->bodyExpr)  dispatch(node->bodyExpr);
    if (node->bodyBlock) dispatch(node->bodyBlock);
}

void ASTDumper::visitNamedType(NamedType* node) {
    printIndent();
    fprintf(out_, "%sNamedType%s  '%.*s'",
            cyan(), reset(),
            (int)node->name.size(), node->name.data());
    printLoc(node->location());
    fputc('\n', out_);
}

void ASTDumper::visitNullableType(NullableType* node) {
    printNode("NullableType", node->location());
    auto g = indented();
    dispatch(node->inner);
}

void ASTDumper::visitSliceType(SliceType* node) {
    printNode("SliceType", node->location());
    auto g = indented();
    dispatch(node->elem);
}

} // namespace vex