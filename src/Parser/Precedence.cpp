// src/Parser/Precedence.cpp — Token precedence and classification
#include "vex/Parser/Precedence.h"
#include "vex/Lexer/TokenKind.h"

namespace vex {

Prec infixPrecedence(TokenKind k) {
    switch (k) {
    case TokenKind::Assign:
    case TokenKind::ColonAssign:
    case TokenKind::PlusAssign:   case TokenKind::MinusAssign:
    case TokenKind::StarAssign:   case TokenKind::SlashAssign:
    case TokenKind::PercentAssign:case TokenKind::AmpAssign:
    case TokenKind::PipeAssign:   case TokenKind::CaretAssign:
    case TokenKind::LShiftAssign: case TokenKind::RShiftAssign:
    case TokenKind::WrapAddAssign:case TokenKind::WrapSubAssign:
    case TokenKind::WrapMulAssign:case TokenKind::SatAddAssign:
    case TokenKind::SatSubAssign: case TokenKind::SatMulAssign:
    case TokenKind::StarStarAssign:   return Prec::Assignment;
    case TokenKind::Question:         return Prec::ErrorProp;
    case TokenKind::PipeArrow:        return Prec::Pipe;
    case TokenKind::QuestionQuestion: return Prec::NullCoal;
    case TokenKind::PipePipe:
    case TokenKind::KW_or:            return Prec::LogicalOr;
    case TokenKind::AmpAmp:
    case TokenKind::KW_and:           return Prec::LogicalAnd;
    case TokenKind::Pipe:             return Prec::BitOr;
    case TokenKind::Caret:            return Prec::BitXor;
    case TokenKind::Amp:              return Prec::BitAnd;
    case TokenKind::Eq:  case TokenKind::NotEq:
                                      return Prec::Equality;
    case TokenKind::Less: case TokenKind::LessEq:
    case TokenKind::Greater: case TokenKind::GreaterEq:
    case TokenKind::KW_is:
    case TokenKind::KW_as: case TokenKind::KW_as_q:
                                      return Prec::Comparison;
    case TokenKind::LShift: case TokenKind::RShift:
    case TokenKind::URShift:          return Prec::Shift;
    case TokenKind::Plus:   case TokenKind::Minus:
    case TokenKind::WrapAdd:case TokenKind::WrapSub:
    case TokenKind::SatAdd: case TokenKind::SatSub:
    case TokenKind::DotDot: case TokenKind::DotDotEq:
                                      return Prec::Additive;
    case TokenKind::Star:   case TokenKind::Slash:
    case TokenKind::Percent:
    case TokenKind::WrapMul:case TokenKind::WrapDiv:
    case TokenKind::SatMul:           return Prec::Multiplicative;
    case TokenKind::StarStar:         return Prec::Power;
    default:                          return Prec::None;
    }
}

Prec rhsPrecedence(TokenKind k) {
    Prec p = infixPrecedence(k);
    if (p == Prec::None) return Prec::None;
    // Right-associative operators: use p-1 so the right side is parsed
    if (isRightAssociative(k))
        return static_cast<Prec>(static_cast<int>(p) - 1);
    return p;
}

bool isRightAssociative(TokenKind k) {
    switch (k) {
    case TokenKind::Assign:
    case TokenKind::ColonAssign:
    case TokenKind::PlusAssign:   case TokenKind::MinusAssign:
    case TokenKind::StarAssign:   case TokenKind::SlashAssign:
    case TokenKind::PercentAssign:case TokenKind::AmpAssign:
    case TokenKind::PipeAssign:   case TokenKind::CaretAssign:
    case TokenKind::LShiftAssign: case TokenKind::RShiftAssign:
    case TokenKind::WrapAddAssign:case TokenKind::WrapSubAssign:
    case TokenKind::WrapMulAssign:case TokenKind::SatAddAssign:
    case TokenKind::SatSubAssign: case TokenKind::SatMulAssign:
    case TokenKind::StarStarAssign:
    case TokenKind::StarStar:
        return true;
    default: return false;
    }
}

bool isAssignment(TokenKind k) {
    return infixPrecedence(k) == Prec::Assignment;
}

bool canStartUnary(TokenKind k) {
    switch (k) {
    case TokenKind::Minus:  case TokenKind::Bang:
    case TokenKind::Tilde:  case TokenKind::Amp:
    case TokenKind::Star:   case TokenKind::PlusPlus:
    case TokenKind::MinusMinus:
    case TokenKind::KW_not: case TokenKind::KW_move:
    case TokenKind::KW_copy:case TokenKind::DotDotDot:
        return true;
    default: return false;
    }
}

bool canStartExpr(TokenKind k) {
    if (canStartUnary(k)) return true;
    switch (k) {
    case TokenKind::IntLiteral:   case TokenKind::FloatLiteral:
    case TokenKind::StringLiteral:case TokenKind::RawString:
    case TokenKind::FmtString:    case TokenKind::CharLiteral:
    case TokenKind::BoolTrue:     case TokenKind::BoolFalse:
    case TokenKind::Null:         case TokenKind::Identifier:
    case TokenKind::LParen:       case TokenKind::LBracket:
    case TokenKind::LBrace:
    case TokenKind::KW_if:        case TokenKind::KW_match:
    case TokenKind::KW_loop:      case TokenKind::KW_while:
    case TokenKind::KW_for:       case TokenKind::KW_fn:
    case TokenKind::KW_async:     case TokenKind::KW_await:
    case TokenKind::KW_yield:     case TokenKind::KW_try:
    case TokenKind::KW_self:      case TokenKind::KW_super:
    case TokenKind::KW_comptime:  case TokenKind::KW_unsafe:
    case TokenKind::ColonColon:
        return true;
    default: return false;
    }
}

bool isLiteral(TokenKind k) {
    switch (k) {
    case TokenKind::IntLiteral:   case TokenKind::FloatLiteral:
    case TokenKind::StringLiteral:case TokenKind::RawString:
    case TokenKind::FmtString:    case TokenKind::CharLiteral:
    case TokenKind::BoolTrue:     case TokenKind::BoolFalse:
    case TokenKind::Null:
        return true;
    default: return false;
    }
}

bool isKeyword(TokenKind k) {
    return k >= TokenKind::KW_var && k <= TokenKind::KW_is;
}

bool canStartType(TokenKind k) {
    switch (k) {
    case TokenKind::Identifier:  case TokenKind::ColonColon:
    case TokenKind::Amp:         case TokenKind::Star:
    case TokenKind::LBracket:    case TokenKind::LParen:
    case TokenKind::Question:    case TokenKind::KW_fn:
    case TokenKind::KW_async:    case TokenKind::Tilde:
    case TokenKind::Hash:
        return true;
    default: return isKeyword(k);
    }
}

bool canStartPattern(TokenKind k) {
    switch (k) {
    case TokenKind::Identifier:   case TokenKind::IntLiteral:
    case TokenKind::FloatLiteral: case TokenKind::StringLiteral:
    case TokenKind::CharLiteral:  case TokenKind::BoolTrue:
    case TokenKind::BoolFalse:    case TokenKind::Null:
    case TokenKind::LParen:       case TokenKind::LBracket:
    case TokenKind::LBrace:       case TokenKind::Minus:
    case TokenKind::ColonColon:   case TokenKind::DotDot:
    case TokenKind::DotDotDot:
        return true;
    default: return false;
    }
}

bool isPostfixOp(TokenKind k) {
    switch (k) {
    case TokenKind::PlusPlus:  case TokenKind::MinusMinus:
    case TokenKind::Question:  case TokenKind::Bang:
        return true;
    default: return false;
    }
}

} // namespace vex
