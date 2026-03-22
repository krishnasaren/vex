#pragma once
// ============================================================================
// vex/Core/DiagnosticIDs.h
// Every diagnostic the compiler can emit has a unique ID here.
// Organized by compiler phase.
// ============================================================================

#include <cstdint>

namespace vex {

enum class DiagID : uint32_t {

    // ── Internal ──────────────────────────────────────────────────────────────
    None = 0,
    InternalError,

    // ── Lexer (LEX_*) ─────────────────────────────────────────────────────────
    LEX_UnknownChar,            // unexpected character in source
    LEX_UnterminatedString,     // string literal not closed
    LEX_UnterminatedChar,       // char literal not closed
    LEX_InvalidCharLiteral,     // char literal has > 1 character
    LEX_UnterminatedComment,    // /* comment not closed
    LEX_InvalidEscape,          // unknown escape sequence \X
    LEX_InvalidUnicodeEscape,   // \u{XXXX} malformed
    LEX_InvalidIntLiteral,      // malformed integer literal
    LEX_InvalidFloatLiteral,    // malformed float literal
    LEX_IntSuffixOverflow,      // literal value doesn't fit in declared type e.g. 300u8
    LEX_InvalidRawString,       // malformed r"..." string

    // ── Parser (PARSE_*) ──────────────────────────────────────────────────────
    PARSE_Expected,             // expected X, got Y
    PARSE_UnexpectedToken,      // token not valid here
    PARSE_UnexpectedEOF,        // file ended unexpectedly
    PARSE_MissingSemicolon,     // (informational — vex doesn't require them but flags oddities)
    PARSE_InvalidDecl,          // malformed declaration
    PARSE_InvalidExpr,          // malformed expression
    PARSE_InvalidType,          // malformed type expression
    PARSE_InvalidPattern,       // malformed match pattern
    PARSE_InvalidImport,        // malformed import statement
    PARSE_ImportHasFunction,    // import std::io::println ← no println.vex
    PARSE_InvalidColonColon,    // :: used outside import statement
    PARSE_MissingArrow,         // fn missing -> return type marker
    PARSE_InvalidGenericParam,  // malformed <T> parameter
    PARSE_TooManyParams,        // too many parameters

    // ── Sema / Name Resolution (SEMA_*) ───────────────────────────────────────
    SEMA_UndefinedName,         // use of undeclared identifier
    SEMA_AmbiguousName,         // name resolves to multiple candidates
    SEMA_UseBeforeInit,         // variable used before initialization
    SEMA_UseAfterMove,          // value used after it was moved
    SEMA_ShadowWarning,         // local shadows outer name (warning)
    SEMA_ModuleNotModule,       // X::y but X is not a module
    SEMA_NotPublic,             // accessing priv/internal symbol from wrong scope
    SEMA_ProtAccessOutside,     // prot field accessed outside class hierarchy
    SEMA_ModuleConflict,        // two imports share same last-segment name
    SEMA_SelectiveConflict,     // selective import name collision — same signature
    SEMA_OpenImportShadow,      // open import { * } shadows existing name (warning)
    SEMA_ModuleNotFound,        // import X::Y — file not found
    SEMA_CircularImport,        // circular import detected
    SEMA_ImportTwice,           // same module imported twice
    SEMA_ColonColonOutsideImport, // io::println but :: not in import ctx

