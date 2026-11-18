// src/Lexer/TokenKind.cpp
#include "vex/Lexer/TokenKind.h"
namespace vex {

std::string_view tokenKindName(TokenKind k) {
    switch (k) {
    case TokenKind::Eof:            return "end of file";
    case TokenKind::Error:          return "<error>";
    case TokenKind::Newline:        return "newline";
    case TokenKind::IntLiteral:     return "integer literal";
    case TokenKind::FloatLiteral:   return "float literal";
    case TokenKind::StringLiteral:  return "string literal";
    case TokenKind::RawString:      return "raw string";
    case TokenKind::FmtString:      return "format string";
    case TokenKind::MultilineStr:   return "multiline string";
    case TokenKind::CharLiteral:    return "char literal";
    case TokenKind::BoolTrue:       return "'true'";
    case TokenKind::BoolFalse:      return "'false'";
    case TokenKind::Null:           return "'null'";
    case TokenKind::Identifier:     return "identifier";
    // Keywords
    case TokenKind::KW_var:         return "'var'";
    case TokenKind::KW_let:         return "'let'";
    case TokenKind::KW_const:       return "'const'";
    case TokenKind::KW_fn:          return "'fn'";
    case TokenKind::KW_async:       return "'async'";
    case TokenKind::KW_await:       return "'await'";
    case TokenKind::KW_yield:       return "'yield'";
    case TokenKind::KW_if:          return "'if'";
    case TokenKind::KW_elif:        return "'elif'";
    case TokenKind::KW_else:        return "'else'";
    case TokenKind::KW_match:       return "'match'";
    case TokenKind::KW_for:         return "'for'";
    case TokenKind::KW_while:       return "'while'";
    case TokenKind::KW_do:          return "'do'";
    case TokenKind::KW_loop:        return "'loop'";
    case TokenKind::KW_in:          return "'in'";
    case TokenKind::KW_step:        return "'step'";
    case TokenKind::KW_skip:        return "'skip'";
    case TokenKind::KW_stop:        return "'stop'";
    case TokenKind::KW_return:      return "'return'";
    case TokenKind::KW_struct:      return "'struct'";
    case TokenKind::KW_class:       return "'class'";
    case TokenKind::KW_enum:        return "'enum'";
    case TokenKind::KW_union:       return "'union'";
    case TokenKind::KW_trait:       return "'trait'";
    case TokenKind::KW_interface:   return "'interface'";
    case TokenKind::KW_impl:        return "'impl'";
    case TokenKind::KW_bitfield:    return "'bitfield'";
    case TokenKind::KW_sealed:      return "'sealed'";
    case TokenKind::KW_abstract:    return "'abstract'";
    case TokenKind::KW_pub:         return "'pub'";
    case TokenKind::KW_priv:        return "'priv'";
    case TokenKind::KW_prot:        return "'prot'";
    case TokenKind::KW_internal:    return "'internal'";
    case TokenKind::KW_self:        return "'self'";
    case TokenKind::KW_super:       return "'super'";
    case TokenKind::KW_Self:        return "'Self'";
    case TokenKind::KW_mod:         return "'mod'";
    case TokenKind::KW_init:        return "'init'";
    case TokenKind::KW_deinit:      return "'deinit'";
    case TokenKind::KW_static:      return "'static'";
    case TokenKind::KW_prop:        return "'prop'";
    case TokenKind::KW_op:          return "'op'";
    case TokenKind::KW_and:         return "'and'";
    case TokenKind::KW_or:          return "'or'";
    case TokenKind::KW_not:         return "'not'";
    case TokenKind::KW_move:        return "'move'";
    case TokenKind::KW_copy:        return "'copy'";
    case TokenKind::KW_unsafe:      return "'unsafe'";
    case TokenKind::KW_extern:      return "'extern'";
    case TokenKind::KW_import:      return "'import'";
    case TokenKind::KW_type:        return "'type'";
    case TokenKind::KW_distinct:    return "'distinct'";
    case TokenKind::KW_comptime:    return "'comptime'";
    case TokenKind::KW_defer:       return "'defer'";
    case TokenKind::KW_use:         return "'use'";
    case TokenKind::KW_try:         return "'try'";
    case TokenKind::KW_catch:       return "'catch'";
    case TokenKind::KW_panic:       return "'panic'";
    case TokenKind::KW_assert:      return "'assert'";
    case TokenKind::KW_assertEq:    return "'assertEq'";
    case TokenKind::KW_select:      return "'select'";
    case TokenKind::KW_timeout:     return "'timeout'";
    case TokenKind::KW_asm:         return "'asm'";
    case TokenKind::KW_inline:      return "'inline'";
    case TokenKind::KW_where:       return "'where'";
    case TokenKind::KW_thread_local:return "'thread_local'";
    case TokenKind::KW_as:          return "'as'";
    case TokenKind::KW_as_q:        return "'as?'";
    case TokenKind::KW_is:          return "'is'";
    // Delimiters
    case TokenKind::LParen:         return "'('";
    case TokenKind::RParen:         return "')'";
    case TokenKind::LBracket:       return "'['";
    case TokenKind::RBracket:       return "']'";
    case TokenKind::LBrace:         return "'{'";
    case TokenKind::RBrace:         return "'}'";
    case TokenKind::LAngle:         return "'<'";
    case TokenKind::RAngle:         return "'>'";
    case TokenKind::Comma:          return "','";
    case TokenKind::Semicolon:      return "';'";
    case TokenKind::Colon:          return "':'";
    case TokenKind::ColonColon:     return "'::'";
    case TokenKind::Dot:            return "'.'";
    case TokenKind::DotDot:         return "'..'";
    case TokenKind::DotDotEq:       return "'..='";
    case TokenKind::DotDotDot:      return "'...'";
    case TokenKind::At:             return "'@'";
    case TokenKind::Hash:           return "'#'";
    case TokenKind::Question:       return "'?'";
    case TokenKind::QuestionDot:    return "'?.'";
    case TokenKind::QuestionQuestion: return "'??'";
    // Assignment
    case TokenKind::Assign:         return "'='";
    case TokenKind::ColonAssign:    return "':='";
    case TokenKind::PlusAssign:     return "'+='";
    case TokenKind::MinusAssign:    return "'-='";
    case TokenKind::StarAssign:     return "'*='";
    case TokenKind::SlashAssign:    return "'/='";
    case TokenKind::PercentAssign:  return "'%='";
    case TokenKind::StarStarAssign: return "'**='";
    case TokenKind::AmpAssign:      return "'&='";
    case TokenKind::PipeAssign:     return "'|='";
    case TokenKind::CaretAssign:    return "'^='";
    case TokenKind::LShiftAssign:   return "'<<='";
    case TokenKind::RShiftAssign:   return "'>>='";
    case TokenKind::WrapAddAssign:  return "'+%='";
    case TokenKind::WrapSubAssign:  return "'-%='";
    case TokenKind::WrapMulAssign:  return "'*%='";
    case TokenKind::SatAddAssign:   return "'+|='";
    case TokenKind::SatSubAssign:   return "'-|='";
    case TokenKind::SatMulAssign:   return "'*|='";
    // Arithmetic
    case TokenKind::Plus:           return "'+'";
    case TokenKind::Minus:          return "'-'";
    case TokenKind::Star:           return "'*'";
    case TokenKind::Slash:          return "'/'";
    case TokenKind::Percent:        return "'%'";
    case TokenKind::StarStar:       return "'**'";
    case TokenKind::WrapAdd:        return "'+%'";
    case TokenKind::WrapSub:        return "'-%'";
    case TokenKind::WrapMul:        return "'*%'";
    case TokenKind::WrapDiv:        return "'/%'";
    case TokenKind::SatAdd:         return "'+|'";
    case TokenKind::SatSub:         return "'-|'";
    case TokenKind::SatMul:         return "'*|'";
    // Bitwise / logical
    case TokenKind::Amp:            return "'&'";
    case TokenKind::Pipe:           return "'|'";
    case TokenKind::Caret:          return "'^'";
    case TokenKind::Tilde:          return "'~'";
    case TokenKind::LShift:         return "'<<'";
    case TokenKind::RShift:         return "'>>'";
    case TokenKind::URShift:        return "'>>>'";
    case TokenKind::Eq:             return "'=='";
    case TokenKind::NotEq:          return "'!='";
    case TokenKind::Less:           return "'<'";
    case TokenKind::LessEq:         return "'<='";
    case TokenKind::Greater:        return "'>'";
    case TokenKind::GreaterEq:      return "'>='";
    case TokenKind::AmpAmp:         return "'&&'";
    case TokenKind::PipePipe:       return "'||'";
    case TokenKind::Bang:           return "'!'";
    case TokenKind::PlusPlus:       return "'++'";
    case TokenKind::MinusMinus:     return "'--'";
    case TokenKind::Arrow:          return "'->'";
    case TokenKind::FatArrow:       return "'=>'";
    case TokenKind::PipeArrow:      return "'|>'";
    case TokenKind::Hat:            return "'^'";
    default:                        return "<token>";
    }
}

bool tokenKindIsKeyword(TokenKind k) {
    return k >= TokenKind::KW_var && k <= TokenKind::KW_is;
}

bool tokenKindIsLiteral(TokenKind k) {
    return k >= TokenKind::IntLiteral && k <= TokenKind::Null;
}

bool tokenKindIsAssignment(TokenKind k) {
    return k >= TokenKind::Assign && k <= TokenKind::SatMulAssign;
}

bool tokenKindIsOverloadable(TokenKind k) {
    switch (k) {
    case TokenKind::Plus: case TokenKind::Minus:
    case TokenKind::Star: case TokenKind::Slash:
    case TokenKind::Percent: case TokenKind::StarStar:
    case TokenKind::Eq: case TokenKind::NotEq:
    case TokenKind::Less: case TokenKind::LessEq:
    case TokenKind::Greater: case TokenKind::GreaterEq:
    case TokenKind::LBracket:
        return true;
    default: return false;
    }
}

} // namespace vex
