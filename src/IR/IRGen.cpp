// src/IR/IRGen.cpp
// ============================================================================
// vex/IR/IRGen.cpp
// Translates the type-checked AST into VexIR (a high-level SSA IR).
// The IRGen visits each ModuleDecl and emits IRFunction objects.
// ============================================================================

#include "vex/IR/IRGen.h"
#include "vex/IR/IRModule.h"
#include "vex/IR/IRFunction.h"
#include "vex/IR/IRBlock.h"
#include "vex/IR/IRInstr.h"
#include "vex/IR/IRType.h"
#include "vex/IR/IRValue.h"
#include "vex/IR/LoweringContext.h"
#include "vex/AST/Decl.h"
#include "vex/AST/Stmt.h"
#include "vex/AST/Expr.h"
#include "vex/Core/Assert.h"
#include "vex/Core/DiagnosticEngine.h"

namespace vex {

// ── IRGen ─────────────────────────────────────────────────────────────────────

std::unique_ptr<IRModule> IRGen::generate(ModuleDecl* mod) {
    VEX_ASSERT(mod != nullptr, "IRGen::generate() called with null ModuleDecl");

    auto irMod = std::make_unique<IRModule>(mod->name);
    ctx_.setModule(irMod.get());

    for (auto* decl : mod->decls) {
        if (!decl) continue;
        lowerTopLevelDecl(decl);
        if (diags_.shouldAbort()) break;
    }

    return irMod;
}

void IRGen::lowerTopLevelDecl(DeclNode* decl) {
    switch (decl->kind()) {
        case ASTNodeKind::FnDecl:
        case ASTNodeKind::AsyncFnDecl:
        case ASTNodeKind::GeneratorFnDecl:
            lowerFnDecl(static_cast<FnDecl*>(decl));
            break;
        case ASTNodeKind::StructDecl:
            lowerStructDecl(static_cast<StructDecl*>(decl));
            break;
        case ASTNodeKind::ClassDecl:
            lowerClassDecl(static_cast<ClassDecl*>(decl));
            break;
        case ASTNodeKind::EnumDecl:
            lowerEnumDecl(static_cast<EnumDecl*>(decl));
            break;
        case ASTNodeKind::ImplDecl:
            lowerImplDecl(static_cast<ImplDecl*>(decl));
            break;
        case ASTNodeKind::VarDecl:
            lowerGlobalVarDecl(static_cast<VarDecl*>(decl));
            break;
        case ASTNodeKind::ConstDecl:
            lowerConstDecl(static_cast<ConstDecl*>(decl));
            break;
        default:
            // Type aliases, distinct types, imports etc. handled by sema
            break;
    }
}

void IRGen::lowerFnDecl(FnDecl* fn) {
    if (!fn->body) return; // abstract/extern — no body

    auto* irFn = ctx_.module()->createFunction(fn->name);
    irFn->setAsync(fn->isAsync);
    irFn->setGenerator(fn->isGenerator);

    // Entry block
    auto* entry = irFn->createBlock("entry");
    ctx_.setCurrentFunction(irFn);
    ctx_.setCurrentBlock(entry);

    // Emit parameters
    for (auto* param : fn->params) {
        if (!param) continue;
        auto* val = irFn->addParam(param->name);
        ctx_.defineLocal(param->name, val);
    }

    // Lower body
    lowerBlock(fn->body);

    // Ensure terminator
    if (!ctx_.currentBlock()->hasTerminator())
        ctx_.emit<IRReturnInstr>(nullptr);

    ctx_.setCurrentFunction(nullptr);
}

void IRGen::lowerStructDecl(StructDecl* decl) {
    ctx_.module()->declareType(decl->name);
    for (auto* member : decl->members) {
        if (!member) continue;
        if (member->is(ASTNodeKind::FnDecl))
            lowerFnDecl(static_cast<FnDecl*>(member));
    }
}

void IRGen::lowerClassDecl(ClassDecl* decl) {
    ctx_.module()->declareType(decl->name);
    for (auto* member : decl->members) {
        if (!member) continue;
        if (member->is(ASTNodeKind::FnDecl))
            lowerFnDecl(static_cast<FnDecl*>(member));
    }
}

void IRGen::lowerEnumDecl(EnumDecl* decl) {
    ctx_.module()->declareType(decl->name);
    for (auto* method : decl->methods) {
        if (method) lowerFnDecl(method);
    }
}

void IRGen::lowerImplDecl(ImplDecl* decl) {
    for (auto* member : decl->members) {
        if (!member) continue;
        if (member->is(ASTNodeKind::FnDecl))
            lowerFnDecl(static_cast<FnDecl*>(member));
    }
}

void IRGen::lowerGlobalVarDecl(VarDecl* decl) {
    auto* global = ctx_.module()->createGlobal(decl->name, decl->isMutable);
    if (decl->initializer) {
        auto* initVal = lowerExpr(decl->initializer);
        global->setInitializer(initVal);
    }
}

void IRGen::lowerConstDecl(ConstDecl* decl) {
    auto* global = ctx_.module()->createGlobal(decl->name, /*mutable=*/false);
    if (decl->value) {
        auto* initVal = lowerExpr(decl->value);
        global->setInitializer(initVal);
    }
}

// ── Statement lowering ────────────────────────────────────────────────────────

void IRGen::lowerBlock(BlockStmt* block) {
    if (!block) return;
    for (auto* stmt : block->stmts) {
        if (!stmt) continue;
        lowerStmt(stmt);
        if (diags_.shouldAbort()) return;
    }
    if (block->trailingExpr)
        lowerExpr(block->trailingExpr);
}

void IRGen::lowerStmt(StmtNode* stmt) {
    switch (stmt->kind()) {
        case ASTNodeKind::VarDeclStmt:
            lowerVarDeclStmt(static_cast<VarDeclStmt*>(stmt));
            break;
        case ASTNodeKind::ExprStmt:
            lowerExpr(static_cast<ExprStmt*>(stmt)->expr);
            break;
        case ASTNodeKind::BlockStmt:
            lowerBlock(static_cast<BlockStmt*>(stmt));
            break;
        case ASTNodeKind::IfStmt:
            lowerIfStmt(static_cast<IfStmt*>(stmt));
            break;
        case ASTNodeKind::WhileStmt:
            lowerWhileStmt(static_cast<WhileStmt*>(stmt));
            break;
        case ASTNodeKind::ForStmt:
        case ASTNodeKind::ForRangeStmt:
            lowerForStmt(static_cast<ForStmt*>(stmt));
            break;
        case ASTNodeKind::LoopStmt:
            lowerLoopStmt(static_cast<LoopStmt*>(stmt));
            break;
        case ASTNodeKind::ReturnStmt:
            lowerReturnStmt(static_cast<ReturnStmt*>(stmt));
            break;
        case ASTNodeKind::SkipStmt:
            ctx_.emitContinue();
            break;
        case ASTNodeKind::StopStmt:
            ctx_.emitBreak();
            break;
        case ASTNodeKind::DeferStmt:
            lowerDeferStmt(static_cast<DeferStmt*>(stmt));
            break;
        case ASTNodeKind::UnsafeStmt:
            lowerBlock(static_cast<UnsafeStmt*>(stmt)->body);
            break;
        case ASTNodeKind::MatchStmt:
            lowerMatchStmt(static_cast<MatchStmt*>(stmt));
            break;
        default:
            // Stmts handled by later lowering passes (defer, select, etc.)
            break;
    }
}

void IRGen::lowerVarDeclStmt(VarDeclStmt* stmt) {
    for (auto& binding : stmt->bindings) {
        IRValue* initVal = nullptr;
        if (binding.initializer)
            initVal = lowerExpr(binding.initializer);
        auto* alloca = ctx_.emit<IRAllocaInstr>(binding.name);
        if (initVal)
            ctx_.emit<IRStoreInstr>(alloca, initVal);
        ctx_.defineLocal(binding.name, alloca);
    }
}

void IRGen::lowerIfStmt(IfStmt* stmt) {
    auto* fn   = ctx_.currentFunction();
    auto* merge = fn->createBlock("if.merge");

    for (size_t i = 0; i < stmt->branches.size(); ++i) {
        auto& branch = stmt->branches[i];
        auto* condVal = lowerExpr(branch.condition);

        auto* thenBB  = fn->createBlock("if.then");
        auto* nextBB  = (i + 1 < stmt->branches.size() || stmt->elseBranch)
                          ? fn->createBlock("if.elif")
                          : merge;

        ctx_.emit<IRCondBrInstr>(condVal, thenBB, nextBB);

        ctx_.setCurrentBlock(thenBB);
        lowerBlock(branch.body);
        if (!ctx_.currentBlock()->hasTerminator())
            ctx_.emit<IRBrInstr>(merge);

        ctx_.setCurrentBlock(nextBB);
    }

    if (stmt->elseBranch) {
        lowerBlock(stmt->elseBranch);
        if (!ctx_.currentBlock()->hasTerminator())
            ctx_.emit<IRBrInstr>(merge);
        ctx_.setCurrentBlock(merge);
    } else {
        // nextBB is already merge in the last iteration
    }
}

void IRGen::lowerWhileStmt(WhileStmt* stmt) {
    auto* fn    = ctx_.currentFunction();
    auto* condBB = fn->createBlock("while.cond");
    auto* bodyBB = fn->createBlock("while.body");
    auto* exitBB = fn->createBlock("while.exit");

    ctx_.emit<IRBrInstr>(condBB);
    ctx_.setCurrentBlock(condBB);

    auto* condVal = lowerExpr(stmt->condition);
    ctx_.emit<IRCondBrInstr>(condVal, bodyBB, exitBB);

    ctx_.setCurrentBlock(bodyBB);
    ctx_.pushLoop(condBB, exitBB);
    lowerBlock(stmt->body);
    ctx_.popLoop();
    if (!ctx_.currentBlock()->hasTerminator())
        ctx_.emit<IRBrInstr>(condBB);

    ctx_.setCurrentBlock(exitBB);
}

void IRGen::lowerForStmt(ForStmt* stmt) {
    auto* fn      = ctx_.currentFunction();
    auto* iterBB  = fn->createBlock("for.iter");
    auto* bodyBB  = fn->createBlock("for.body");
    auto* exitBB  = fn->createBlock("for.exit");

    // Evaluate iterable once
    auto* iterVal = lowerExpr(stmt->iterable);
    // Iterator variable alloca
    auto* iterAlloca = ctx_.emit<IRAllocaInstr>("__iter");
    ctx_.emit<IRStoreInstr>(iterAlloca, iterVal);

    ctx_.emit<IRBrInstr>(iterBB);
    ctx_.setCurrentBlock(iterBB);

    // Call .next() on iterator
    auto* nextVal = ctx_.emit<IRCallInstr>("__iter_next", std::vector<IRValue*>{iterAlloca});
    // Check for null (exhausted)
    auto* nullVal = ctx_.emit<IRConstInstr>(IRConstKind::Null);
    auto* cond    = ctx_.emit<IRCmpInstr>(IRCmpOp::NEq, nextVal, nullVal);
    ctx_.emit<IRCondBrInstr>(cond, bodyBB, exitBB);

    ctx_.setCurrentBlock(bodyBB);
    ctx_.defineLocal(stmt->valueVar, nextVal);
    if (!stmt->indexVar.empty()) {
        // TODO: enumerate index tracking
    }
    ctx_.pushLoop(iterBB, exitBB);
    lowerBlock(stmt->body);
    ctx_.popLoop();
    if (!ctx_.currentBlock()->hasTerminator())
        ctx_.emit<IRBrInstr>(iterBB);

    ctx_.setCurrentBlock(exitBB);
}

void IRGen::lowerLoopStmt(LoopStmt* stmt) {
    auto* fn      = ctx_.currentFunction();
    auto* bodyBB  = fn->createBlock("loop.body");
    auto* exitBB  = fn->createBlock("loop.exit");

    ctx_.emit<IRBrInstr>(bodyBB);
    ctx_.setCurrentBlock(bodyBB);

    ctx_.pushLoop(bodyBB, exitBB);
    lowerBlock(stmt->body);
    ctx_.popLoop();

    if (!ctx_.currentBlock()->hasTerminator())
        ctx_.emit<IRBrInstr>(bodyBB);

    ctx_.setCurrentBlock(exitBB);
}

void IRGen::lowerReturnStmt(ReturnStmt* stmt) {
    IRValue* retVal = nullptr;
    if (stmt->value)
        retVal = lowerExpr(stmt->value);
    ctx_.emit<IRReturnInstr>(retVal);
}

void IRGen::lowerDeferStmt(DeferStmt* stmt) {
    ctx_.registerDefer(stmt->expr);
}

void IRGen::lowerMatchStmt(MatchStmt* stmt) {
    auto* subject = lowerExpr(stmt->subject);
    auto* fn      = ctx_.currentFunction();
    auto* exitBB  = fn->createBlock("match.exit");

    for (auto* arm : stmt->arms) {
        auto* armBB  = fn->createBlock("match.arm");
        auto* skipBB = fn->createBlock("match.skip");

        // Pattern test
        auto* matches = ctx_.emitPatternTest(subject, arm->pattern);
        ctx_.emit<IRCondBrInstr>(matches, armBB, skipBB);

        ctx_.setCurrentBlock(armBB);
        if (arm->bodyBlock)  lowerBlock(arm->bodyBlock);
        else if (arm->bodyExpr) lowerExpr(arm->bodyExpr);
        if (!ctx_.currentBlock()->hasTerminator())
            ctx_.emit<IRBrInstr>(exitBB);

        ctx_.setCurrentBlock(skipBB);
    }

    // Fall through to exit (for exhaustiveness, sema ensures all cases covered)
    ctx_.emit<IRBrInstr>(exitBB);
    ctx_.setCurrentBlock(exitBB);
}

// ── Expression lowering ───────────────────────────────────────────────────────

IRValue* IRGen::lowerExpr(ExprNode* expr) {
    if (!expr) return ctx_.emit<IRConstInstr>(IRConstKind::Void);

    switch (expr->kind()) {
        case ASTNodeKind::IntLitExpr: {
            auto* e = static_cast<IntLitExpr*>(expr);
            return ctx_.emit<IRIntConstInstr>(e->value, e->suffix, e->base);
        }
        case ASTNodeKind::FloatLitExpr: {
            auto* e = static_cast<FloatLitExpr*>(expr);
            return ctx_.emit<IRFloatConstInstr>(e->value, e->suffix);
        }
        case ASTNodeKind::StringLitExpr: {
            auto* e = static_cast<StringLitExpr*>(expr);
            return ctx_.emit<IRStringConstInstr>(e->value);
        }
        case ASTNodeKind::CharLitExpr: {
            auto* e = static_cast<CharLitExpr*>(expr);
            return ctx_.emit<IRIntConstInstr>(e->codepoint,
                                              IntSuffix::u32, 10);
        }
        case ASTNodeKind::BoolLitExpr: {
            auto* e = static_cast<BoolLitExpr*>(expr);
            return ctx_.emit<IRBoolConstInstr>(e->value);
        }
        case ASTNodeKind::NullLitExpr:
            return ctx_.emit<IRConstInstr>(IRConstKind::Null);

        case ASTNodeKind::IdentExpr: {
            auto* e = static_cast<IdentExpr*>(expr);
            return ctx_.lookupOrLoad(e->name);
        }

        case ASTNodeKind::BinaryExpr: {
            auto* e = static_cast<BinaryExpr*>(expr);
            auto* lhs = lowerExpr(e->left);
            auto* rhs = lowerExpr(e->right);
            return ctx_.emit<IRBinOpInstr>(e->op, lhs, rhs);
        }

        case ASTNodeKind::UnaryExpr: {
            auto* e = static_cast<UnaryExpr*>(expr);
            auto* operand = lowerExpr(e->operand);
            return ctx_.emit<IRUnaryOpInstr>(e->op, operand, e->isPostfix);
        }

        case ASTNodeKind::AssignExpr: {
            auto* e = static_cast<AssignExpr*>(expr);
            auto* rhs = lowerExpr(e->value);
            auto* target = ctx_.lowerLValue(e->target);
            ctx_.emit<IRStoreInstr>(target, rhs);
            return rhs;
        }

        case ASTNodeKind::CallExpr: {
            auto* e = static_cast<CallExpr*>(expr);
            auto* callee = lowerExpr(e->callee);
            std::vector<IRValue*> args;
            for (auto& arg : e->args)
                args.push_back(lowerExpr(arg.value));
            return ctx_.emit<IRCallInstr>(callee, args);
        }

        case ASTNodeKind::MemberExpr: {
            auto* e = static_cast<MemberExpr*>(expr);
            auto* base = lowerExpr(e->base);
            return ctx_.emit<IRGepInstr>(base, e->member);
        }

        case ASTNodeKind::IndexExpr: {
            auto* e = static_cast<IndexExpr*>(expr);
            auto* base = lowerExpr(e->base);
            auto* idx  = lowerExpr(e->index);
            return ctx_.emit<IRIndexInstr>(base, idx);
        }

        case ASTNodeKind::IfExpr: {
            auto* e = static_cast<IfExpr*>(expr);
            auto* cond = lowerExpr(e->condition);
            auto* fn   = ctx_.currentFunction();
            auto* thenBB  = fn->createBlock("ifexpr.then");
            auto* elseBB  = fn->createBlock("ifexpr.else");
            auto* mergeBB = fn->createBlock("ifexpr.merge");
            ctx_.emit<IRCondBrInstr>(cond, thenBB, elseBB);

            ctx_.setCurrentBlock(thenBB);
            auto* thenVal = lowerExpr(e->thenBranch);
            ctx_.emit<IRBrInstr>(mergeBB);
            auto* thenEnd = ctx_.currentBlock();

            ctx_.setCurrentBlock(elseBB);
            IRValue* elseVal = nullptr;
            if (e->elseBranch) elseVal = lowerExpr(e->elseBranch);
            ctx_.emit<IRBrInstr>(mergeBB);
            auto* elseEnd = ctx_.currentBlock();

            ctx_.setCurrentBlock(mergeBB);
            return ctx_.emit<IRPhiInstr>(
                std::vector<std::pair<IRValue*, IRBlock*>>{{thenVal, thenEnd},
                                                           {elseVal, elseEnd}});
        }

        case ASTNodeKind::ErrorPropExpr: {
            auto* e = static_cast<ErrorPropExpr*>(expr);
            auto* inner = lowerExpr(e->inner);
            return ctx_.emitErrorProp(inner);
        }

        case ASTNodeKind::CastExpr: {
            auto* e = static_cast<CastExpr*>(expr);
            auto* val = lowerExpr(e->expr);
            return ctx_.emit<IRCastInstr>(val, e->targetType);
        }

        case ASTNodeKind::BlockExpr: {
            auto* e = static_cast<BlockExpr*>(expr);
            lowerBlock(e->block);
            return e->block->trailingExpr
                ? ctx_.lastValue()
                : ctx_.emit<IRConstInstr>(IRConstKind::Void);
        }

        case ASTNodeKind::RangeExpr: {
            auto* e   = static_cast<RangeExpr*>(expr);
            auto* lo  = lowerExpr(e->start);
            auto* hi  = lowerExpr(e->end);
            return ctx_.emit<IRRangeInstr>(lo, hi, e->inclusive);
        }

        case ASTNodeKind::NullCoalesceExpr: {
            auto* e   = static_cast<NullCoalesceExpr*>(expr);
            auto* lhs = lowerExpr(e->left);
            auto* rhs = lowerExpr(e->right);
            return ctx_.emit<IRNullCoalesceInstr>(lhs, rhs);
        }

        case ASTNodeKind::TupleExpr: {
            auto* e = static_cast<TupleExpr*>(expr);
            std::vector<IRValue*> elems;
            for (auto* el : e->elements)
                elems.push_back(lowerExpr(el));
            return ctx_.emit<IRTupleInstr>(elems);
        }

        case ASTNodeKind::ListExpr: {
            auto* e = static_cast<ListExpr*>(expr);
            std::vector<IRValue*> elems;
            for (auto* el : e->elements)
                elems.push_back(lowerExpr(el));
            return ctx_.emit<IRListInstr>(elems);
        }

        case ASTNodeKind::LambdaExpr: {
            auto* e = static_cast<LambdaExpr*>(expr);
            return ctx_.emitLambda(e);
        }

        case ASTNodeKind::SelfExpr:
            return ctx_.lookupOrLoad("self");

        case ASTNodeKind::SuperExpr:
            return ctx_.lookupOrLoad("super");

        default:
            // Complex expressions handled by specialized lowering passes
            return ctx_.emit<IRConstInstr>(IRConstKind::Void);
    }
}

} // namespace vex