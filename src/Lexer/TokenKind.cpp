// ============================================================================
// vex/Lexer/TokenKind.cpp
// ============================================================================

#include "vex/Lexer/TokenKind.h"
#include <unordered_map>
#include <string_view>

namespace vex {

std::string_view tokenKindName(TokenKind kind) {
    switch (kind) {
        case TokenKind::Eof:              return "<eof>";
        case TokenKind::Error:            return "<error>";
        case TokenKind::Newline:          return "<newline>";
        case TokenKind::IntLiteral:       return "integer literal";
        case TokenKind::FloatLiteral:     return "float literal";
        case TokenKind::StringLiteral:    return "string literal";
        case TokenKind::RawString:        return "raw string";
        case TokenKind::FmtString:        return "format string";
        case TokenKind::MultilineStr:     return "multiline string";
        case TokenKind::CharLiteral:      return "char literal";
        case TokenKind::BoolTrue:         return "true";
        case TokenKind::BoolFalse:        return "false";
        case TokenKind::Null:             return "null";
        case TokenKind::Identifier:       return "identifier";
        case TokenKind::KW_var:           return "var";
        case TokenKind::KW_let:           return "let";
        case TokenKind::KW_const:         return "const";
        case TokenKind::KW_fn:            return "fn";
        case TokenKind::KW_async:         return "async";
        case TokenKind::KW_await:         return "await";
        case TokenKind::KW_yield:         return "yield";
        case TokenKind::KW_if:            return "if";
        case TokenKind::KW_elif:          return "elif";
        case TokenKind::KW_else:          return "else";
        case TokenKind::KW_match:         return "match";
        case TokenKind::KW_for:           return "for";
        case TokenKind::KW_while:         return "while";
        case TokenKind::KW_do:            return "do";
        case TokenKind::KW_loop:          return "loop";
        case TokenKind::KW_in:            return "in";
        case TokenKind::KW_step:          return "step";
        case TokenKind::KW_skip:          return "skip";
        case TokenKind::KW_stop:          return "stop";
        case TokenKind::KW_return:        return "return";
        case TokenKind::KW_struct:        return "struct";
        case TokenKind::KW_class:         return "class";
        case TokenKind::KW_enum:          return "enum";
        case TokenKind::KW_union:         return "union";
        case TokenKind::KW_trait:         return "trait";
        case TokenKind::KW_interface:     return "interface";
        case TokenKind::KW_impl:          return "impl";
        case TokenKind::KW_bitfield:      return "bitfield";
        case TokenKind::KW_sealed:        return "sealed";
        case TokenKind::KW_abstract:      return "abstract";
        case TokenKind::KW_pub:           return "pub";
        case TokenKind::KW_priv:          return "priv";
        case TokenKind::KW_prot:          return "prot";
        case TokenKind::KW_internal:      return "internal";
        case TokenKind::KW_self:          return "self";
        case TokenKind::KW_super:         return "super";
        case TokenKind::KW_Self:          return "Self";
        case TokenKind::KW_mod:           return "mod";
        case TokenKind::KW_init:          return "init";
        case TokenKind::KW_deinit:        return "deinit";
        case TokenKind::KW_static:        return "static";
        case TokenKind::KW_prop:          return "prop";
        case TokenKind::KW_op:            return "op";
        case TokenKind::KW_and:           return "and";
        case TokenKind::KW_or:            return "or";
        case TokenKind::KW_not:           return "not";
        case TokenKind::KW_move:          return "move";
        case TokenKind::KW_copy:          return "copy";
        case TokenKind::KW_unsafe:        return "unsafe";
        case TokenKind::KW_extern:        return "extern";
        case TokenKind::KW_import:        return "import";
        case TokenKind::KW_type:          return "type";
        case TokenKind::KW_distinct:      return "distinct";
        case TokenKind::KW_comptime:      return "comptime";
        case TokenKind::KW_defer:         return "defer";
        case TokenKind::KW_use:           return "use";
        case TokenKind::KW_try:           return "try";
        case TokenKind::KW_catch:         return "catch";
        case TokenKind::KW_panic:         return "panic";
        case TokenKind::KW_assert:        return "assert";
        case TokenKind::KW_assertEq:      return "assertEq";
        case TokenKind::KW_select:        return "select";
        case TokenKind::KW_timeout:       return "timeout";
        case TokenKind::KW_asm:           return "asm";
        case TokenKind::KW_inline:        return "inline";
        case TokenKind::KW_where:         return "where";
        case TokenKind::KW_thread_local:  return "thread_local";
        case TokenKind::KW_as:            return "as";
        case TokenKind::KW_as_q:          return "as?";
        case TokenKind::KW_is:            return "is";
        case TokenKind::KW_for:           return "for";
        case TokenKind::LParen:           return "(";
        case TokenKind::RParen:           return ")";
        case TokenKind::LBracket:         return "[";
        case TokenKind::RBracket:         return "]";
        case TokenKind::LBrace:           return "{";
        case TokenKind::RBrace:           return "}";
        case TokenKind::Comma:            return ",";
        case TokenKind::Semicolon:        return ";";
        case TokenKind::Colon:            return ":";
        case TokenKind::ColonColon:       return "::";
        case TokenKind::Dot:              return ".";
        case TokenKind::DotDot:           return "..";
        case TokenKind::DotDotEq:         return "..=";
        case TokenKind::DotDotDot:        return "...";
        case TokenKind::At:               return "@";
        case TokenKind::Hash:             return "#";
        case TokenKind::Question:         return "?";
        case TokenKind::QuestionDot:      return "?.";
        case TokenKind::QuestionQuestion: return "??";
        case TokenKind::Assign:           return "=";
        case TokenKind::ColonAssign:      return ":=";
        case TokenKind::PlusAssign:       return "+=";
        case TokenKind::MinusAssign:      return "-=";
        case TokenKind::StarAssign:       return "*=";
        case TokenKind::SlashAssign:      return "/=";
        case TokenKind::PercentAssign:    return "%=";
        case TokenKind::StarStarAssign:   return "**=";
        case TokenKind::AmpAssign:        return "&=";
        case TokenKind::PipeAssign:       return "|=";
        case TokenKind::CaretAssign:      return "^=";
        case TokenKind::LShiftAssign:     return "<<=";
        case TokenKind::RShiftAssign:     return ">>=";
        case TokenKind::WrapAddAssign:    return "+%=";
        case TokenKind::WrapSubAssign:    return "-%=";
        case TokenKind::WrapMulAssign:    return "*%=";
        case TokenKind::SatAddAssign:     return "+|=";
        case TokenKind::SatSubAssign:     return "-|=";
        case TokenKind::SatMulAssign:     return "*|=";
        case TokenKind::Plus:             return "+";
        case TokenKind::Minus:            return "-";
        case TokenKind::Star:             return "*";
        case TokenKind::Slash:            return "/";
        case TokenKind::Percent:          return "%";
        case TokenKind::StarStar:         return "**";
        case TokenKind::WrapAdd:          return "+%";
        case TokenKind::WrapSub:          return "-%";
        case TokenKind::WrapMul:          return "*%";
        case TokenKind::WrapDiv:          return "/%";
        case TokenKind::SatAdd:           return "+|";
        case TokenKind::SatSub:           return "-|";
        case TokenKind::SatMul:           return "*|";
        case TokenKind::Amp:              return "&";
        case TokenKind::Pipe:             return "|";
        case TokenKind::Caret:            return "^";
        case TokenKind::Tilde:            return "~";
        case TokenKind::LShift:           return "<<";
        case TokenKind::RShift:           return ">>";
        case TokenKind::URShift:          return ">>>";
        case TokenKind::Eq:               return "==";
        case TokenKind::NotEq:            return "!=";
        case TokenKind::Less:             return "<";
        case TokenKind::LessEq:           return "<=";
        case TokenKind::Greater:          return ">";
        case TokenKind::GreaterEq:        return ">=";
        case TokenKind::AmpAmp:           return "&&";
        case TokenKind::PipePipe:         return "||";
        case TokenKind::Bang:             return "!";
        case TokenKind::PlusPlus:         return "++";
        case TokenKind::MinusMinus:       return "--";
        case TokenKind::Arrow:            return "->";
        case TokenKind::FatArrow:         return "=>";
        case TokenKind::PipeArrow:        return "|>";
        case TokenKind::Hat:              return "^";
        default:                          return "<unknown>";
    }
}

bool isKeyword(TokenKind kind) {
    auto v  = static_cast<uint16_t>(kind);
    auto lo = static_cast<uint16_t>(TokenKind::KW_var);
    auto hi = static_cast<uint16_t>(TokenKind::KW_is);
    return v >= lo && v <= hi;
}

bool isLiteral(TokenKind kind) {
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
            return true;
        default:
            return false;
    }
}

