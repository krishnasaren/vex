// ============================================================================
// vex/Parser/Precedence.cpp
// ============================================================================

#include "vex/Parser/Precedence.h"

namespace vex {

Prec infixPrecedence(TokenKind kind) {
    switch (kind) {
        // Level 15 — module path
        case TokenKind::ColonColon:
            return Prec::Module;

        // Level 12 — power (right-associative)
        case TokenKind::StarStar:
            return Prec::Power;

        // Level 11 — multiplicative
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent:
        case TokenKind::WrapMul:
        case TokenKind::WrapDiv:
        case TokenKind::SatMul:
            return Prec::Multiplicative;

        // Level 10 — additive
        case TokenKind::Plus:
        case TokenKind::Minus:
        case TokenKind::WrapAdd:
        case TokenKind::WrapSub:
        case TokenKind::SatAdd:
        case TokenKind::SatSub:
            return Prec::Additive;

        // Level 9 — shift
        case TokenKind::LShift:
        case TokenKind::RShift:
        case TokenKind::URShift:
            return Prec::Shift;

        // Level 8 — comparison
        case TokenKind::Less:
        case TokenKind::LessEq:
        case TokenKind::Greater:
        case TokenKind::GreaterEq:
        case TokenKind::KW_is:
        case TokenKind::KW_as:
        case TokenKind::KW_as_q:
            return Prec::Comparison;

        // Level 7 — equality
        case TokenKind::Eq:
        case TokenKind::NotEq:
            return Prec::Equality;

        // Level 6 — bitwise AND
        case TokenKind::Amp:
            return Prec::BitAnd;

        // Level 5 — bitwise XOR
        case TokenKind::Caret:
            return Prec::BitXor;

        // Level 4 — bitwise OR
        case TokenKind::Pipe:
            return Prec::BitOr;

        // Level 3 — logical AND
        case TokenKind::AmpAmp:
        case TokenKind::KW_and:
            return Prec::LogicalAnd;

        // Level 2 — logical OR
        case TokenKind::PipePipe:
        case TokenKind::KW_or:
            return Prec::LogicalOr;

        // Level 1 — null coalesce
        case TokenKind::QuestionQuestion:
            return Prec::NullCoal;

        // Level 0 — pipe operator
        case TokenKind::PipeArrow:
            return Prec::Pipe;

        // Range operators (between additive and comparison)
        case TokenKind::DotDot:
        case TokenKind::DotDotEq:
            return Prec::Additive; // lower than arithmetic, parsed as postfix-like

        // Level -2 — assignment (lowest)
        case TokenKind::Assign:
        case TokenKind::ColonAssign:
        case TokenKind::PlusAssign:
        case TokenKind::MinusAssign:
        case TokenKind::StarAssign:
        case TokenKind::SlashAssign:
        case TokenKind::PercentAssign:
        case TokenKind::StarStarAssign:
        case TokenKind::AmpAssign:
        case TokenKind::PipeAssign:
        case TokenKind::CaretAssign:
        case TokenKind::LShiftAssign:
        case TokenKind::RShiftAssign:
        case TokenKind::WrapAddAssign:
        case TokenKind::WrapSubAssign:
        case TokenKind::WrapMulAssign:
        case TokenKind::SatAddAssign:
        case TokenKind::SatSubAssign:
        case TokenKind::SatMulAssign:
            return Prec::Assignment;

        default:
            return Prec::None;
    }
}

bool isRightAssociative(TokenKind kind) {
    switch (kind) {
        case TokenKind::StarStar:
        case TokenKind::Assign:
        case TokenKind::ColonAssign:
        case TokenKind::PlusAssign:
        case TokenKind::MinusAssign:
        case TokenKind::StarAssign:
        case TokenKind::SlashAssign:
        case TokenKind::PercentAssign:
        case TokenKind::StarStarAssign:
        case TokenKind::AmpAssign:
        case TokenKind::PipeAssign:
        case TokenKind::CaretAssign:
        case TokenKind::LShiftAssign:
        case TokenKind::RShiftAssign:
        case TokenKind::WrapAddAssign:
        case TokenKind::WrapSubAssign:
        case TokenKind::WrapMulAssign:
        case TokenKind::SatAddAssign:
        case TokenKind::SatSubAssign:
        case TokenKind::SatMulAssign:
            return true;
        default:
            return false;
    }
}

Prec rhsPrecedence(TokenKind kind) {
    Prec p = infixPrecedence(kind);
    if (p == Prec::None) return Prec::None;
    if (isRightAssociative(kind))
        return static_cast<Prec>(static_cast<int8_t>(p) - 1);
    return p;
}

bool canStartUnary(TokenKind kind) {
    switch (kind) {
        case TokenKind::Bang:
        case TokenKind::Tilde:
        case TokenKind::Minus:
        case TokenKind::Star:
        case TokenKind::Amp:
        case TokenKind::PlusPlus:
        case TokenKind::MinusMinus:
        case TokenKind::KW_not:
        case TokenKind::KW_move:
        case TokenKind::KW_copy:
        case TokenKind::Hat:
            return true;
        default:
            return false;
    }
}

bool canStartExpr(TokenKind kind) {
    if (canStartUnary(kind)) return true;
    switch (kind) {
        case TokenKind::IntLiteral:
        case TokenKind::FloatLiteral:
        case TokenKind::StringLiteral:
        case TokenKind::RawString:
        case TokenKind::FmtString:
        case TokenKind::MultilineStr:
        case TokenKind::CharLiteral:
        case TokenKind::BoolTrue:
        case TokenKind::BoolFalse:
        case TokenKind::Null:
        case TokenKind::Identifier:
        case TokenKind::KW_self:
        case TokenKind::KW_super:
        case TokenKind::KW_Self:
        case TokenKind::KW_mod:
        case TokenKind::KW_if:
        case TokenKind::KW_match:
        case TokenKind::KW_for:
        case TokenKind::KW_while:
        case TokenKind::KW_loop:
        case TokenKind::KW_do:
        case TokenKind::KW_try:
        case TokenKind::KW_fn:
        case TokenKind::KW_async:
        case TokenKind::KW_unsafe:
        case TokenKind::KW_comptime:
        case TokenKind::LParen:
        case TokenKind::LBracket:
        case TokenKind::LBrace:
        case TokenKind::Pipe:
            return true;
        default:
            return false;
    }
}

} // namespace vex