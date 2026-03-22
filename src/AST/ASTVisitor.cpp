// ============================================================================
// vex/AST/ASTVisitor.cpp
// ============================================================================

#include "vex/AST/ASTVisitor.h"

namespace vex {

void ASTVisitor::visit(ASTNode* node) {
    if (!node) return;
    switch (node->kind()) {
        case ASTNodeKind::ModuleDecl:       visitModuleDecl(node->as<ModuleDecl>()); break;
        case ASTNodeKind::ImportDecl:       visitImportDecl(node->as<ImportDecl>()); break;
        case ASTNodeKind::FnDecl:
        case ASTNodeKind::AsyncFnDecl:
        case ASTNodeKind::GeneratorFnDecl:  visitFnDecl(node->as<FnDecl>()); break;
        case ASTNodeKind::InitDecl:         visitInitDecl(node->as<InitDecl>()); break;
        case ASTNodeKind::DeinitDecl:       visitDeinitDecl(node->as<DeinitDecl>()); break;
        case ASTNodeKind::StructDecl:       visitStructDecl(node->as<StructDecl>()); break;
        case ASTNodeKind::ClassDecl:        visitClassDecl(node->as<ClassDecl>()); break;
        case ASTNodeKind::EnumDecl:         visitEnumDecl(node->as<EnumDecl>()); break;
        case ASTNodeKind::EnumVariantDecl:  visitEnumVariantDecl(node->as<EnumVariantDecl>()); break;
        case ASTNodeKind::TraitDecl:        visitTraitDecl(node->as<TraitDecl>()); break;
        case ASTNodeKind::InterfaceDecl:    visitInterfaceDecl(node->as<InterfaceDecl>()); break;
        case ASTNodeKind::ImplDecl:         visitImplDecl(node->as<ImplDecl>()); break;
        case ASTNodeKind::FieldDecl:
        case ASTNodeKind::StaticFieldDecl:  visitFieldDecl(node->as<FieldDecl>()); break;
        case ASTNodeKind::PropDecl:         visitPropDecl(node->as<PropDecl>()); break;
        case ASTNodeKind::OperatorDecl:     visitOperatorDecl(node->as<OperatorDecl>()); break;
        case ASTNodeKind::TypeAliasDecl:    visitTypeAliasDecl(node->as<TypeAliasDecl>()); break;
        case ASTNodeKind::DistinctDecl:     visitDistinctDecl(node->as<DistinctDecl>()); break;
        case ASTNodeKind::VarDecl:          visitVarDecl(node->as<VarDecl>()); break;
        case ASTNodeKind::ConstDecl:        visitConstDecl(node->as<ConstDecl>()); break;
        case ASTNodeKind::BitfieldDecl:     visitBitfieldDecl(node->as<BitfieldDecl>()); break;
        case ASTNodeKind::BlockStmt:        visitBlockStmt(node->as<BlockStmt>()); break;
        case ASTNodeKind::VarDeclStmt:      visitVarDeclStmt(node->as<VarDeclStmt>()); break;
        case ASTNodeKind::ExprStmt:         visitExprStmt(node->as<ExprStmt>()); break;
        case ASTNodeKind::IfStmt:           visitIfStmt(node->as<IfStmt>()); break;
        case ASTNodeKind::WhileStmt:        visitWhileStmt(node->as<WhileStmt>()); break;
        case ASTNodeKind::DoWhileStmt:      visitDoWhileStmt(node->as<DoWhileStmt>()); break;
        case ASTNodeKind::ForStmt:
        case ASTNodeKind::ForRangeStmt:     visitForStmt(node->as<ForStmt>()); break;
        case ASTNodeKind::ForCStmt:         visitForCStmt(node->as<ForCStmt>()); break;
        case ASTNodeKind::LoopStmt:         visitLoopStmt(node->as<LoopStmt>()); break;
        case ASTNodeKind::MatchStmt:        visitMatchStmt(node->as<MatchStmt>()); break;
        case ASTNodeKind::ReturnStmt:       visitReturnStmt(node->as<ReturnStmt>()); break;
        case ASTNodeKind::SkipStmt:         visitSkipStmt(node->as<SkipStmt>()); break;
        case ASTNodeKind::StopStmt:         visitStopStmt(node->as<StopStmt>()); break;
        case ASTNodeKind::DeferStmt:        visitDeferStmt(node->as<DeferStmt>()); break;
        case ASTNodeKind::UseLetStmt:       visitUseLetStmt(node->as<UseLetStmt>()); break;
        case ASTNodeKind::TryCatchStmt:     visitTryCatchStmt(node->as<TryCatchStmt>()); break;
        case ASTNodeKind::SelectStmt:       visitSelectStmt(node->as<SelectStmt>()); break;
        case ASTNodeKind::UnsafeStmt:       visitUnsafeStmt(node->as<UnsafeStmt>()); break;
        case ASTNodeKind::AsmStmt:          visitAsmStmt(node->as<AsmStmt>()); break;
        case ASTNodeKind::LabeledStmt:      visitLabeledStmt(node->as<LabeledStmt>()); break;
        case ASTNodeKind::IntLitExpr:       visitIntLitExpr(node->as<IntLitExpr>()); break;
        case ASTNodeKind::FloatLitExpr:     visitFloatLitExpr(node->as<FloatLitExpr>()); break;
        case ASTNodeKind::StringLitExpr:    visitStringLitExpr(node->as<StringLitExpr>()); break;
        case ASTNodeKind::CharLitExpr:      visitCharLitExpr(node->as<CharLitExpr>()); break;
        case ASTNodeKind::BoolLitExpr:      visitBoolLitExpr(node->as<BoolLitExpr>()); break;
        case ASTNodeKind::NullLitExpr:      visitNullLitExpr(node->as<NullLitExpr>()); break;
        case ASTNodeKind::FmtStringExpr:    visitFmtStringExpr(node->as<FmtStringExpr>()); break;
        case ASTNodeKind::MultilineStrExpr: visitMultilineStrExpr(node->as<MultilineStrExpr>()); break;
        case ASTNodeKind::IdentExpr:        visitIdentExpr(node->as<IdentExpr>()); break;
        case ASTNodeKind::SelfExpr:         visitSelfExpr(node->as<SelfExpr>()); break;
        case ASTNodeKind::SuperExpr:        visitSuperExpr(node->as<SuperExpr>()); break;
        case ASTNodeKind::SelfTypeExpr:     visitSelfTypeExpr(node->as<SelfTypeExpr>()); break;
        case ASTNodeKind::ModExpr:          visitModExpr(node->as<ModExpr>()); break;
        case ASTNodeKind::TupleExpr:        visitTupleExpr(node->as<TupleExpr>()); break;
        case ASTNodeKind::ListExpr:         visitListExpr(node->as<ListExpr>()); break;
        case ASTNodeKind::MapExpr:          visitMapExpr(node->as<MapExpr>()); break;
        case ASTNodeKind::SetExpr:          visitSetExpr(node->as<SetExpr>()); break;
        case ASTNodeKind::StructLitExpr:    visitStructLitExpr(node->as<StructLitExpr>()); break;
        case ASTNodeKind::LambdaExpr:       visitLambdaExpr(node->as<LambdaExpr>()); break;
        case ASTNodeKind::UnaryExpr:        visitUnaryExpr(node->as<UnaryExpr>()); break;
        case ASTNodeKind::BinaryExpr:       visitBinaryExpr(node->as<BinaryExpr>()); break;
        case ASTNodeKind::AssignExpr:       visitAssignExpr(node->as<AssignExpr>()); break;
        case ASTNodeKind::CallExpr:         visitCallExpr(node->as<CallExpr>()); break;
        case ASTNodeKind::IndexExpr:        visitIndexExpr(node->as<IndexExpr>()); break;
        case ASTNodeKind::MemberExpr:       visitMemberExpr(node->as<MemberExpr>()); break;
        case ASTNodeKind::ModulePathExpr:   visitModulePathExpr(node->as<ModulePathExpr>()); break;
        case ASTNodeKind::CastExpr:         visitCastExpr(node->as<CastExpr>()); break;
        case ASTNodeKind::SafeCastExpr:     visitSafeCastExpr(node->as<SafeCastExpr>()); break;
        case ASTNodeKind::TypeCheckExpr:    visitTypeCheckExpr(node->as<TypeCheckExpr>()); break;
        case ASTNodeKind::IfExpr:           visitIfExpr(node->as<IfExpr>()); break;
        case ASTNodeKind::MatchExpr:        visitMatchExpr(node->as<MatchExpr>()); break;
        case ASTNodeKind::MatchArm:         visitMatchArm(node->as<MatchArm>()); break;
        case ASTNodeKind::NullCoalesceExpr: visitNullCoalesceExpr(node->as<NullCoalesceExpr>()); break;
        case ASTNodeKind::SafeCallExpr:     visitSafeCallExpr(node->as<SafeCallExpr>()); break;
        case ASTNodeKind::ErrorPropExpr:    visitErrorPropExpr(node->as<ErrorPropExpr>()); break;
        case ASTNodeKind::PipeExpr:         visitPipeExpr(node->as<PipeExpr>()); break;
        case ASTNodeKind::RangeExpr:        visitRangeExpr(node->as<RangeExpr>()); break;
        case ASTNodeKind::BlockExpr:        visitBlockExpr(node->as<BlockExpr>()); break;
        case ASTNodeKind::ComptimeExpr:     visitComptimeExpr(node->as<ComptimeExpr>()); break;
        case ASTNodeKind::UnsafeExpr:       visitUnsafeExpr(node->as<UnsafeExpr>()); break;
        case ASTNodeKind::GenericInstExpr:  visitGenericInstExpr(node->as<GenericInstExpr>()); break;
        case ASTNodeKind::NamedType:        visitNamedType(node->as<NamedType>()); break;
        case ASTNodeKind::QualifiedType:    visitQualifiedType(node->as<QualifiedType>()); break;
        case ASTNodeKind::NullableType:     visitNullableType(node->as<NullableType>()); break;
        case ASTNodeKind::OwnedPtrType:     visitOwnedPtrType(node->as<OwnedPtrType>()); break;
        case ASTNodeKind::SharedPtrType:    visitSharedPtrType(node->as<SharedPtrType>()); break;
        case ASTNodeKind::BorrowType:       visitBorrowType(node->as<BorrowType>()); break;
        case ASTNodeKind::MutBorrowType:    visitMutBorrowType(node->as<MutBorrowType>()); break;
        case ASTNodeKind::RawPtrType:       visitRawPtrType(node->as<RawPtrType>()); break;
        case ASTNodeKind::SliceType:        visitSliceType(node->as<SliceType>()); break;
        case ASTNodeKind::FixedArrayType:   visitFixedArrayType(node->as<FixedArrayType>()); break;
        case ASTNodeKind::MapType:          visitMapType(node->as<MapType>()); break;
        case ASTNodeKind::SetType:          visitSetType(node->as<SetType>()); break;
        case ASTNodeKind::TupleType:        visitTupleType(node->as<TupleType>()); break;
        case ASTNodeKind::FnType:           visitFnType(node->as<FnType>()); break;
        case ASTNodeKind::GenericType:      visitGenericType(node->as<GenericType>()); break;
        case ASTNodeKind::InferType:        visitInferType(node->as<InferType>()); break;
        case ASTNodeKind::WildcardPattern:  visitWildcardPattern(node->as<WildcardPattern>()); break;
        case ASTNodeKind::BindingPattern:   visitBindingPattern(node->as<BindingPattern>()); break;
        case ASTNodeKind::LiteralPattern:   visitLiteralPattern(node->as<LiteralPattern>()); break;
        case ASTNodeKind::RangePattern:     visitRangePattern(node->as<RangePattern>()); break;
        case ASTNodeKind::OrPattern:        visitOrPattern(node->as<OrPattern>()); break;
        case ASTNodeKind::TuplePattern:     visitTuplePattern(node->as<TuplePattern>()); break;
        case ASTNodeKind::StructPattern:    visitStructPattern(node->as<StructPattern>()); break;
        case ASTNodeKind::EnumPattern:      visitEnumPattern(node->as<EnumPattern>()); break;
        case ASTNodeKind::TypePattern:      visitTypePattern(node->as<TypePattern>()); break;
        case ASTNodeKind::GuardPattern:     visitGuardPattern(node->as<GuardPattern>()); break;
        default: break;
    }
}
void ASTVisitor::visit(DeclNode* n)    { visit(static_cast<ASTNode*>(n)); }
void ASTVisitor::visit(StmtNode* n)    { visit(static_cast<ASTNode*>(n)); }
void ASTVisitor::visit(ExprNode* n)    { visit(static_cast<ASTNode*>(n)); }
void ASTVisitor::visit(TypeNode* n)    { visit(static_cast<ASTNode*>(n)); }
void ASTVisitor::visit(PatternNode* n) { visit(static_cast<ASTNode*>(n)); }

// Default walk implementations
void ASTVisitor::visitModuleDecl(ModuleDecl* n)   { for (auto* d : n->imports) visit(d); for (auto* d : n->decls) visit(d); }
void ASTVisitor::visitFnDecl(FnDecl* n)           { for (auto* p : n->params) { if (p->type) visit(p->type); } if (n->returnType) visit(n->returnType); if (n->body) visit(n->body); }
void ASTVisitor::visitInitDecl(InitDecl* n)       { for (auto* p : n->params) { if (p->type) visit(p->type); } if (n->body) visit(n->body); }
void ASTVisitor::visitDeinitDecl(DeinitDecl* n)   { if (n->body) visit(n->body); }
void ASTVisitor::visitStructDecl(StructDecl* n)   { for (auto* m : n->members) visit(m); }
void ASTVisitor::visitClassDecl(ClassDecl* n)     { for (auto* m : n->members) visit(m); }
void ASTVisitor::visitEnumDecl(EnumDecl* n)       { for (auto* v : n->variants) visit(v); for (auto* m : n->methods) visit(m); }
void ASTVisitor::visitTraitDecl(TraitDecl* n)     { for (auto* m : n->members) visit(m); }
void ASTVisitor::visitInterfaceDecl(InterfaceDecl* n) { for (auto* m : n->methods) visit(m); }
void ASTVisitor::visitImplDecl(ImplDecl* n)       { for (auto* m : n->members) visit(m); }
void ASTVisitor::visitOperatorDecl(OperatorDecl* n) { if (n->returnType) visit(n->returnType); if (n->body) visit(n->body); if (n->bodyExpr) visit(n->bodyExpr); }
void ASTVisitor::visitBlockStmt(BlockStmt* n)     { for (auto* s : n->stmts) visit(s); if (n->trailingExpr) visit(n->trailingExpr); }
void ASTVisitor::visitVarDeclStmt(VarDeclStmt* n) { for (auto& b : n->bindings) { if (b.type) visit(b.type); if (b.initializer) visit(b.initializer); } }
void ASTVisitor::visitExprStmt(ExprStmt* n)       { if (n->expr) visit(n->expr); }
void ASTVisitor::visitIfStmt(IfStmt* n)           { for (auto& br : n->branches) { if (br.condition) visit(br.condition); if (br.body) visit(br.body); } if (n->elseBranch) visit(n->elseBranch); }
void ASTVisitor::visitWhileStmt(WhileStmt* n)     { if (n->condition) visit(n->condition); if (n->body) visit(n->body); }
void ASTVisitor::visitDoWhileStmt(DoWhileStmt* n) { if (n->body) visit(n->body); if (n->condition) visit(n->condition); }
void ASTVisitor::visitForStmt(ForStmt* n)         { if (n->iterable) visit(n->iterable); if (n->stepExpr) visit(n->stepExpr); if (n->body) visit(n->body); }
void ASTVisitor::visitForCStmt(ForCStmt* n)       { if (n->init) visit(n->init); if (n->condition) visit(n->condition); if (n->step) visit(n->step); if (n->body) visit(n->body); }
void ASTVisitor::visitLoopStmt(LoopStmt* n)       { if (n->body) visit(n->body); }
void ASTVisitor::visitMatchStmt(MatchStmt* n)     { if (n->subject) visit(n->subject); for (auto* a : n->arms) visit(a); }
void ASTVisitor::visitReturnStmt(ReturnStmt* n)   { if (n->value) visit(n->value); }
void ASTVisitor::visitDeferStmt(DeferStmt* n)     { if (n->expr) visit(n->expr); }
void ASTVisitor::visitUseLetStmt(UseLetStmt* n)   { if (n->resource) visit(n->resource); if (n->body) visit(n->body); }
void ASTVisitor::visitTryCatchStmt(TryCatchStmt* n) { if (n->tryBlock) visit(n->tryBlock); if (n->catchBlock) visit(n->catchBlock); }
void ASTVisitor::visitSelectStmt(SelectStmt* n)   { for (auto& a : n->arms) { if (a.chanExpr) visit(a.chanExpr); if (a.timeoutDur) visit(a.timeoutDur); if (a.body) visit(a.body); } }
void ASTVisitor::visitUnsafeStmt(UnsafeStmt* n)   { if (n->body) visit(n->body); }
void ASTVisitor::visitLabeledStmt(LabeledStmt* n) { if (n->inner) visit(n->inner); }
void ASTVisitor::visitFmtStringExpr(FmtStringExpr* n) { for (auto& s : n->segments) if (s.expr) visit(s.expr); }
void ASTVisitor::visitTupleExpr(TupleExpr* n)     { for (auto* e : n->elements) visit(e); }
void ASTVisitor::visitListExpr(ListExpr* n)       { for (auto* e : n->elements) visit(e); }
void ASTVisitor::visitMapExpr(MapExpr* n)         { for (auto& e : n->entries) { visit(e.key); visit(e.value); } }
void ASTVisitor::visitSetExpr(SetExpr* n)         { for (auto* e : n->elements) visit(e); }
void ASTVisitor::visitStructLitExpr(StructLitExpr* n) { if (n->typeName) visit(n->typeName); for (auto& f : n->fields) visit(f.value); if (n->spreadFrom) visit(n->spreadFrom); }
void ASTVisitor::visitLambdaExpr(LambdaExpr* n)   { for (auto& p : n->params) if (p.type) visit(p.type); if (n->returnType) visit(n->returnType); if (n->bodyExpr) visit(n->bodyExpr); if (n->bodyBlock) visit(n->bodyBlock); }
void ASTVisitor::visitUnaryExpr(UnaryExpr* n)     { if (n->operand) visit(n->operand); }
void ASTVisitor::visitBinaryExpr(BinaryExpr* n)   { visit(n->left); visit(n->right); }
void ASTVisitor::visitAssignExpr(AssignExpr* n)   { visit(n->target); visit(n->value); }
void ASTVisitor::visitCallExpr(CallExpr* n)       { if (n->callee) visit(n->callee); for (auto& a : n->args) visit(a.value); for (auto* t : n->typeArgs) visit(t); }
void ASTVisitor::visitIndexExpr(IndexExpr* n)     { visit(n->base); visit(n->index); }
void ASTVisitor::visitMemberExpr(MemberExpr* n)   { if (n->base) visit(n->base); }
void ASTVisitor::visitCastExpr(CastExpr* n)       { visit(n->expr); visit(n->targetType); }
void ASTVisitor::visitSafeCastExpr(SafeCastExpr* n) { visit(n->expr); visit(n->targetType); }
void ASTVisitor::visitTypeCheckExpr(TypeCheckExpr* n) { visit(n->expr); visit(n->checkType); }
void ASTVisitor::visitIfExpr(IfExpr* n)           { if (n->condition) visit(n->condition); if (n->thenBranch) visit(n->thenBranch); if (n->elseBranch) visit(n->elseBranch); }
void ASTVisitor::visitMatchExpr(MatchExpr* n)     { if (n->subject) visit(n->subject); for (auto* a : n->arms) visit(a); }
void ASTVisitor::visitMatchArm(MatchArm* n)       { if (n->pattern) visit(n->pattern); if (n->guard) visit(n->guard); if (n->bodyExpr) visit(n->bodyExpr); if (n->bodyBlock) visit(n->bodyBlock); }
void ASTVisitor::visitNullCoalesceExpr(NullCoalesceExpr* n) { visit(n->left); visit(n->right); }
void ASTVisitor::visitSafeCallExpr(SafeCallExpr* n) { if (n->base) visit(n->base); for (auto& a : n->args) visit(a.value); }
void ASTVisitor::visitErrorPropExpr(ErrorPropExpr* n) { if (n->inner) visit(n->inner); }
void ASTVisitor::visitPipeExpr(PipeExpr* n)       { visit(n->left); visit(n->right); }
void ASTVisitor::visitRangeExpr(RangeExpr* n)     { visit(n->start); visit(n->end); }
void ASTVisitor::visitBlockExpr(BlockExpr* n)     { if (n->block) visit(n->block); }
void ASTVisitor::visitComptimeExpr(ComptimeExpr* n) { if (n->inner) visit(n->inner); }
void ASTVisitor::visitUnsafeExpr(UnsafeExpr* n)   { if (n->inner) visit(n->inner); }
void ASTVisitor::visitGenericInstExpr(GenericInstExpr* n) { if (n->base) visit(n->base); for (auto* t : n->typeArgs) visit(t); for (auto& a : n->args) visit(a.value); }
void ASTVisitor::visitNullableType(NullableType* n) { if (n->inner) visit(n->inner); }
void ASTVisitor::visitOwnedPtrType(OwnedPtrType* n) { if (n->pointee) visit(n->pointee); }
void ASTVisitor::visitSharedPtrType(SharedPtrType* n) { if (n->pointee) visit(n->pointee); }
void ASTVisitor::visitBorrowType(BorrowType* n)   { if (n->pointee) visit(n->pointee); }
void ASTVisitor::visitMutBorrowType(MutBorrowType* n) { if (n->pointee) visit(n->pointee); }
void ASTVisitor::visitRawPtrType(RawPtrType* n)   { if (n->pointee) visit(n->pointee); }
void ASTVisitor::visitSliceType(SliceType* n)     { if (n->elem) visit(n->elem); }
void ASTVisitor::visitFixedArrayType(FixedArrayType* n) { if (n->size) visit(n->size); if (n->elem) visit(n->elem); }
void ASTVisitor::visitMapType(MapType* n)         { visit(n->key); visit(n->val); }
void ASTVisitor::visitSetType(SetType* n)         { if (n->elem) visit(n->elem); }
void ASTVisitor::visitTupleType(TupleType* n)     { for (auto* e : n->elements) visit(e); }
void ASTVisitor::visitFnType(FnType* n)           { for (auto* p : n->params) visit(p); if (n->returnType) visit(n->returnType); }
void ASTVisitor::visitGenericType(GenericType* n) { if (n->base) visit(n->base); for (auto* a : n->args) visit(a); }
void ASTVisitor::visitLiteralPattern(LiteralPattern* n) { if (n->literal) visit(n->literal); }
void ASTVisitor::visitRangePattern(RangePattern* n) { visit(n->lo); visit(n->hi); }
void ASTVisitor::visitOrPattern(OrPattern* n)     { for (auto* a : n->alternatives) visit(a); }
void ASTVisitor::visitTuplePattern(TuplePattern* n) { for (auto* e : n->elements) visit(e); }
void ASTVisitor::visitStructPattern(StructPattern* n) { for (auto& f : n->fields) if (f.pattern) visit(f.pattern); }
void ASTVisitor::visitEnumPattern(EnumPattern* n) { for (auto& f : n->fields) if (f.pattern) visit(f.pattern); }
void ASTVisitor::visitGuardPattern(GuardPattern* n) { if (n->inner) visit(n->inner); if (n->guard) visit(n->guard); }

} // namespace vex