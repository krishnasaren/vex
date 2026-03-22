#pragma once
// ============================================================================
// vex/AST/ASTVisitor.h
// Visitor base class for all AST passes.
// ============================================================================

#include "vex/AST/Decl.h"
#include "vex/AST/Stmt.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Type.h"
#include "vex/AST/Pattern.h"

namespace vex {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // ── Declarations ─────────────────────────────────────────────────────────
    virtual void visitModuleDecl(ModuleDecl* n);
    virtual void visitImportDecl(ImportDecl* n)       { (void)n; }
    virtual void visitFnDecl(FnDecl* n);
    virtual void visitInitDecl(InitDecl* n);
    virtual void visitDeinitDecl(DeinitDecl* n);
    virtual void visitStructDecl(StructDecl* n);
    virtual void visitClassDecl(ClassDecl* n);
    virtual void visitEnumDecl(EnumDecl* n);
    virtual void visitEnumVariantDecl(EnumVariantDecl* n) { (void)n; }
    virtual void visitTraitDecl(TraitDecl* n);
    virtual void visitInterfaceDecl(InterfaceDecl* n);
    virtual void visitImplDecl(ImplDecl* n);
    virtual void visitFieldDecl(FieldDecl* n)         { (void)n; }
    virtual void visitPropDecl(PropDecl* n)           { (void)n; }
    virtual void visitOperatorDecl(OperatorDecl* n);
    virtual void visitTypeAliasDecl(TypeAliasDecl* n) { (void)n; }
    virtual void visitDistinctDecl(DistinctDecl* n)   { (void)n; }
    virtual void visitVarDecl(VarDecl* n)             { (void)n; }
    virtual void visitConstDecl(ConstDecl* n)         { (void)n; }
    virtual void visitBitfieldDecl(BitfieldDecl* n)   { (void)n; }

    // ── Statements ───────────────────────────────────────────────────────────
    virtual void visitBlockStmt(BlockStmt* n);
    virtual void visitVarDeclStmt(VarDeclStmt* n);
    virtual void visitExprStmt(ExprStmt* n);
    virtual void visitIfStmt(IfStmt* n);
    virtual void visitWhileStmt(WhileStmt* n);
    virtual void visitDoWhileStmt(DoWhileStmt* n);
    virtual void visitForStmt(ForStmt* n);
    virtual void visitForCStmt(ForCStmt* n);
    virtual void visitLoopStmt(LoopStmt* n);
    virtual void visitMatchStmt(MatchStmt* n);
    virtual void visitReturnStmt(ReturnStmt* n);
    virtual void visitSkipStmt(SkipStmt* n)           { (void)n; }
    virtual void visitStopStmt(StopStmt* n)           { (void)n; }
    virtual void visitDeferStmt(DeferStmt* n);
    virtual void visitUseLetStmt(UseLetStmt* n);
    virtual void visitTryCatchStmt(TryCatchStmt* n);
    virtual void visitSelectStmt(SelectStmt* n);
    virtual void visitUnsafeStmt(UnsafeStmt* n);
    virtual void visitAsmStmt(AsmStmt* n)             { (void)n; }
    virtual void visitLabeledStmt(LabeledStmt* n);

