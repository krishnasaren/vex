// ============================================================================
// vex/Core/DiagnosticEngine.cpp
// ============================================================================

#include "vex/Core/DiagnosticEngine.h"

namespace vex {

// ── DiagnosticBuilder ─────────────────────────────────────────────────────────

DiagnosticBuilder::~DiagnosticBuilder() {
    if (!committed_)
        emit();
}

void DiagnosticBuilder::emit() {
    if (!committed_) {
        committed_ = true;
        engine_.emit(diag_);
    }
}

// ── DiagnosticEngine ──────────────────────────────────────────────────────────

DiagnosticBuilder DiagnosticEngine::report(DiagID id,
                                            SourceLocation loc,
                                            std::string message) {
    return DiagnosticBuilder(*this, Diagnostic(id, loc, std::move(message)));
}

void DiagnosticEngine::emitSimple(DiagID id,
                                   SourceLocation loc,
                                   std::string message) {
    emit(Diagnostic(id, loc, std::move(message)));
}

void DiagnosticEngine::emit(const Diagnostic& diag) {
    // Apply warnings-as-errors upgrade
    Diagnostic d = diag;
    if (warningsAsErrors_ && d.isWarning())
        d.setSeverity(DiagSeverity::Error);

    // Suppress warnings if configured
    if (suppressWarnings_ && d.isWarning())
        return;

    // Count
    if (d.isError() || d.isFatal()) {
        ++errorCount_;
        if (d.isFatal())
            hasFatal_ = true;
    } else if (d.isWarning()) {
        ++warningCount_;
    }

    // Deliver to consumer
    consumer_.handleDiagnostic(d);
}

} // namespace vex