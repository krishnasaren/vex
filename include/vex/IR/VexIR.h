#pragma once
// include/vex/IR/VexIR.h


// ============================================================================
// vex/IR/VexIR.h
// Core VexIR data structures: Module, Function, Block, Instructions, Values.
// ============================================================================

#include "vex/Lexer/TokenKind.h"
#include "vex/Lexer/Token.h"
#include "vex/AST/ASTNode.h"
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>
#include <ostream>
#include <cstdint>

namespace vex {

// Forward declarations
class IRBlock;
class IRFunction;
class IRModule;
class IRInstr;
class TypeNode;

// ── IRValueKind ───────────────────────────────────────────────────────────────

enum class IRValueKind : uint8_t {
    Instruction,
    Param,
    Global,
    Const,
    Block,   // used as value when blocks are first-class
};

// ── IRValue ───────────────────────────────────────────────────────────────────
// Base class for all IR values (instructions, params, globals, constants).

class IRValue {
public:
    explicit IRValue(IRValueKind kind, std::string_view name = {});
    virtual ~IRValue() = default;

    IRValueKind      kind() const { return kind_; }
    uint64_t         id()   const { return id_; }
    std::string_view name() const { return name_; }
    std::string      debugName() const;

    void setName(std::string_view n) { name_ = std::string(n); }

private:
    IRValueKind kind_;
    std::string name_;
    uint64_t    id_;
    static uint64_t nextId_;
};

// ── IRParam ───────────────────────────────────────────────────────────────────

class IRParam final : public IRValue {
public:
    explicit IRParam(std::string_view name)
        : IRValue(IRValueKind::Param, name) {}
};

// ── IRGlobal ──────────────────────────────────────────────────────────────────

class IRGlobal final : public IRValue {
public:
    IRGlobal(std::string_view name, bool mutable_);

    bool    isMutable()    const { return mutable_; }
    IRValue* initializer() const { return initializer_; }
    void    setInitializer(IRValue* v) { initializer_ = v; }

private:
    bool     mutable_ = true;
    IRValue* initializer_ = nullptr;
};

// ── IRInstrKind ───────────────────────────────────────────────────────────────

enum class IRInstrKind : uint16_t {
    // Terminators
    Return,
    Br,
    CondBr,
    Unreachable,

    // Memory
    Alloca,
    Store,
    Load,

    // Arithmetic / logic
    BinOp,
    UnaryOp,
    Cmp,

    // Calls
    Call,

    // Access
    Gep,        // field access
    Index,      // array/list index

    // Control / SSA
    Phi,

    // Constants
    IntConst,
    FloatConst,
    BoolConst,
    StringConst,
    Const,      // null / void / undef

    // Type operations
    Cast,
    Range,
    NullCoalesce,
    Tuple,
    List,
    Map,
    Lambda,
    ErrorProp,
    PatternTest,
};

// ── IRCmpOp ───────────────────────────────────────────────────────────────────

enum class IRCmpOp : uint8_t { Eq, NEq, Lt, Lte, Gt, Gte };

// ── IRConstKind ───────────────────────────────────────────────────────────────

enum class IRConstKind : uint8_t { Null, Void, Undef };

// ── IRInstr ───────────────────────────────────────────────────────────────────

class IRInstr : public IRValue {
public:
    explicit IRInstr(IRInstrKind k)
        : IRValue(IRValueKind::Instruction), instrKind_(k) {}

    IRInstrKind kind()   const { return instrKind_; }
    IRBlock*    parent() const { return parent_; }
    void        setParent(IRBlock* b);