bool isAssignment(TokenKind kind) {
    switch (kind) {
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

bool isBinaryOp(TokenKind kind) {
    switch (kind) {
        case TokenKind::Plus:     case TokenKind::Minus:
        case TokenKind::Star:     case TokenKind::Slash:
        case TokenKind::Percent:  case TokenKind::StarStar:
        case TokenKind::WrapAdd:  case TokenKind::WrapSub:
        case TokenKind::WrapMul:  case TokenKind::WrapDiv:
        case TokenKind::SatAdd:   case TokenKind::SatSub:
        case TokenKind::SatMul:
        case TokenKind::Amp:      case TokenKind::Pipe:
        case TokenKind::Caret:    case TokenKind::LShift:
        case TokenKind::RShift:   case TokenKind::URShift:
        case TokenKind::Eq:       case TokenKind::NotEq:
        case TokenKind::Less:     case TokenKind::LessEq:
        case TokenKind::Greater:  case TokenKind::GreaterEq:
        case TokenKind::AmpAmp:   case TokenKind::PipePipe:
        case TokenKind::KW_and:   case TokenKind::KW_or:
        case TokenKind::QuestionQuestion:
        case TokenKind::PipeArrow:
        case TokenKind::ColonColon:
        case TokenKind::KW_as:    case TokenKind::KW_is:
        case TokenKind::DotDot:   case TokenKind::DotDotEq:
            return true;
        default:
            return false;
    }
}

bool isUnaryPrefixOp(TokenKind kind) {
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
            return true;
        default:
            return false;
    }
}

TokenKind lookupKeyword(std::string_view text) {
    static const std::unordered_map<std::string_view, TokenKind> kw = {
        {"var",          TokenKind::KW_var},
        {"let",          TokenKind::KW_let},
        {"const",        TokenKind::KW_const},
        {"fn",           TokenKind::KW_fn},
        {"async",        TokenKind::KW_async},
        {"await",        TokenKind::KW_await},
        {"yield",        TokenKind::KW_yield},
        {"if",           TokenKind::KW_if},
        {"elif",         TokenKind::KW_elif},
        {"else",         TokenKind::KW_else},
        {"match",        TokenKind::KW_match},
        {"for",          TokenKind::KW_for},
        {"while",        TokenKind::KW_while},
        {"do",           TokenKind::KW_do},
        {"loop",         TokenKind::KW_loop},
        {"in",           TokenKind::KW_in},
        {"step",         TokenKind::KW_step},
        {"skip",         TokenKind::KW_skip},
        {"stop",         TokenKind::KW_stop},
        {"return",       TokenKind::KW_return},
        {"struct",       TokenKind::KW_struct},
        {"class",        TokenKind::KW_class},
        {"enum",         TokenKind::KW_enum},
        {"union",        TokenKind::KW_union},
        {"trait",        TokenKind::KW_trait},
        {"interface",    TokenKind::KW_interface},
        {"impl",         TokenKind::KW_impl},
        {"bitfield",     TokenKind::KW_bitfield},
        {"sealed",       TokenKind::KW_sealed},
        {"abstract",     TokenKind::KW_abstract},
        {"pub",          TokenKind::KW_pub},
        {"priv",         TokenKind::KW_priv},
        {"prot",         TokenKind::KW_prot},
        {"internal",     TokenKind::KW_internal},
        {"self",         TokenKind::KW_self},
        {"super",        TokenKind::KW_super},
        {"Self",         TokenKind::KW_Self},
        {"mod",          TokenKind::KW_mod},
        {"init",         TokenKind::KW_init},
        {"deinit",       TokenKind::KW_deinit},
        {"static",       TokenKind::KW_static},
        {"prop",         TokenKind::KW_prop},
        {"op",           TokenKind::KW_op},
        {"true",         TokenKind::BoolTrue},
        {"false",        TokenKind::BoolFalse},
        {"null",         TokenKind::Null},
        {"and",          TokenKind::KW_and},
        {"or",           TokenKind::KW_or},
        {"not",          TokenKind::KW_not},
        {"move",         TokenKind::KW_move},
        {"copy",         TokenKind::KW_copy},
        {"unsafe",       TokenKind::KW_unsafe},
        {"extern",       TokenKind::KW_extern},
        {"import",       TokenKind::KW_import},
        {"type",         TokenKind::KW_type},
        {"distinct",     TokenKind::KW_distinct},
        {"comptime",     TokenKind::KW_comptime},
        {"defer",        TokenKind::KW_defer},
        {"use",          TokenKind::KW_use},
        {"try",          TokenKind::KW_try},
        {"catch",        TokenKind::KW_catch},
        {"panic",        TokenKind::KW_panic},
        {"assert",       TokenKind::KW_assert},
        {"assertEq",     TokenKind::KW_assertEq},
        {"select",       TokenKind::KW_select},
        {"timeout",      TokenKind::KW_timeout},
        {"asm",          TokenKind::KW_asm},
        {"inline",       TokenKind::KW_inline},
        {"where",        TokenKind::KW_where},
        {"thread_local", TokenKind::KW_thread_local},
        {"as",           TokenKind::KW_as},
        {"is",           TokenKind::KW_is},
    };

    auto it = kw.find(text);
    return (it != kw.end()) ? it->second : TokenKind::Identifier;
}

} // namespace vex