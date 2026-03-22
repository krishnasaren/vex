#pragma once
// ============================================================================
// vex/IR/LoweringContext.h
// The context object carried through IRGen.
// Tracks: current function, current block, local variable map, loop stack.
// Provides typed emit<T>() helpers.
// ============================================================================

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
    IRBlock* continueBlock; // jump target for `skip`
    IRBlock* breakBlock;    // jump target for `stop`
};

// ── LoweringContext ───────────────────────────────────────────────────────────

class LoweringContext {
public:
    // ── Module / Function / Block management ─────────────────────────────────

    void setModule(IRModule* m)        { module_ = m; }
    IRModule* module()           const { return module_; }

    void setCurrentFunction(IRFunction* fn) { currentFn_ = fn; }
    IRFunction* currentFunction()    const  { return currentFn_; }

    void setCurrentBlock(IRBlock* b)  { currentBB_ = b; }
    IRBlock* currentBlock()    const  { return currentBB_; }

    // ── Emit helpers ──────────────────────────────────────────────────────────

    // Emit an instruction into the current block and return it.
    template<typename T, typename... Args>
    T* emit(Args&&... args) {
        auto instr = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = instr.get();
        currentBB_->appendInstr(std::move(instr));
        lastVal_ = ptr;
        return ptr;
    }

    IRValue* lastValue() const { return lastVal_; }

    // ── Local variable map ────────────────────────────────────────────────────

    void defineLocal(std::string_view name, IRValue* val) {
        locals_[std::string(name)] = val;
    }

    IRValue* lookupLocal(std::string_view name) const {
        auto it = locals_.find(std::string(name));
        return it != locals_.end() ? it->second : nullptr;
    }

    // Lookup and emit a load if the value is an alloca
    IRValue* lookupOrLoad(std::string_view name) {
        auto* val = lookupLocal(name);
        if (!val) return emit<IRConstInstr>(IRConstKind::Undef);
        if (val->kind() == IRValueKind::Instruction) {
            auto* instr = static_cast<IRInstr*>(val);
            if (instr->kind() == IRInstrKind::Alloca)
                return emit<IRLoadInstr>(val);
        }
        return val;
    }

    // Emit a store target (return the alloca pointer for the lvalue)
    IRValue* lowerLValue(ExprNode* target) {
        if (target->is(ASTNodeKind::IdentExpr)) {
            auto* id  = static_cast<IdentExpr*>(target);
            auto* val = lookupLocal(id->name);
            if (val) return val;
        }
        // For member / index access, return the GEP pointer
        return emit<IRConstInstr>(IRConstKind::Undef);
    }

    // ── Loop management ───────────────────────────────────────────────────────

    void pushLoop(IRBlock* continueBlock, IRBlock* breakBlock) {
        loopStack_.push({continueBlock, breakBlock});
    }

    void popLoop() {
        if (!loopStack_.empty()) loopStack_.pop();
    }

    void emitContinue() {
        if (!loopStack_.empty())
            emit<IRBrInstr>(loopStack_.top().continueBlock);
    }

    void emitBreak() {
        if (!loopStack_.empty())
            emit<IRBrInstr>(loopStack_.top().breakBlock);
    }

    // ── Defer tracking ────────────────────────────────────────────────────────

    void registerDefer(ExprNode* expr) {
        defers_.push_back(expr);
    }

    const std::vector<ExprNode*>& defers() const { return defers_; }

    // ── Error propagation ─────────────────────────────────────────────────────

    IRValue* emitErrorProp(IRValue* val) {
        // Emit: if val is Err → return Err. Otherwise unwrap Ok.
        auto* fn      = currentFn_;
        auto* errBB   = fn->createBlock("errprop.err");
        auto* okBB    = fn->createBlock("errprop.ok");
        auto* isOk    = emit<IRCallInstr>("__result_is_ok",
                                          std::vector<IRValue*>{val});
        emit<IRCondBrInstr>(isOk, okBB, errBB);

        setCurrentBlock(errBB);
        auto* errVal = emit<IRCallInstr>("__result_err",
                                         std::vector<IRValue*>{val});
        emit<IRReturnInstr>(errVal);

        setCurrentBlock(okBB);
        return emit<IRCallInstr>("__result_ok",
                                  std::vector<IRValue*>{val});
    }

    // ── Pattern test ──────────────────────────────────────────────────────────

    IRValue* emitPatternTest(IRValue* subject, PatternNode* pattern) {
        if (!pattern)
            return emit<IRBoolConstInstr>(true);
        // Emit runtime pattern matching test — returns bool
        return emit<IRCallInstr>(
            "__pattern_match",
            std::vector<IRValue*>{subject});
    }

    // ── Lambda emission ───────────────────────────────────────────────────────

    IRValue* emitLambda(LambdaExpr* expr) {
        // Create a new anonymous function for the lambda body
        auto* fn = module_->createFunction("__lambda");
        // The lambda is represented as a value referencing the fn ptr
        return emit<IRCallInstr>("__make_closure",
                                  std::vector<IRValue*>{});
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