    virtual void dump(std::ostream& os) const = 0;

private:
    IRInstrKind instrKind_;
    IRBlock*    parent_ = nullptr;
};

// ── Concrete instructions ─────────────────────────────────────────────────────

class IRReturnInstr final : public IRInstr {
public:
    explicit IRReturnInstr(IRValue* val = nullptr)
        : IRInstr(IRInstrKind::Return), value_(val) {}
    IRValue* value() const { return value_; }
    void dump(std::ostream& os) const override;
private:
    IRValue* value_;
};

class IRBrInstr final : public IRInstr {
public:
    explicit IRBrInstr(IRBlock* target)
        : IRInstr(IRInstrKind::Br), target_(target) {}
    IRBlock* target() const { return target_; }
    void dump(std::ostream& os) const override;
private:
    IRBlock* target_;
};

class IRCondBrInstr final : public IRInstr {
public:
    IRCondBrInstr(IRValue* cond, IRBlock* thenBB, IRBlock* elseBB)
        : IRInstr(IRInstrKind::CondBr)
        , cond_(cond), thenBB_(thenBB), elseBB_(elseBB) {}
    IRValue* cond()   const { return cond_; }
    IRBlock* thenBB() const { return thenBB_; }
    IRBlock* elseBB() const { return elseBB_; }
    void dump(std::ostream& os) const override;
private:
    IRValue* cond_;
    IRBlock* thenBB_;
    IRBlock* elseBB_;
};

class IRAllocaInstr final : public IRInstr {
public:
    explicit IRAllocaInstr(std::string_view name)
        : IRInstr(IRInstrKind::Alloca), name_(name) {}
    std::string_view varName() const { return name_; }
    void dump(std::ostream& os) const override;
private:
    std::string name_;
};

class IRStoreInstr final : public IRInstr {
public:
    IRStoreInstr(IRValue* ptr, IRValue* val)
        : IRInstr(IRInstrKind::Store), ptr_(ptr), val_(val) {}
    IRValue* ptr() const { return ptr_; }
    IRValue* val() const { return val_; }
    void dump(std::ostream& os) const override;
private:
    IRValue* ptr_;
    IRValue* val_;
};

class IRLoadInstr final : public IRInstr {
public:
    explicit IRLoadInstr(IRValue* ptr)
        : IRInstr(IRInstrKind::Load), ptr_(ptr) {}
    IRValue* ptr() const { return ptr_; }
    void dump(std::ostream& os) const override;
private:
    IRValue* ptr_;
};

class IRCallInstr final : public IRInstr {
public:
    IRCallInstr(std::string_view callee, std::vector<IRValue*> args);
    IRCallInstr(IRValue* callee, std::vector<IRValue*> args);

