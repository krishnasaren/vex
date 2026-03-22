// ============================================================================
// vex/Core/DiagnosticConsumer.cpp
// Text diagnostic renderer — outputs to stderr with color and source context.
// ============================================================================

#include "vex/Core/DiagnosticConsumer.h"
#include "vex/Core/DiagnosticIDs.h"

#include <cstdio>
#include <algorithm>
#include <string>

namespace vex {

// ── ANSI color codes ──────────────────────────────────────────────────────────

namespace Color {
    constexpr const char* Reset   = "\033[0m";
    constexpr const char* Bold    = "\033[1m";
    constexpr const char* Red     = "\033[1;31m";
    constexpr const char* Yellow  = "\033[1;33m";
    constexpr const char* Cyan    = "\033[1;36m";
    constexpr const char* White   = "\033[1;37m";
    constexpr const char* Blue    = "\033[1;34m";
    constexpr const char* Green   = "\033[1;32m";
}

// ── TextDiagnosticConsumer ────────────────────────────────────────────────────

TextDiagnosticConsumer::TextDiagnosticConsumer(const SourceManager& srcMgr,
                                               bool useColor)
    : srcMgr_(srcMgr), useColor_(useColor) {}

const char* TextDiagnosticConsumer::colorFor(DiagSeverity sev) const {
    if (!useColor_) return "";
    switch (sev) {
        case DiagSeverity::Error:   return Color::Red;
        case DiagSeverity::Fatal:   return Color::Red;
        case DiagSeverity::Warning: return Color::Yellow;
        case DiagSeverity::Note:    return Color::Cyan;
    }
    return "";
}

const char* TextDiagnosticConsumer::colorReset() const {
    return useColor_ ? Color::Reset : "";
}

void TextDiagnosticConsumer::renderSeverity(DiagSeverity sev) const {
    const char* col  = colorFor(sev);
    const char* rst  = colorReset();
    const char* bold = useColor_ ? Color::Bold : "";

    switch (sev) {
        case DiagSeverity::Error:
        case DiagSeverity::Fatal:
            fprintf(stderr, "%s%serror%s", col, bold, rst);
            break;
        case DiagSeverity::Warning:
            fprintf(stderr, "%s%swarning%s", col, bold, rst);
            break;
        case DiagSeverity::Note:
            fprintf(stderr, "%s%snote%s", col, bold, rst);
            break;
    }
}

void TextDiagnosticConsumer::renderLocation(const SourceLocation& loc) const {
    if (!loc.isValid()) return;

    std::string_view path = srcMgr_.getFilePath(loc);
    if (useColor_) fprintf(stderr, "%s", Color::White);
    fprintf(stderr, "%.*s:%u:%u",
        (int)path.size(), path.data(),
        loc.line(), loc.col());
    if (useColor_) fprintf(stderr, "%s", Color::Reset);
}

void TextDiagnosticConsumer::renderSourceContext(const Diagnostic& diag) const {
    SourceLocation loc = diag.location();
    if (!loc.isValid()) return;

    std::string_view line = srcMgr_.getLineText(loc);
    if (line.empty()) return;

    uint32_t lineNo  = loc.line();
    uint32_t lineNoWidth = 1;
    for (uint32_t n = lineNo; n >= 10; n /= 10) ++lineNoWidth;

    // Print line number gutter + source line
    if (useColor_) fprintf(stderr, "%s", Color::Blue);
    fprintf(stderr, " %*u | ", lineNoWidth, lineNo);
    if (useColor_) fprintf(stderr, "%s", Color::Reset);
    fprintf(stderr, "%.*s\n", (int)line.size(), line.data());

    // Print caret underline at column
    if (useColor_) fprintf(stderr, "%s", Color::Blue);
    fprintf(stderr, " %*s | ", lineNoWidth, "");
    if (useColor_) fprintf(stderr, "%s", Color::Reset);

    uint32_t col = loc.col();
    for (uint32_t i = 1; i < col; ++i) fprintf(stderr, " ");

    // Check if there is a primary label for this location
    bool hasPrimary = false;
    for (auto& label : diag.labels()) {
        if (label.isPrimary) {
            hasPrimary = true;
            break;
        }
    }

    if (useColor_) fprintf(stderr, "%s", colorFor(diag.severity()));
    fprintf(stderr, "^");
    if (useColor_) fprintf(stderr, "%s", Color::Reset);
    fprintf(stderr, "\n");
}

void TextDiagnosticConsumer::renderNote(const DiagnosticNote& note) const {
    if (useColor_) fprintf(stderr, "%s", Color::Cyan);
    fprintf(stderr, "  = note: ");
    if (useColor_) fprintf(stderr, "%s", Color::Reset);
    fprintf(stderr, "%s\n", note.text.c_str());

    if (note.location.isValid()) {
        fprintf(stderr, "    --> ");
        renderLocation(note.location);
        fprintf(stderr, "\n");
    }
}

void TextDiagnosticConsumer::handleDiagnostic(const Diagnostic& diag) {
    // Count
    if (diag.isError() || diag.isFatal()) ++errorCount_;
    else if (diag.isWarning())            ++warningCount_;

    // Header: severity[bold] + message
    //   error: cannot borrow `x` as mutable — already borrowed
    renderSeverity(diag.severity());

    if (useColor_) fprintf(stderr, "%s", Color::White);
    fprintf(stderr, ": %s\n", diag.message().c_str());
    if (useColor_) fprintf(stderr, "%s", Color::Reset);

    // Location: " --> file.vex:10:5"
    if (diag.location().isValid()) {
        fprintf(stderr, "  --> ");
        renderLocation(diag.location());
        fprintf(stderr, "\n");
    }

    // Source context with caret
    renderSourceContext(diag);

    // Labels (non-primary)
    for (auto& label : diag.labels()) {
        if (!label.isPrimary && !label.text.empty()) {
            fprintf(stderr, "      | ");
            if (useColor_) fprintf(stderr, "%s", Color::Cyan);
            fprintf(stderr, "  %s\n", label.text.c_str());
            if (useColor_) fprintf(stderr, "%s", Color::Reset);
        }
    }

    // Hint
    if (!diag.hint().empty()) {
        if (useColor_) fprintf(stderr, "%s", Color::Green);
        fprintf(stderr, "  = help: ");
        if (useColor_) fprintf(stderr, "%s", Color::Reset);
        fprintf(stderr, "%s\n", diag.hint().c_str());
    }

    // Notes
    for (auto& note : diag.notes())
        renderNote(note);

    fprintf(stderr, "\n");
    fflush(stderr);
}

void TextDiagnosticConsumer::finish() {
    if (errorCount_ > 0 || warningCount_ > 0) {
        fprintf(stderr, "Compilation ");
        if (errorCount_ > 0) {
            if (useColor_) fprintf(stderr, "%s", Color::Red);
            fprintf(stderr, "failed");
            if (useColor_) fprintf(stderr, "%s", Color::Reset);
            fprintf(stderr, " with %u error%s",
                errorCount_, errorCount_ == 1 ? "" : "s");
            if (warningCount_ > 0)
                fprintf(stderr, " and %u warning%s",
                    warningCount_, warningCount_ == 1 ? "" : "s");
        } else {
            if (useColor_) fprintf(stderr, "%s", Color::Yellow);
            fprintf(stderr, "succeeded");
            if (useColor_) fprintf(stderr, "%s", Color::Reset);
            fprintf(stderr, " with %u warning%s",
                warningCount_, warningCount_ == 1 ? "" : "s");
        }
        fprintf(stderr, "\n");
        fflush(stderr);
    }
}

} // namespace vex