// ============================================================================
// vex/Core/DiagnosticIDs.cpp
// ============================================================================

#include "vex/Core/DiagnosticIDs.h"

namespace vex {

DiagSeverity defaultSeverity(DiagID id) {
    switch (id) {
        // Warnings
        case DiagID::SEMA_ShadowWarning:
        case DiagID::SEMA_OpenImportShadow:
        case DiagID::FLOW_UnreachableCode:
            return DiagSeverity::Warning;

        // Fatal
        case DiagID::InternalError:
            return DiagSeverity::Fatal;

        // Notes
        case DiagID::None:
            return DiagSeverity::Note;

        // Everything else is an error
        default:
            return DiagSeverity::Error;
    }
}

const char* diagName(DiagID id) {
    switch (id) {
        case DiagID::None:                        return "none";
        case DiagID::InternalError:               return "internal-error";
        case DiagID::LEX_UnknownChar:             return "unknown-char";
        case DiagID::LEX_UnterminatedString:      return "unterminated-string";
        case DiagID::LEX_UnterminatedChar:        return "unterminated-char";
        case DiagID::LEX_InvalidCharLiteral:      return "invalid-char-literal";
        case DiagID::LEX_UnterminatedComment:     return "unterminated-comment";
        case DiagID::LEX_InvalidEscape:           return "invalid-escape";
        case DiagID::LEX_InvalidUnicodeEscape:    return "invalid-unicode-escape";
        case DiagID::LEX_InvalidIntLiteral:       return "invalid-int-literal";
        case DiagID::LEX_InvalidFloatLiteral:     return "invalid-float-literal";
        case DiagID::LEX_IntSuffixOverflow:       return "int-suffix-overflow";
        case DiagID::LEX_InvalidRawString:        return "invalid-raw-string";
        case DiagID::PARSE_Expected:              return "expected";
        case DiagID::PARSE_UnexpectedToken:       return "unexpected-token";
        case DiagID::PARSE_UnexpectedEOF:         return "unexpected-eof";
        case DiagID::PARSE_InvalidDecl:           return "invalid-decl";
        case DiagID::PARSE_InvalidExpr:           return "invalid-expr";
        case DiagID::PARSE_InvalidType:           return "invalid-type";
        case DiagID::PARSE_InvalidPattern:        return "invalid-pattern";
        case DiagID::PARSE_InvalidImport:         return "invalid-import";
        case DiagID::PARSE_ImportHasFunction:     return "import-has-function";
        case DiagID::PARSE_InvalidColonColon:     return "invalid-coloncolon";
        case DiagID::SEMA_UndefinedName:          return "undefined-name";
        case DiagID::SEMA_AmbiguousName:          return "ambiguous-name";
        case DiagID::SEMA_UseBeforeInit:          return "use-before-init";
        case DiagID::SEMA_UseAfterMove:           return "use-after-move";
        case DiagID::SEMA_ShadowWarning:          return "shadow";
        case DiagID::SEMA_ModuleConflict:         return "module-conflict";
        case DiagID::SEMA_SelectiveConflict:      return "selective-import-conflict";
        case DiagID::SEMA_OpenImportShadow:       return "open-import-shadow";
        case DiagID::SEMA_ModuleNotFound:         return "module-not-found";
        case DiagID::SEMA_CircularImport:         return "circular-import";
        case DiagID::SEMA_ImportTwice:            return "import-twice";
        case DiagID::SEMA_ColonColonOutsideImport: return "coloncolon-outside-import";
        case DiagID::TYPE_Mismatch:               return "type-mismatch";
        case DiagID::TYPE_CannotInfer:            return "cannot-infer-type";
        case DiagID::TYPE_NullableRequired:       return "nullable-required";
        case DiagID::TYPE_NullableUnwrap:         return "nullable-unwrap";
        case DiagID::TYPE_ReturnTypeMismatch:     return "return-type-mismatch";
        case DiagID::TYPE_NoReturnValue:          return "no-return-value";
        case DiagID::TYPE_MatchArmMismatch:       return "match-arm-mismatch";
        case DiagID::TYPE_EmptyCollectionNoCtx:   return "empty-collection-no-ctx";
        case DiagID::TYPE_TraitNotImplemented:    return "trait-not-implemented";
        case DiagID::TYPE_TraitBoundViolated:     return "trait-bound-violated";
        case DiagID::TYPE_StaticOnInstance:       return "static-on-instance";
        case DiagID::TYPE_InstanceOnType:         return "instance-on-type";
        case DiagID::TYPE_AmbiguousOverload:      return "ambiguous-overload";
        case DiagID::TYPE_NoMatchingOverload:     return "no-matching-overload";
        case DiagID::TYPE_CannotCallAny:          return "cannot-call-any";
        case DiagID::BORROW_UseAfterMove:         return "borrow-use-after-move";
        case DiagID::BORROW_DoubleMove:           return "double-move";
        case DiagID::BORROW_MutBorrowWhileBorrowed: return "mut-borrow-while-borrowed";
        case DiagID::BORROW_BorrowOutlivesOwner:  return "borrow-outlives-owner";
        case DiagID::BORROW_ReturnLocalRef:       return "return-local-ref";
        case DiagID::FLOW_NonExhaustiveMatch:     return "non-exhaustive-match";
        case DiagID::FLOW_UnreachablePattern:     return "unreachable-pattern";
        case DiagID::FLOW_BreakOutsideLoop:       return "break-outside-loop";
        case DiagID::FLOW_YieldOutsideGenerator:  return "yield-outside-generator";
        case DiagID::FLOW_AwaitOutsideAsync:      return "await-outside-async";
        case DiagID::FLOW_UnreachableCode:        return "unreachable-code";
        case DiagID::ARITH_ConstOverflow:         return "const-overflow";
        case DiagID::ARITH_ShiftTooLarge:         return "shift-too-large";
        case DiagID::ARITH_ShiftNegative:         return "shift-negative";
        case DiagID::VIS_ProtOnNonClass:          return "prot-on-non-class";
        case DiagID::VIS_InternalFromOutside:     return "internal-from-outside";
        case DiagID::VIS_PrivFromOutside:         return "priv-from-outside";
        default:                                  return "unknown";
    }
}

} // namespace vex