    bool        hasNamedCallee() const { return calleeVal_ == nullptr; }
    std::string_view calleeName()  const { return calleeName_; }
    IRValue*    calleeVal()       const { return calleeVal_; }
    const std::vector<IRValue*>& args() const { return args_; }
    void dump(std::ostream& os) const override;
private:
    std::string          calleeName_;
    IRValue*             calleeVal_ = nullptr;
    std::vector<IRValue*> args_;
};

class IRBinOpInstr final : public IRInstr {
public:
    IRBinOpInstr(TokenKind op, IRValue* lhs, IRValue* rhs)
        : IRInstr(IRInstrKind::BinOp), op_(op), lhs_(lhs), rhs_(rhs) {}
    TokenKind op()  const { return op_; }
    IRValue*  lhs() const { return lhs_; }
    IRValue*  rhs() const { return rhs_; }
    void dump(std::ostream& os) const override;
private:
    TokenKind op_;
    IRValue*  lhs_;
    IRValue*  rhs_;
};

class IRUnaryOpInstr final : public IRInstr {
public:
    IRUnaryOpInstr(TokenKind op, IRValue* operand, bool postfix)
        : IRInstr(IRInstrKind::UnaryOp)
        , op_(op), operand_(operand), postfix_(postfix) {}
    TokenKind op()      const { return op_; }
    IRValue*  operand() const { return operand_; }
    bool      isPostfix() const { return postfix_; }
    void dump(std::ostream& os) const override;
private:
    TokenKind op_;
    IRValue*  operand_;
    bool      postfix_;
};

class IRCmpInstr final : public IRInstr {
public:
    IRCmpInstr(IRCmpOp op, IRValue* lhs, IRValue* rhs)
        : IRInstr(IRInstrKind::Cmp), op_(op), lhs_(lhs), rhs_(rhs) {}
    IRCmpOp  op()  const { return op_; }
    IRValue* lhs() const { return lhs_; }
    IRValue* rhs() const { return rhs_; }
    void dump(std::ostream& os) const override;
private:
    IRCmpOp  op_;
    IRValue* lhs_;
    IRValue* rhs_;
};

class IRGepInstr final : public IRInstr {
public:
    IRGepInstr(IRValue* base, std::string_view field)
        : IRInstr(IRInstrKind::Gep), base_(base), field_(field) {}
    IRValue*         base()  const { return base_; }
    std::string_view field() const { return field_; }
    void dump(std::ostream& os) const override;
private:
    IRValue*    base_;
    std::string field_;
};

class IRIndexInstr final : public IRInstr {
public:
    IRIndexInstr(IRValue* base, IRValue* idx)
        : IRInstr(IRInstrKind::Index), base_(base), idx_(idx) {}
    IRValue* base() const { return base_; }
    IRValue* idx()  const { return idx_; }
    void dump(std::ostream& os) const override;
private:
    IRValue* base_;
    IRValue* idx_;
};

class IRPhiInstr final : public IRInstr {
public:
    using IncomingPair = std::pair<IRValue*, IRBlock*>;
    explicit IRPhiInstr(std::vector<IncomingPair> incoming)
        : IRInstr(IRInstrKind::Phi), incoming_(std::move(incoming)) {}
    const std::vector<IncomingPair>& incoming() const { return incoming_; }
    void dump(std::ostream& os) const override;
private:
    std::vector<IncomingPair> incoming_;
};

class IRIntConstInstr final : public IRInstr {
public:
    IRIntConstInstr(uint64_t val, IntSuffix suffix, uint8_t base)
        : IRInstr(IRInstrKind::IntConst)
        , value_(val), suffix_(suffix), base_(base) {}
    uint64_t   value()  const { return value_; }
    IntSuffix  suffix() const { return suffix_; }
    uint8_t    base()   const { return base_; }
    void dump(std::ostream& os) const override;
private:
    uint64_t  value_;
    IntSuffix suffix_;
    uint8_t   base_;
};

class IRFloatConstInstr final : public IRInstr {
public:
    IRFloatConstInstr(double val, FloatSuffix suffix)
        : IRInstr(IRInstrKind::FloatConst)
        , value_(val), suffix_(suffix) {}
    double      value()  const { return value_; }
    FloatSuffix suffix() const { return suffix_; }
    void dump(std::ostream& os) const override;
private:
    double      value_;
    FloatSuffix suffix_;
};

class IRBoolConstInstr final : public IRInstr {
public:
    explicit IRBoolConstInstr(bool val)
        : IRInstr(IRInstrKind::BoolConst), value_(val) {}
    bool value() const { return value_; }
    void dump(std::ostream& os) const override;
private:
    bool value_;
};

class IRStringConstInstr final : public IRInstr {
public:
    explicit IRStringConstInstr(std::string_view val)
        : IRInstr(IRInstrKind::StringConst), value_(val) {}
    std::string_view value() const { return value_; }
    void dump(std::ostream& os) const override;
private:
    std::string value_;
};

class IRConstInstr final : public IRInstr {
public:
    explicit IRConstInstr(IRConstKind k)
        : IRInstr(IRInstrKind::Const), kind_(k) {}
    IRConstKind constKind() const { return kind_; }
    void dump(std::ostream& os) const override;
private:
    IRConstKind kind_;
};

class IRCastInstr final : public IRInstr {
public:
    IRCastInstr(IRValue* val, TypeNode* targetType)
        : IRInstr(IRInstrKind::Cast)
        , val_(val), targetType_(targetType) {}
    IRValue*  val()        const { return val_; }
    TypeNode* targetType() const { return targetType_; }
    void dump(std::ostream& os) const override;
private:
    IRValue*  val_;
    TypeNode* targetType_;
};

class IRRangeInstr final : public IRInstr {
public:
    IRRangeInstr(IRValue* lo, IRValue* hi, bool inclusive)
        : IRInstr(IRInstrKind::Range)
        , lo_(lo), hi_(hi), inclusive_(inclusive) {}
    IRValue* lo()        const { return lo_; }
    IRValue* hi()        const { return hi_; }
    bool     inclusive() const { return inclusive_; }
    void dump(std::ostream& os) const override;
private:
    IRValue* lo_;
    IRValue* hi_;
    bool     inclusive_;
};

class IRNullCoalesceInstr final : public IRInstr {
public:
    IRNullCoalesceInstr(IRValue* lhs, IRValue* rhs)
        : IRInstr(IRInstrKind::NullCoalesce), lhs_(lhs), rhs_(rhs) {}
    IRValue* lhs() const { return lhs_; }
    IRValue* rhs() const { return rhs_; }
    void dump(std::ostream& os) const override;
private:
    IRValue* lhs_;
    IRValue* rhs_;
};

class IRTupleInstr final : public IRInstr {
public:
    explicit IRTupleInstr(std::vector<IRValue*> elems)
        : IRInstr(IRInstrKind::Tuple), elems_(std::move(elems)) {}
    const std::vector<IRValue*>& elems() const { return elems_; }
    void dump(std::ostream& os) const override;
private:
    std::vector<IRValue*> elems_;
};

class IRListInstr final : public IRInstr {
public:
    explicit IRListInstr(std::vector<IRValue*> elems)
        : IRInstr(IRInstrKind::List), elems_(std::move(elems)) {}
    const std::vector<IRValue*>& elems() const { return elems_; }
    void dump(std::ostream& os) const override;
private:
    std::vector<IRValue*> elems_;
};

// ── IRBlock ───────────────────────────────────────────────────────────────────

class IRBlock {
public:
    IRBlock(std::string_view label, IRFunction* parent);