    // ── Type Checker (TYPE_*) ─────────────────────────────────────────────────
    TYPE_Mismatch,              // type A expected, got B
    TYPE_CannotInfer,           // cannot infer type of expression
    TYPE_NullableRequired,      // null used where T? not expected
    TYPE_NullableUnwrap,        // T? used where T expected — must unwrap
    TYPE_IntLiteralTooBig,      // literal doesn't fit in target int type
    TYPE_FloatLiteralContext,   // float used in int context without cast
    TYPE_NarrowingCast,         // implicit narrowing not allowed
    TYPE_ImplicitConversion,    // implicit conversion not allowed
    TYPE_ReturnTypeMismatch,    // return type doesn't match fn signature
    TYPE_VoidReturn,            // returning value from void function
    TYPE_NoReturnValue,         // missing return value in non-void function
    TYPE_MatchArmMismatch,      // match arms have inconsistent types
    TYPE_EmptyCollectionNoCtx,  // [] without type context
    TYPE_LambdaParamUnknown,    // lambda parameter type cannot be inferred
    TYPE_GenericArgConflict,    // conflicting types for generic parameter T
    TYPE_TraitNotImplemented,   // type does not implement required trait
    TYPE_TraitBoundViolated,    // generic instantiation fails trait bound
    TYPE_SelfOutsideMethod,     // `self` used outside a method
    TYPE_SuperOutsideClass,     // `super` used outside a class
    TYPE_ModOutsideModule,      // `mod` used in invalid context
    TYPE_StaticOnInstance,      // calling static method on instance variable
    TYPE_InstanceOnType,        // calling instance method on type name
    TYPE_NotCallable,           // expression is not callable
    TYPE_WrongArgCount,         // wrong number of arguments
    TYPE_AmbiguousOverload,     // overload resolution is ambiguous
    TYPE_NoMatchingOverload,    // no overload matches the argument types
    TYPE_CannotCallAny,         // method called directly on `any` type

    // ── Borrow Checker (BORROW_*) ─────────────────────────────────────────────
    BORROW_UseAfterMove,        // value was moved, cannot use
    BORROW_DoubleMove,          // value moved twice
    BORROW_MovedWhileBorrowed,  // cannot move value — active borrow exists
    BORROW_MutBorrowWhileBorrowed, // cannot borrow mutably — shared borrow exists
    BORROW_SharedWhileMutBorrowed, // cannot borrow — mutable borrow exists
    BORROW_BorrowOutlivesOwner, // borrow lives longer than the owned value
    BORROW_ReturnLocalRef,      // returning reference to local variable

    // ── Control Flow (FLOW_*) ─────────────────────────────────────────────────
    FLOW_NonExhaustiveMatch,    // match doesn't cover all variants
    FLOW_UnreachablePattern,    // match arm can never match
    FLOW_BreakOutsideLoop,      // stop/skip used outside a loop
    FLOW_ContinueOutsideLoop,
    FLOW_YieldOutsideGenerator, // yield used outside fn*
    FLOW_AwaitOutsideAsync,     // await used outside async fn
    FLOW_ReturnInGenerator,     // return value inside fn* (not allowed)
    FLOW_UnreachableCode,       // code after return/panic/never (warning)

    // ── Overflow & Arithmetic (ARITH_*) ───────────────────────────────────────
    ARITH_ConstOverflow,        // comptime overflow detected
    ARITH_ShiftTooLarge,        // shift amount >= bit width
    ARITH_ShiftNegative,        // shift by negative amount
    ARITH_DivByZeroConst,       // division by zero at comptime

    // ── Generics (GEN_*) ──────────────────────────────────────────────────────
    GEN_RecursiveDifferentType, // generic fn recurses with different T
    GEN_ComptimeRecursionLimit, // comptime recursion depth exceeded

    // ── Visibility (VIS_*) ────────────────────────────────────────────────────
    VIS_ProtOnNonClass,         // prot used on non-class item
    VIS_InternalFromOutside,    // internal symbol accessed from outside project dir
    VIS_PrivFromOutside,        // priv symbol accessed from different file

    // ── Compiler/Driver (DRIVER_*) ────────────────────────────────────────────
    DRIVER_UnknownOption,       // unrecognized command-line flag
    DRIVER_NoInput,             // no input files given
    DRIVER_OutputConflict,      // conflicting output flags

    _COUNT
};

// Severity of a diagnostic
enum class DiagSeverity : uint8_t {
    Note,       // informational — no impact on compilation
    Warning,    // issue detected but compilation continues
    Error,      // compilation fails after current phase completes
    Fatal,      // compilation stops immediately
};

// Get the default severity for a DiagID.
DiagSeverity defaultSeverity(DiagID id);

// Human-readable short name for a DiagID (for -Wno-xxx flags etc.)
const char* diagName(DiagID id);

} // namespace vex