    // ── Expressions ──────────────────────────────────────────────────────────
    virtual void visitIntLitExpr(IntLitExpr* n)           { (void)n; }
    virtual void visitFloatLitExpr(FloatLitExpr* n)       { (void)n; }
    virtual void visitStringLitExpr(StringLitExpr* n)     { (void)n; }
    virtual void visitCharLitExpr(CharLitExpr* n)         { (void)n; }
    virtual void visitBoolLitExpr(BoolLitExpr* n)         { (void)n; }
    virtual void visitNullLitExpr(NullLitExpr* n)         { (void)n; }
    virtual void visitFmtStringExpr(FmtStringExpr* n);
    virtual void visitMultilineStrExpr(MultilineStrExpr* n){ (void)n; }
    virtual void visitIdentExpr(IdentExpr* n)             { (void)n; }
    virtual void visitSelfExpr(SelfExpr* n)               { (void)n; }
    virtual void visitSuperExpr(SuperExpr* n)             { (void)n; }
    virtual void visitSelfTypeExpr(SelfTypeExpr* n)       { (void)n; }
    virtual void visitModExpr(ModExpr* n)                 { (void)n; }
    virtual void visitTupleExpr(TupleExpr* n);
    virtual void visitListExpr(ListExpr* n);
    virtual void visitMapExpr(MapExpr* n);
    virtual void visitSetExpr(SetExpr* n);
    virtual void visitStructLitExpr(StructLitExpr* n);
    virtual void visitLambdaExpr(LambdaExpr* n);
    virtual void visitUnaryExpr(UnaryExpr* n);
    virtual void visitBinaryExpr(BinaryExpr* n);
    virtual void visitAssignExpr(AssignExpr* n);
    virtual void visitCallExpr(CallExpr* n);
    virtual void visitIndexExpr(IndexExpr* n);
    virtual void visitMemberExpr(MemberExpr* n);
    virtual void visitModulePathExpr(ModulePathExpr* n)   { (void)n; }
    virtual void visitCastExpr(CastExpr* n);
    virtual void visitSafeCastExpr(SafeCastExpr* n);
    virtual void visitTypeCheckExpr(TypeCheckExpr* n);
    virtual void visitIfExpr(IfExpr* n);
    virtual void visitMatchExpr(MatchExpr* n);
    virtual void visitMatchArm(MatchArm* n);
    virtual void visitNullCoalesceExpr(NullCoalesceExpr* n);
    virtual void visitSafeCallExpr(SafeCallExpr* n);
    virtual void visitErrorPropExpr(ErrorPropExpr* n);
    virtual void visitPipeExpr(PipeExpr* n);
    virtual void visitRangeExpr(RangeExpr* n);
    virtual void visitBlockExpr(BlockExpr* n);
    virtual void visitComptimeExpr(ComptimeExpr* n);
    virtual void visitUnsafeExpr(UnsafeExpr* n);
    virtual void visitGenericInstExpr(GenericInstExpr* n);

    // ── Types ─────────────────────────────────────────────────────────────────
    virtual void visitNamedType(NamedType* n)         { (void)n; }
    virtual void visitQualifiedType(QualifiedType* n) { (void)n; }
    virtual void visitNullableType(NullableType* n);
    virtual void visitOwnedPtrType(OwnedPtrType* n);
    virtual void visitSharedPtrType(SharedPtrType* n);
    virtual void visitBorrowType(BorrowType* n);
    virtual void visitMutBorrowType(MutBorrowType* n);
    virtual void visitRawPtrType(RawPtrType* n);
    virtual void visitSliceType(SliceType* n);
    virtual void visitFixedArrayType(FixedArrayType* n);
    virtual void visitMapType(MapType* n);
    virtual void visitSetType(SetType* n);
    virtual void visitTupleType(TupleType* n);
    virtual void visitFnType(FnType* n);
    virtual void visitGenericType(GenericType* n);
    virtual void visitInferType(InferType* n)         { (void)n; }

    // ── Patterns ─────────────────────────────────────────────────────────────
    virtual void visitWildcardPattern(WildcardPattern* n)   { (void)n; }
    virtual void visitBindingPattern(BindingPattern* n)     { (void)n; }
    virtual void visitLiteralPattern(LiteralPattern* n);
    virtual void visitRangePattern(RangePattern* n);
    virtual void visitOrPattern(OrPattern* n);
    virtual void visitTuplePattern(TuplePattern* n);
    virtual void visitStructPattern(StructPattern* n);
    virtual void visitEnumPattern(EnumPattern* n);
    virtual void visitTypePattern(TypePattern* n)           { (void)n; }
    virtual void visitGuardPattern(GuardPattern* n);

    // ── Dispatch ─────────────────────────────────────────────────────────────
    void visit(ASTNode* node);
    void visit(DeclNode* node);
    void visit(StmtNode* node);
    void visit(ExprNode* node);
    void visit(TypeNode* node);
    void visit(PatternNode* node);
};

} // namespace vex