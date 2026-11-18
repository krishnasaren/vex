// src/Core/DiagnosticEngine.cpp
#include "vex/Core/DiagnosticEngine.h"

namespace vex {

// ── DiagnosticBuilder ─────────────────────────────────────────────────────────

DiagnosticBuilder::~DiagnosticBuilder() {
    if (!committed_) emit();
}

void DiagnosticBuilder::emit() {
    if (committed_) return;
    committed_ = true;
    engine_.emit(diag_);
}

// ── DiagnosticEngine ──────────────────────────────────────────────────────────

DiagnosticBuilder DiagnosticEngine::report(DiagID id,
                                           SourceLocation loc,
                                           std::string message) {
    Diagnostic diag(id, loc, std::move(message));
    return DiagnosticBuilder(*this, std::move(diag));
}

void DiagnosticEngine::emitSimple(DiagID id, SourceLocation loc,
                                   std::string message) {
    Diagnostic diag(id, loc, std::move(message));
    emit(diag);
}

void DiagnosticEngine::emit(const Diagnostic& diag) {
    // Update counters before potentially truncating
    if (diag.isError() || diag.isFatal()) {
        ++errorCount_;
        if (diag.isFatal()) hasFatal_ = true;
    } else if (diag.isWarning()) {
        if (suppressWarnings_) return;
        if (warningsAsErrors_) {
            // Upgrade to error
            Diagnostic upgraded = diag;
            upgraded.setSeverity(DiagSeverity::Error);
            ++errorCount_;
            consumer_.handleDiagnostic(upgraded);
            return;
        }
        ++warningCount_;
    }

    consumer_.handleDiagnostic(diag);
}

} // namespace vex
