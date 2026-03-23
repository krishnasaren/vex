#pragma once
// include/vex/IR/IRInstrKind.h


// ============================================================================
// vex/IR/IRInstrKind.h  — Enumeration of all VEX IR instruction opcodes
// ============================================================================
#include <cstdint>
#include <string_view>

namespace vex {

enum class IRInstrKind : uint16_t {
    // Constants
    Const,
    Undef,
    Null,

    // Arithmetic
    Add, Sub, Mul, Div, Mod,
    FAdd, FSub, FMul, FDiv,
    Neg, FNeg,

    // Bitwise
    And, Or, Xor, Not,
    Shl, Shr, Ushr,

    // Comparison
    ICmpEq, ICmpNe, ICmpLt, ICmpLe, ICmpGt, ICmpGe,
    UCmpLt, UCmpLe, UCmpGt, UCmpGe,
    FCmpEq, FCmpNe, FCmpLt, FCmpLe, FCmpGt, FCmpGe,

    // Logical
    LogicalAnd, LogicalOr,

    // Memory
    Alloca,
    Load,
    Store,
    GetElementPtr,  // struct/array field access
    MemCopy,
    MemSet,

    // Conversions
    ZExt, SExt, Trunc,
    FPExt, FPTrunc, FPToSI, FPToUI, SIToFP, UIToFP,
    PtrToInt, IntToPtr,
    Bitcast,

    // Control flow
    Br,       // unconditional branch
    CondBr,   // conditional branch
    Switch,
    Ret,
    RetVoid,
    Unreachable,

    // Calls
    Call,
    TailCall,
    IndirectCall,

    // Aggregates
    InsertValue,
    ExtractValue,
    InsertElement,
    ExtractElement,
    ShuffleVector,

    // Atomics
    AtomicRMW,
    AtomicCmpXchg,
    Fence,

    // SIMD
    SIMDSplat,
    SIMDReduce,

    // Special
    Phi,
    Select,
    Poison,
    Freeze,

    // VEX-specific high-level
    Await,        // lowered by AsyncLowering
    Yield,        // lowered by GeneratorLowering
    Defer,        // lowered by DeferLowering
    Try,          // lowered by ErrorPropLowering
    Drop,         // lowered by DropInserter
    Move,         // explicit ownership move
    Clone,        // explicit clone

    Count_
};

std::string_view instrKindName(IRInstrKind k);
bool             isBinaryOp(IRInstrKind k);
bool             isTerminator(IRInstrKind k);
bool             isMemoryOp(IRInstrKind k);

} // namespace vex






