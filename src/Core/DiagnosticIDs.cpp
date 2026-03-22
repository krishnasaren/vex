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

        // Note
        case DiagID::None:
            return DiagSeverity::Note;

        // Everything else is an error
        default:
            return DiagSeverity::Error;
    }
}

const char* diagName(DiagID id) {
    switch (id) {
        case DiagID::None:                          return "none";
        case DiagID::InternalError:                 return "internal-error";
        // Lexer
        case DiagID::LEX_UnknownChar:               return "unknown-char";
        case DiagID::LEX_UnterminatedString:        return "unterminated-string";
        case DiagID::LEX_UnterminatedChar:          return "unterminated-char";
        case DiagID::LEX_InvalidCharLiteral:        return "invalid-char-literal";
        case DiagID::LEX_UnterminatedComment:       return "unterminated-comment";
        case DiagID::LEX_InvalidEscape:             return "invalid-escape";
        case DiagID::LEX_InvalidUnicodeEscape:      return "invalid-unicode-escape";
        case DiagID::LEX_InvalidIntLiteral:         return "invalid-int-literal";
        case DiagID::LEX_InvalidFloatLiteral:       return "invalid-float-literal";
        case DiagID::LEX_IntSuffixOverflow:         return "int-suffix-overflow";
        case DiagID::LEX_InvalidRawString:          return "invalid-raw-string";
        // Parser
        case DiagID::PARSE_Expected:                return "expected";
        case DiagID::PARSE_UnexpectedToken:         return "unexpected-token";
        case DiagID::PARSE_UnexpectedEOF:           return "unexpected-eof";
        case DiagID::PARSE_MissingSemicolon:        return "missing-semicolon";
        case DiagID::PARSE_InvalidDecl:             return "invalid-decl";
        case DiagID::PARSE_InvalidExpr:             return "invalid-expr";
        case DiagID::PARSE_InvalidType:             return "invalid-type";
        case DiagID::PARSE_InvalidPattern:          return "invalid-pattern";
        case DiagID::PARSE_InvalidImport:           return "invalid-import";
        case DiagID::PARSE_ImportHasFunction:       return "import-has-function";
        case DiagID::PARSE_InvalidColonColon:       return "invalid-coloncolon";
        case DiagID::PARSE_MissingArrow:            return "missing-arrow";
        case DiagID::PARSE_InvalidGenericParam:     return "invalid-generic-param";
        case DiagID::PARSE_TooManyParams:           return "too-many-params";
        // Sema
        case DiagID::SEMA_UndefinedName:            return "undefined-name";
        case DiagID::SEMA_AmbiguousName:            return "ambiguous-name";
        case DiagID::SEMA_UseBeforeInit:            return "use-before-init";
        case DiagID::SEMA_UseAfterMove:             return "use-after-move";
        case DiagID::SEMA_ShadowWarning:            return "shadow";
        case DiagID::SEMA_ModuleNotModule:          return "module-not-module";
        case DiagID::SEMA_NotPublic:                return "not-public";
        case DiagID::SEMA_ProtAccessOutside:        return "prot-access-outside";
        case DiagID::SEMA_ModuleConflict:           return "module-conflict";
        case DiagID::SEMA_SelectiveConflict:        return "selective-conflict";
        case DiagID::SEMA_OpenImportShadow:         return "open-import-shadow";
        case DiagID::SEMA_ModuleNotFound:           return "module-not-found";
        case DiagID::SEMA_CircularImport:           return "circular-import";
        case DiagID::SEMA_ImportTwice:              return "import-twice";
        case DiagID::SEMA_ColonColonOutsideImport:  return "coloncolon-outside-import";
        // Type
        case DiagID::TYPE_Mismatch:                 return "type-mismatch";
        case DiagID::TYPE_CannotInfer:              return "cannot-infer-type";
        case DiagID::TYPE_NullableRequired:         return "nullable-required";
        case DiagID::TYPE_NullableUnwrap:           return "nullable-unwrap";
        case DiagID::TYPE_IntLiteralTooBig:         return "int-literal-too-big";
        case DiagID::TYPE_FloatLiteralContext:      return "float-literal-context";
        case DiagID::TYPE_NarrowingCast:            return "narrowing-cast";
        case DiagID::TYPE_ImplicitConversion:       return "implicit-conversion";
        case DiagID::TYPE_ReturnTypeMismatch:       return "return-type-mismatch";
        case DiagID::TYPE_VoidReturn:               return "void-return";
        case DiagID::TYPE_NoReturnValue:            return "no-return-value";
        case DiagID::TYPE_MatchArmMismatch:         return "match-arm-mismatch";
        case DiagID::TYPE_EmptyCollectionNoCtx:     return "empty-collection-no-ctx";
        case DiagID::TYPE_LambdaParamUnknown:       return "lambda-param-unknown";
        case DiagID::TYPE_GenericArgConflict:       return "generic-arg-conflict";
        case DiagID::TYPE_TraitNotImplemented:      return "trait-not-implemented";
        case DiagID::TYPE_TraitBoundViolated:       return "trait-bound-violated";
        case DiagID::TYPE_SelfOutsideMethod:        return "self-outside-method";
        case DiagID::TYPE_SuperOutsideClass:        return "super-outside-class";
        case DiagID::TYPE_ModOutsideModule:         return "mod-outside-module";
        case DiagID::TYPE_StaticOnInstance:         return "static-on-instance";
        case DiagID::TYPE_InstanceOnType:           return "instance-on-type";
        case DiagID::TYPE_NotCallable:              return "not-callable";
        case DiagID::TYPE_WrongArgCount:            return "wrong-arg-count";
        case DiagID::TYPE_AmbiguousOverload:        return "ambiguous-overload";
        case DiagID::TYPE_NoMatchingOverload:       return "no-matching-overload";
        case DiagID::TYPE_CannotCallAny:            return "cannot-call-any";
        // Borrow
        case DiagID::BORROW_UseAfterMove:           return "borrow-use-after-move";
        case DiagID::BORROW_DoubleMove:             return "double-move";
        case DiagID::BORROW_MovedWhileBorrowed:     return "moved-while-borrowed";
        case DiagID::BORROW_MutBorrowWhileBorrowed: return "mut-borrow-while-borrowed";
        case DiagID::BORROW_SharedWhileMutBorrowed: return "shared-while-mut-borrowed";
        case DiagID::BORROW_BorrowOutlivesOwner:    return "borrow-outlives-owner";
        case DiagID::BORROW_ReturnLocalRef:         return "return-local-ref";
        // Flow
        case DiagID::FLOW_NonExhaustiveMatch:       return "non-exhaustive-match";
        case DiagID::FLOW_UnreachablePattern:       return "unreachable-pattern";
        case DiagID::FLOW_BreakOutsideLoop:         return "break-outside-loop";
        case DiagID::FLOW_ContinueOutsideLoop:      return "continue-outside-loop";
        case DiagID::FLOW_YieldOutsideGenerator:    return "yield-outside-generator";
        case DiagID::FLOW_AwaitOutsideAsync:        return "await-outside-async";
        case DiagID::FLOW_ReturnInGenerator:        return "return-in-generator";
        case DiagID::FLOW_UnreachableCode:          return "unreachable-code";
        // Arith
        case DiagID::ARITH_ConstOverflow:           return "const-overflow";
        case DiagID::ARITH_ShiftTooLarge:           return "shift-too-large";
        case DiagID::ARITH_ShiftNegative:           return "shift-negative";
        case DiagID::ARITH_DivByZeroConst:          return "div-by-zero-const";
        // Generics
        case DiagID::GEN_RecursiveDifferentType:    return "recursive-different-type";
        case DiagID::GEN_ComptimeRecursionLimit:    return "comptime-recursion-limit";
        // Visibility
        case DiagID::VIS_ProtOnNonClass:            return "prot-on-non-class";
        case DiagID::VIS_InternalFromOutside:       return "internal-from-outside";
        case DiagID::VIS_PrivFromOutside:           return "priv-from-outside";
        // Driver
        case DiagID::DRIVER_UnknownOption:          return "unknown-option";
        case DiagID::DRIVER_NoInput:                return "no-input";
        case DiagID::DRIVER_OutputConflict:         return "output-conflict";
        default:                                    return "unknown";
    }
}

} // namespace vex