// src/Core/Diagnostic.cpp
#include "vex/Core/Diagnostic.h"
#include "vex/Core/DiagnosticIDs.h"

namespace vex {

DiagSeverity Diagnostic::defaultSeverity(DiagID id) {
    switch (id) {
        case DiagID::None:              return DiagSeverity::Note;
        case DiagID::InternalError:     return DiagSeverity::Fatal;
        // Lexer errors
        case DiagID::LEX_UnknownChar:
        case DiagID::LEX_UnterminatedString:
        case DiagID::LEX_UnterminatedChar:
        case DiagID::LEX_InvalidCharLiteral:
        case DiagID::LEX_UnterminatedComment:
        case DiagID::LEX_InvalidEscape:
        case DiagID::LEX_InvalidUnicodeEscape:
        case DiagID::LEX_InvalidIntLiteral:
        case DiagID::LEX_InvalidFloatLiteral:
        case DiagID::LEX_IntSuffixOverflow:
        case DiagID::LEX_InvalidRawString:
            return DiagSeverity::Error;
        // Parser errors
        case DiagID::PARSE_Expected:
        case DiagID::PARSE_UnexpectedToken:
        case DiagID::PARSE_UnexpectedEOF:
        case DiagID::PARSE_InvalidDecl:
        case DiagID::PARSE_InvalidExpr:
        case DiagID::PARSE_InvalidType:
        case DiagID::PARSE_InvalidPattern:
        case DiagID::PARSE_InvalidImport:
        case DiagID::PARSE_ImportHasFunction:
        case DiagID::PARSE_InvalidColonColon:
        case DiagID::PARSE_MissingArrow:
        case DiagID::PARSE_InvalidGenericParam:
        case DiagID::PARSE_TooManyParams:
            return DiagSeverity::Error;
        case DiagID::PARSE_MissingSemicolon:
            return DiagSeverity::Warning;
        // Sema
        case DiagID::SEMA_ShadowWarning:
        case DiagID::SEMA_OpenImportShadow:
            return DiagSeverity::Warning;
        default:
            return DiagSeverity::Error;
    }
}

} // namespace vex
