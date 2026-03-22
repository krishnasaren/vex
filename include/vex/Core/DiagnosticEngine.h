#pragma once
// ============================================================================
// vex/Core/DiagnosticEngine.h
// Central hub for emitting diagnostics throughout the compiler.
// Every compiler phase holds a reference to DiagnosticEngine.
// ============================================================================

#include "vex/Core/Diagnostic.h"
#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/SourceManager.h"

#include <memory>
#include <string>
#include <functional>

namespace vex {

// ── DiagnosticBuilder ─────────────────────────────────────────────────────────
// RAII builder returned by DiagnosticEngine::report().
// The diagnostic is emitted when the builder is destroyed.
//
// Usage:
//   engine.report(DiagID::TYPE_Mismatch, loc, "expected int, found str")
//         .setPrimaryLabel(range, "this is str")
//         .addNote("declared as int here", declLoc);

class DiagnosticEngine;

class DiagnosticBuilder {
public:
    DiagnosticBuilder(DiagnosticEngine& engine, Diagnostic diag)
        : engine_(engine), diag_(std::move(diag)), committed_(false) {}

    ~DiagnosticBuilder();

    // Prevent copying — only one builder per diagnostic
    DiagnosticBuilder(const DiagnosticBuilder&) = delete;
    DiagnosticBuilder& operator=(const DiagnosticBuilder&) = delete;

    // Allow moving
    DiagnosticBuilder(DiagnosticBuilder&& o)
        : engine_(o.engine_), diag_(std::move(o.diag_)), committed_(o.committed_) {
        o.committed_ = true; // prevent double-emit
    }

    DiagnosticBuilder& setPrimaryLabel(SourceRange range, std::string text = {}) {
        diag_.setPrimaryLabel(range, std::move(text));
        return *this;
    }

    DiagnosticBuilder& addLabel(SourceRange range, std::string text,
                                bool primary = false) {
        diag_.addLabel(range, std::move(text), primary);
        return *this;
    }

    DiagnosticBuilder& addNote(std::string text,
                               SourceLocation loc = SourceLocation::invalid()) {
        diag_.addNote(std::move(text), loc);
        return *this;
    }

    DiagnosticBuilder& setHint(std::string hint) {
        diag_.setHint(std::move(hint));
        return *this;
    }

    DiagnosticBuilder& setSeverity(DiagSeverity sev) {
        diag_.setSeverity(sev);
        return *this;
    }

    // Emit now instead of waiting for destruction
    void emit();

private:
    DiagnosticEngine& engine_;
    Diagnostic        diag_;
    bool              committed_;
};

// ── DiagnosticEngine ──────────────────────────────────────────────────────────

class DiagnosticEngine {
public:
    explicit DiagnosticEngine(DiagnosticConsumer& consumer)
        : consumer_(consumer) {}

    // Non-copyable
    DiagnosticEngine(const DiagnosticEngine&) = delete;
    DiagnosticEngine& operator=(const DiagnosticEngine&) = delete;

    // ── Emit a diagnostic ─────────────────────────────────────────────────────

    // Returns a builder. Diagnostic is emitted when the builder is destroyed.
    DiagnosticBuilder report(DiagID id,
                             SourceLocation loc,
                             std::string message);

    // Convenience: emit immediately with no extra context
    void emitSimple(DiagID id, SourceLocation loc, std::string message);

    // Internal: called by DiagnosticBuilder::emit()
    void emit(const Diagnostic& diag);

    // ── Statistics ────────────────────────────────────────────────────────────

    uint32_t errorCount()   const { return errorCount_; }
    uint32_t warningCount() const { return warningCount_; }
    bool     hasErrors()    const { return errorCount_ > 0; }
    bool     hasFatalError() const { return hasFatal_; }

    // ── Configuration ─────────────────────────────────────────────────────────

    // Stop after this many errors (default: 20)
    void setErrorLimit(uint32_t limit) { errorLimit_ = limit; }

    // Treat all warnings as errors
    void setWarningsAsErrors(bool val) { warningsAsErrors_ = val; }

    // Suppress all warnings
    void setSuppressWarnings(bool val) { suppressWarnings_ = val; }

    // Called to check if compilation should abort (fatal error occurred)
    bool shouldAbort() const { return hasFatal_ || errorCount_ >= errorLimit_; }

private:
    DiagnosticConsumer& consumer_;
    uint32_t            errorCount_       = 0;
    uint32_t            warningCount_     = 0;
    uint32_t            errorLimit_       = 20;
    bool                hasFatal_         = false;
    bool                warningsAsErrors_ = false;
    bool                suppressWarnings_ = false;
};

} // namespace vex