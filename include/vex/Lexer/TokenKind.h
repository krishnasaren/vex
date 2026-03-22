#pragma once
// ============================================================================
// vex/Lexer/TokenKind.h
// Every possible token in the VEX language.
// Derived directly from Chapter 3 (Lexical Rules) of vex_final_rules.vex.
// ============================================================================

#include <cstdint>
#include <string_view>

namespace vex {

enum class TokenKind : uint16_t {

    // ── Special ───────────────────────────────────────────────────────────────
    Eof = 0,        // end of file
    Error,          // lexer error token (bad character, unterminated literal)
    Newline,        // not emitted by default — used internally for position tracking

    // ── Literals ─────────────────────────────────────────────────────────────
    IntLiteral,     // 42  0xFF  0b1010  0o755  42L  42u8  etc.
    FloatLiteral,   // 3.14  1.5e10  3.14f
    StringLiteral,  // "hello"
    RawString,      // r"no escapes here"
    FmtString,      // f"x={x}"   (interpolated — lexed as segments)
    MultilineStr,   // `raw multiline`
    CharLiteral,    // 'A'  '\n'  '\u{1F600}'
    BoolTrue,       // true
    BoolFalse,      // false
    Null,           // null

    // Duration literals — emitted as IntLiteral with a DurationSuffix flag
    // The suffix is stored in the token's extra data field

    // ── Identifiers ───────────────────────────────────────────────────────────
    Identifier,     // any non-keyword identifier

    // ── Keywords (Chapter 3 — KEYWORDS) ──────────────────────────────────────
    // Declaration
    KW_var,
    KW_let,
    KW_const,
    KW_fn,
    KW_async,
    KW_await,
    KW_yield,

    // Control flow
    KW_if,
    KW_elif,
    KW_else,
    KW_match,
    KW_for,
    KW_while,
    KW_do,
    KW_loop,
    KW_in,
    KW_step,
    KW_skip,        // continue
    KW_stop,        // break
    KW_return,

    // Type declarations
    KW_struct,
    KW_class,
    KW_enum,
    KW_union,
    KW_trait,
    KW_interface,
    KW_impl,
    KW_bitfield,
    KW_sealed,
    KW_abstract,

    // Visibility
    KW_pub,
    KW_priv,
    KW_prot,
    KW_internal,

    // Self/super/Self/mod
    KW_self,        // current instance
    KW_super,       // parent class
    KW_Self,        // current type (capital S)
    KW_mod,         // current module scope (new keyword)

    // Type members
    KW_init,
    KW_deinit,
    KW_static,
    KW_prop,
    KW_op,

    // Logic aliases
    KW_and,         // alias for &&
    KW_or,          // alias for ||
    KW_not,         // alias for !

    // Ownership
    KW_move,
    KW_copy,

    // Unsafe / extern
    KW_unsafe,
    KW_extern,

    // Module system
    KW_import,
    KW_type,
    KW_distinct,

    // Comptime / resource
    KW_comptime,
    KW_defer,
    KW_use,

    // Error handling
    KW_try,
    KW_catch,
    KW_panic,
    KW_assert,
    KW_assertEq,

    // Concurrency
    KW_select,
    KW_timeout,

    // Inline / asm
    KW_asm,
    KW_inline,

    // Generic constraint
    KW_where,

    // Thread local
    KW_thread_local,

    // ── Punctuation & Operators ───────────────────────────────────────────────

    // Delimiters
    LParen,         // (
    RParen,         // )
    LBracket,       // [
    RBracket,       // ]
    LBrace,         // {
    RBrace,         // }
    LAngle,         // <   (disambiguated as generic opener or less-than by parser)
    RAngle,         // >   (disambiguated as generic closer or greater-than by parser)

    // Separators
    Comma,          // ,
    Semicolon,      // ;
    Colon,          // :
    ColonColon,     // ::  (module path separator — only valid in import statements)
    Dot,            // .
    DotDot,         // ..  (exclusive range or struct fill)
    DotDotEq,       // ..= (inclusive range)
    DotDotDot,      // ... (variadic / spread)
    At,             // @  (decorator prefix)
    Hash,           // #  (compile directive)
    Question,       // ?  (nullable type T? or error propagate)
    QuestionDot,    // ?. (safe call)
    QuestionQuestion,// ?? (null coalesce)

    // Assignment
    Assign,         // =
    ColonAssign,    // :=  (infer-and-assign)
    PlusAssign,     // +=
    MinusAssign,    // -=
    StarAssign,     // *=
    SlashAssign,    // /=
    PercentAssign,  // %=
    StarStarAssign, // **=
    AmpAssign,      // &=
    PipeAssign,     // |=
    CaretAssign,    // ^=
    LShiftAssign,   // <<=
    RShiftAssign,   // >>=
    WrapAddAssign,  // +%=
    WrapSubAssign,  // -%=
    WrapMulAssign,  // *%=
    SatAddAssign,   // +|=
    SatSubAssign,   // -|=
    SatMulAssign,   // *|=

    // Arithmetic
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Percent,        // %
    StarStar,       // **  (power)
    WrapAdd,        // +%  (wrapping add)
    WrapSub,        // -%  (wrapping sub)
    WrapMul,        // *%  (wrapping mul)
    WrapDiv,        // /%  (wrapping div)
    SatAdd,         // +|  (saturating add)
    SatSub,         // -|  (saturating sub)
    SatMul,         // *|  (saturating mul)

    // Bitwise
    Amp,            // &   (borrow type, bitwise AND, set intersect)
    Pipe,           // |   (lambda param delimiter, bitwise OR, set union, match OR)
    Caret,          // ^   (bitwise XOR)
    Tilde,          // ~   (bitwise NOT, shared pointer type ~T)
    LShift,         // <<
    RShift,         // >>
    URShift,        // >>>

    // Comparison
    Eq,             // ==
    NotEq,          // !=
    Less,           // <
    LessEq,         // <=
    Greater,        // >
    GreaterEq,      // >=

    // Logical
    AmpAmp,         // &&
    PipePipe,       // ||
    Bang,           // !

    // Increment/Decrement
    PlusPlus,       // ++
    MinusMinus,     // --

    // Arrow operators
    Arrow,          // ->   (fn return type, lambda body)
    FatArrow,       // =>   (match arm separator)

    // Pipe operator
    PipeArrow,      // |>   (pipe operator)

    // Pointer types
    Hat,            // ^   (owned pointer type ^T)

    // Type cast / check
    KW_as,          // as
    KW_as_q,        // as?  (safe cast — lexed as two tokens: as + ?)
    KW_is,          // is

    // ── End sentinel ─────────────────────────────────────────────────────────
    _COUNT
};

// ── Utility functions ─────────────────────────────────────────────────────────

// Human-readable name of a token kind (for error messages)
std::string_view tokenKindName(TokenKind kind);

// True if this token kind is a keyword
bool isKeyword(TokenKind kind);

// True if this token kind is a literal
bool isLiteral(TokenKind kind);

// True if this token kind is an assignment operator
bool isAssignment(TokenKind kind);

// True if this token kind is a binary operator
bool isBinaryOp(TokenKind kind);

// True if this token kind is a unary prefix operator
bool isUnaryPrefixOp(TokenKind kind);

// Map identifier text to keyword token kind (or Identifier if not a keyword)
TokenKind lookupKeyword(std::string_view text);

} // namespace vex