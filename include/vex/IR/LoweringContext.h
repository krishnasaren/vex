#pragma once
// vex/IR/LoweringContext.h — Context object carried through IRGen
#include "vex/IR/VexIR.h"
#include "vex/AST/Expr.h"
#include "vex/AST/Pattern.h"
#include <string_view>
#include <unordered_map>
#include <stack>
#include <vector>
#include <memory>

namespace vex {

struct LoopFrame {
    IRBlock* continueBlock;
    IRBlock* breakBlock;
};

class LoweringContext {
public:
    void setModule(IRModule* m)             { module_ = m; }
    IRModule* module()           const      { return module_; }
    void setCurrentFunction(IRFunction* fn) { currentFn_ = fn; }
    IRFunction* currentFunction() const     { return currentFn_; }
    void setCurrentBlock(IRBlock* b)        { currentBB_ = b; }
    IRBlock* currentBlock()      const      { return currentBB_; }

    // ── Emit ─────────────────────────────────────────────────────────────────
    template<typename T, typename... Args>
    T* emit(Args&&... args) {
        auto instr = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = instr.get();
        currentBB_->appendInstr(std::move(instr));
        lastVal_ = ptr;
        return ptr;
    }
    IRValue* lastValue() const { return lastVal_; }

    // ── Locals ───────────────────────────────────────────────────────────────
    void defineLocal(std::string_view name, IRValue* val) {
        locals_[std::string(name)] = val;
    }
    IRValue* lookupLocal(std::string_view name) const {
        auto it = locals_.find(std::string(name));
        return it != locals_.end() ? it->second : nullptr;
    }
    IRValue* lookupOrLoad(std::string_view name) {
        auto* val = lookupLocal(name);
        if (!val) return emit<IRConstInstr>(IRConstKind::Undef);
        if (val->kind() == IRValueKind::Instruction) {
            auto* instr = static_cast<IRInstr*>(val);
            if (static_cast<IRInstr*>(instr)->kind() == IRInstrKind::Alloca)
                return emit<IRLoadInstr>(val);
        }
        return val;
    }
    IRValue* lowerLValue(ExprNode* target) {
        if (target && target->is(ASTNodeKind::IdentExpr)) {
            auto* id  = static_cast<IdentExpr*>(target);
            auto* val = lookupLocal(id->name);
            if (val) return val;
        }
        return emit<IRConstInstr>(IRConstKind::Undef);
    }

    // ── Loop stack ────────────────────────────────────────────────────────────
    void pushLoop(IRBlock* cont, IRBlock* brk) { loopStack_.push({cont, brk}); }
    void popLoop()  { if (!loopStack_.empty()) loopStack_.pop(); }
    void emitContinue() {
        if (!loopStack_.empty()) emit<IRBrInstr>(loopStack_.top().continueBlock);
    }
    void emitBreak() {
        if (!loopStack_.empty()) emit<IRBrInstr>(loopStack_.top().breakBlock);
    }

    // ── Defer list ────────────────────────────────────────────────────────────
    void registerDefer(ExprNode* e) { defers_.push_back(e); }
    const std::vector<ExprNode*>& defers() const { return defers_; }

    // ── Error propagation ─────────────────────────────────────────────────────
    IRValue* emitErrorProp(IRValue* val) {
        auto* errBB = currentFn_->createBlock("errprop.err");
        auto* okBB  = currentFn_->createBlock("errprop.ok");
        auto* isOk  = emit<IRCallInstr>("__result_is_ok", std::vector<IRValue*>{val});
        emit<IRCondBrInstr>(isOk, okBB, errBB);
        setCurrentBlock(errBB);
        auto* ev = emit<IRCallInstr>("__result_err", std::vector<IRValue*>{val});
        emit<IRReturnInstr>(ev);
        setCurrentBlock(okBB);
        return emit<IRCallInstr>("__result_ok", std::vector<IRValue*>{val});
    }

    // ── Pattern test ──────────────────────────────────────────────────────────
    IRValue* emitPatternTest(IRValue* subject, PatternNode* pattern) {
        if (!pattern) return emit<IRBoolConstInstr>(true);
        return emit<IRCallInstr>("__pattern_match", std::vector<IRValue*>{subject});
    }

    // ── Lambda ────────────────────────────────────────────────────────────────
    IRValue* emitLambda(LambdaExpr*) {
        return emit<IRCallInstr>("__make_closure", std::vector<IRValue*>{});
    }

private:
    IRModule*   module_    = nullptr;
    IRFunction* currentFn_ = nullptr;
    IRBlock*    currentBB_ = nullptr;
    IRValue*    lastVal_   = nullptr;
    std::unordered_map<std::string, IRValue*> locals_;
    std::stack<LoopFrame>                     loopStack_;
    std::vector<ExprNode*>                    defers_;
};

} // namespace vex
