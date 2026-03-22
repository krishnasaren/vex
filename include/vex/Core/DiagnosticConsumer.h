#pragma once
// ============================================================================
// vex/Core/DiagnosticConsumer.h
// Interface for receiving and displaying diagnostics.
// The default implementation renders to stderr with colour and source context.
// ============================================================================

#include "vex/Core/Diagnostic.h"
#include "vex/Core/SourceManager.h"
#include <cstdint>

namespace vex {

// ── DiagnosticConsumer ────────────────────────────────────────────────────────
// Abstract base. Implement to redirect diagnostics (e.g. to JSON, IDE protocol).

class DiagnosticConsumer {
public:
    virtual ~DiagnosticConsumer() = default;

    // Called for every diagnostic emitted.
    virtual void handleDiagnostic(const Diagnostic& diag) = 0;

    // Called when compilation finishes. Default: no-op.
    virtual void finish() {}
};

// ── TextDiagnosticConsumer ────────────────────────────────────────────────────
// Renders diagnostics to stderr in a human-readable text format.
// Format inspired by Rust/Clang — includes source context and underlines.

class TextDiagnosticConsumer final : public DiagnosticConsumer {
public:
    explicit TextDiagnosticConsumer(const SourceManager& srcMgr,
                                    bool                  useColor = true);

    void handleDiagnostic(const Diagnostic& diag) override;
    void finish() override;

    // Statistics
    uint32_t errorCount()   const { return errorCount_; }
    uint32_t warningCount() const { return warningCount_; }
    bool     hasErrors()    const { return errorCount_ > 0; }

private:
    const SourceManager& srcMgr_;
    bool                 useColor_;
    uint32_t             errorCount_   = 0;
    uint32_t             warningCount_ = 0;

    void renderLocation(const SourceLocation& loc) const;
    void renderSourceContext(const Diagnostic& diag) const;
    void renderSeverity(DiagSeverity sev) const;
    void renderNote(const DiagnosticNote& note) const;

    const char* colorFor(DiagSeverity sev) const;
    const char* colorReset() const;
};

// ── NullDiagnosticConsumer ────────────────────────────────────────────────────
// Silently discards all diagnostics. Used in testing when you only want counts.

class NullDiagnosticConsumer final : public DiagnosticConsumer {
public:
    void handleDiagnostic(const Diagnostic&) override {}
};

// ── StoringDiagnosticConsumer ─────────────────────────────────────────────────
// Stores all diagnostics in a vector for later inspection. Used in tests.

class StoringDiagnosticConsumer final : public DiagnosticConsumer {
public:
    void handleDiagnostic(const Diagnostic& diag) override {
        stored_.push_back(diag);
    }

    const std::vector<Diagnostic>& diagnostics() const { return stored_; }

    uint32_t errorCount() const {
        uint32_t n = 0;
        for (auto& d : stored_)
            if (d.isError()) ++n;
        return n;
    }

    bool hasErrors() const { return errorCount() > 0; }
    void clear() { stored_.clear(); }

private:
    std::vector<Diagnostic> stored_;
};

} // namespace vex