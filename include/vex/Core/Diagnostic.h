#pragma once
// ============================================================================
// vex/Core/Diagnostic.h
// A single diagnostic message produced by the compiler.
// Contains: ID, severity, location, message text, notes, hints.
// ============================================================================

#include "vex/Core/DiagnosticIDs.h"
#include "vex/Core/SourceLocation.h"

#include <string>
#include <vector>

namespace vex {

// ── DiagnosticLabel ──────────────────────────────────────────────────────────
// A source range + label text used to annotate a specific span in the source.
// Rendered as an underline with an arrow message.

struct DiagnosticLabel {
    SourceRange range;
    std::string text;      // e.g. "expected int, found str"
    bool        isPrimary; // true = main underline; false = secondary

    DiagnosticLabel(SourceRange r, std::string t, bool primary = false)
        : range(r), text(std::move(t)), isPrimary(primary) {}
};

// ── DiagnosticNote ───────────────────────────────────────────────────────────
// An attached note — displayed after the main message.
// Can have its own location (e.g. "defined here") or be location-less.

struct DiagnosticNote {
    std::string    text;
    SourceLocation location; // may be invalid if note has no location

    DiagnosticNote(std::string t, SourceLocation loc = SourceLocation::invalid())
        : text(std::move(t)), location(loc) {}
};

// ── Diagnostic ───────────────────────────────────────────────────────────────

class Diagnostic {
public:
    Diagnostic(DiagID id, SourceLocation loc, std::string message)
        : id_(id)
        , severity_(defaultSeverity(id))
        , location_(loc)
        , message_(std::move(message))
    {}

    // ── Accessors ────────────────────────────────────────────────────────────
    DiagID         id()       const { return id_; }
    DiagSeverity   severity() const { return severity_; }
    SourceLocation location() const { return location_; }
    const std::string& message() const { return message_; }

    const std::vector<DiagnosticLabel>& labels() const { return labels_; }
    const std::vector<DiagnosticNote>&  notes()  const { return notes_; }
    const std::string& hint() const { return hint_; }

    bool isError()   const { return severity_ >= DiagSeverity::Error; }
    bool isFatal()   const { return severity_ == DiagSeverity::Fatal; }
    bool isWarning() const { return severity_ == DiagSeverity::Warning; }
    bool isNote()    const { return severity_ == DiagSeverity::Note; }

    // ── Builder methods ───────────────────────────────────────────────────────

    // Add a source range label (underlines a region of source code)
    Diagnostic& addLabel(SourceRange range, std::string text, bool primary = false) {
        labels_.emplace_back(range, std::move(text), primary);
        return *this;
    }

    // Primary label — the main span being pointed at
    Diagnostic& setPrimaryLabel(SourceRange range, std::string text = {}) {
        return addLabel(range, std::move(text), /*primary=*/true);
    }

    // Attach a note (with optional location)
    Diagnostic& addNote(std::string text,
                        SourceLocation loc = SourceLocation::invalid()) {
        notes_.emplace_back(std::move(text), loc);
        return *this;
    }

    // Suggest a fix in prose ("did you mean X?")
    Diagnostic& setHint(std::string hint) {
        hint_ = std::move(hint);
        return *this;
    }

    // Override severity (e.g. to downgrade an error to a warning)
    Diagnostic& setSeverity(DiagSeverity sev) {
        severity_ = sev;
        return *this;
    }

private:
    DiagID       id_;
    DiagSeverity severity_;
    SourceLocation           location_;
    std::string              message_;
    std::vector<DiagnosticLabel> labels_;
    std::vector<DiagnosticNote>  notes_;
    std::string              hint_;
};

} // namespace vex