    std::string_view label()    const { return label_; }
    IRFunction*      parent()   const { return parent_; }
    bool             hasTerminator() const;

    void appendInstr(std::unique_ptr<IRInstr> instr);
    const std::vector<std::unique_ptr<IRInstr>>& instrs() const { return instrs_; }

    void dump(std::ostream& os) const;

private:
    std::string                           label_;
    IRFunction*                           parent_;
    std::vector<std::unique_ptr<IRInstr>> instrs_;
};

// ── IRFunction ────────────────────────────────────────────────────────────────

class IRFunction {
public:
    IRFunction(std::string_view name, IRModule* parent);

    std::string_view name()   const { return name_; }
    IRModule*        parent() const { return parent_; }

    IRBlock* createBlock(std::string_view label);
    IRValue* addParam(std::string_view name);
    IRBlock* entryBlock() const;

    bool isAsync()     const { return async_; }
    bool isGenerator() const { return generator_; }
    void setAsync(bool v)     { async_     = v; }
    void setGenerator(bool v) { generator_ = v; }

    const std::vector<std::unique_ptr<IRBlock>>& blocks() const { return blocks_; }
    const std::vector<std::unique_ptr<IRParam>>& params() const { return params_; }

    void dump(std::ostream& os) const;

private:
    std::string                            name_;
    IRModule*                              parent_;
    std::vector<std::unique_ptr<IRBlock>>  blocks_;
    std::vector<std::unique_ptr<IRParam>>  params_;
    bool                                   async_     = false;
    bool                                   generator_ = false;
};

// ── IRModule ──────────────────────────────────────────────────────────────────

class IRModule {
public:
    explicit IRModule(std::string_view name);

    std::string_view name() const { return name_; }

    IRFunction* createFunction(std::string_view name);
    IRGlobal*   createGlobal(std::string_view name, bool mutable_);
    void        declareType(std::string_view name);
    IRFunction* findFunction(std::string_view name) const;

    const std::vector<std::unique_ptr<IRFunction>>& functions() const { return functions_; }
    const std::vector<std::unique_ptr<IRGlobal>>&   globals()   const { return globals_; }

    void dump(std::ostream& os) const;

private:
    std::string                              name_;
    std::vector<std::unique_ptr<IRFunction>> functions_;
    std::vector<std::unique_ptr<IRGlobal>>   globals_;
    std::vector<std::string>                 typeNames_;
    std::unordered_map<std::string, IRFunction*> fnMap_;
};

} // namespace vex