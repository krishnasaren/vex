#pragma once
// ============================================================================
// vex/Parser/Precedence.h
// Operator precedence levels matching Chapter 2 of vex_final_rules.vex.
// Used by the Pratt parser for expression parsing.
// ============================================================================

#include "vex/Lexer/TokenKind.h"
#include <cstdint>

namespace vex {

// Precedence levels — exact values from Chapter 2 precedence table.
// Higher number = binds tighter.
// Negative values for postfix/assignment operators.

enum class Prec : int8_t {
    None       = -3,  // not an operator
    Assignment = -2,  // = += -= etc.  (right-associative, lowest)
    ErrorProp  = -1,  // ? (postfix error propagate)
    Pipe       =  0,  // |>
    NullCoal   =  1,  // ??
    LogicalOr  =  2,  // || or
    LogicalAnd =  3,  // && and
    BitOr      =  4,  // |
    BitXor     =  5,  // ^
    BitAnd     =  6,  // &
    Equality   =  7,  // == !=
    Comparison =  8,  // < > <= >= is as as?
    Shift      =  9,  // << >> >>>
    Additive   = 10,  // + - +% -% +| -|
    Multiplicative = 11,  // * / % *% /% *|
    Power      = 12,  // ** (right-associative)
    Unary      = 13,  // ! ~ - * & ++ -- move (prefix)
    Postfix    = 14,  // () [] . ++ --
    Module     = 15,  // ::  (highest — module path)
};

// Returns the infix precedence for a token (when it appears between two expressions).
// Returns Prec::None if the token is not an infix operator.
Prec infixPrecedence(TokenKind kind);

// Returns the right-hand precedence for a right-associative operator.
// For left-associative: same as infixPrecedence.
// For right-associative (** and assignments): infixPrecedence - 1.
Prec rhsPrecedence(TokenKind kind);

// True if the operator is right-associative.
bool isRightAssociative(TokenKind kind);

// True if this token can start a unary prefix expression.
bool canStartUnary(TokenKind kind);

// True if this token can start a primary expression.
bool canStartExpr(TokenKind kind);

} // namespace vex