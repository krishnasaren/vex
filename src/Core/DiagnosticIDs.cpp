// src/Core/DiagnosticIDs.cpp
#include "vex/Core/DiagnosticIDs.h"

namespace vex {

DiagSeverity defaultSeverity(DiagID id) {
    switch (id) {
    case DiagID::None:              return DiagSeverity::Note;
    case DiagID::InternalError:     return DiagSeverity::Fatal;
    // Lexer
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
    // Parser
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
    case DiagID::SEMA_UndefinedName:
    case DiagID::SEMA_AmbiguousName:
    case DiagID::SEMA_UseBeforeInit:
    case DiagID::SEMA_UseAfterMove:
    case DiagID::SEMA_ModuleNotModule:
    case DiagID::SEMA_NotPublic:
    case DiagID::SEMA_ProtAccessOutside:
    case DiagID::SEMA_ModuleConflict:
    case DiagID::SEMA_SelectiveConflict:
    case DiagID::SEMA_ModuleNotFound:
    case DiagID::SEMA_CircularImport:
    case DiagID::SEMA_ImportTwice:
    case DiagID::SEMA_ColonColonOutsideImport:
        return DiagSeverity::Error;
    case DiagID::SEMA_ShadowWarning:
    case DiagID::SEMA_OpenImportShadow:
        return DiagSeverity::Warning;
    // Type
    case DiagID::TYPE_Mismatch:
    case DiagID::TYPE_CannotInfer:
    case DiagID::TYPE_NullableRequired:
    case DiagID::TYPE_NullableUnwrap:
    case DiagID::TYPE_IntLiteralTooBig:
    case DiagID::TYPE_FloatLiteralContext:
    case DiagID::TYPE_NarrowingCast:
    case DiagID::TYPE_ImplicitConversion:
    case DiagID::TYPE_ReturnTypeMismatch:
    case DiagID::TYPE_VoidReturn:
    case DiagID::TYPE_NoReturnValue:
    case DiagID::TYPE_MatchArmMismatch:
    case DiagID::TYPE_EmptyCollectionNoCtx:
    case DiagID::TYPE_LambdaParamUnknown:
    case DiagID::TYPE_GenericArgConflict:
    case DiagID::TYPE_TraitNotImplemented:
    case DiagID::TYPE_TraitBoundViolated:
    case DiagID::TYPE_SelfOutsideMethod:
    case DiagID::TYPE_SuperOutsideClass:
    case DiagID::TYPE_ModOutsideModule:
    case DiagID::TYPE_StaticOnInstance:
    case DiagID::TYPE_InstanceOnType:
    case DiagID::TYPE_NotCallable:
    case DiagID::TYPE_WrongArgCount:
    case DiagID::TYPE_AmbiguousOverload:
    case DiagID::TYPE_NoMatchingOverload:
    case DiagID::TYPE_CannotCallAny:
        return DiagSeverity::Error;
    // Borrow
    case DiagID::BORROW_UseAfterMove:
    case DiagID::BORROW_DoubleMove:
    case DiagID::BORROW_MovedWhileBorrowed:
    case DiagID::BORROW_MutBorrowWhileBorrowed:
    case DiagID::BORROW_SharedWhileMutBorrowed:
    case DiagID::BORROW_BorrowOutlivesOwner:
    case DiagID::BORROW_ReturnLocalRef:
        return DiagSeverity::Error;
    // Control flow
    case DiagID::FLOW_NonExhaustiveMatch:
    case DiagID::FLOW_BreakOutsideLoop:
    case DiagID::FLOW_ContinueOutsideLoop:
    case DiagID::FLOW_YieldOutsideGenerator:
    case DiagID::FLOW_AwaitOutsideAsync:
    case DiagID::FLOW_ReturnInGenerator:
        return DiagSeverity::Error;
    case DiagID::FLOW_UnreachablePattern:
    case DiagID::FLOW_UnreachableCode:
        return DiagSeverity::Warning;
    // Arith
    case DiagID::ARITH_ConstOverflow:
    case DiagID::ARITH_ShiftTooLarge:
    case DiagID::ARITH_ShiftNegative:
    case DiagID::ARITH_DivByZeroConst:
        return DiagSeverity::Error;
    // Generics
    case DiagID::GEN_RecursiveDifferentType:
    case DiagID::GEN_ComptimeRecursionLimit:
        return DiagSeverity::Error;
    // Visibility
    case DiagID::VIS_ProtOnNonClass:
    case DiagID::VIS_InternalFromOutside:
    case DiagID::VIS_PrivFromOutside:
        return DiagSeverity::Error;
    // Driver
    case DiagID::DRIVER_UnknownOption:
    case DiagID::DRIVER_NoInput:
    case DiagID::DRIVER_OutputConflict:
        return DiagSeverity::Error;
    default:
        return DiagSeverity::Error;
    }
}

const char* diagName(DiagID id) {
    switch (id) {
    case DiagID::None:              return "none";
    case DiagID::InternalError:     return "internal-error";
    case DiagID::LEX_UnknownChar:   return "unknown-char";
    case DiagID::LEX_UnterminatedString: return "unterminated-string";
    case DiagID::LEX_UnterminatedChar:   return "unterminated-char";
    case DiagID::LEX_InvalidCharLiteral: return "invalid-char-literal";
    case DiagID::LEX_UnterminatedComment: return "unterminated-comment";
    case DiagID::LEX_InvalidEscape:  return "invalid-escape";
    case DiagID::LEX_InvalidUnicodeEscape: return "invalid-unicode-escape";
    case DiagID::LEX_InvalidIntLiteral:    return "invalid-int-literal";
    case DiagID::LEX_InvalidFloatLiteral:  return "invalid-float-literal";
    case DiagID::LEX_IntSuffixOverflow:    return "int-suffix-overflow";
    case DiagID::LEX_InvalidRawString:     return "invalid-raw-string";
    case DiagID::PARSE_Expected:           return "expected";
    case DiagID::PARSE_UnexpectedToken:    return "unexpected-token";
    case DiagID::PARSE_UnexpectedEOF:      return "unexpected-eof";
    case DiagID::SEMA_UndefinedName:       return "undefined-name";
    case DiagID::TYPE_Mismatch:            return "type-mismatch";
    case DiagID::TYPE_NotCallable:         return "not-callable";
    case DiagID::TYPE_WrongArgCount:       return "wrong-arg-count";
    case DiagID::BORROW_UseAfterMove:      return "use-after-move";
    case DiagID::FLOW_NonExhaustiveMatch:  return "non-exhaustive-match";
    case DiagID::DRIVER_NoInput:           return "no-input";
    default:                               return "unknown";
    }
}

} // namespace vex
