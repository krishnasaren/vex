// src/Core/DiagnosticConsumer.cpp
#include "vex/Core/DiagnosticConsumer.h"
#include <cstdio>
#include <algorithm>
#include <string>

namespace vex {

// ── ANSI color helpers ────────────────────────────────────────────────────────
static const char* RESET  = "\033[0m";
static const char* BOLD   = "\033[1m";
static const char* RED    = "\033[1;31m";
static const char* YELLOW = "\033[1;33m";
static const char* CYAN   = "\033[1;36m";
static const char* WHITE  = "\033[1;37m";
static const char* BLUE   = "\033[1;34m";

TextDiagnosticConsumer::TextDiagnosticConsumer(const SourceManager& srcMgr,
                                               bool useColor)
    : srcMgr_(&srcMgr), useColor_(useColor) {}

const char* TextDiagnosticConsumer::colorFor(DiagSeverity sev) const {
    if (!useColor_) return "";
    switch (sev) {
    case DiagSeverity::Error:   return RED;
    case DiagSeverity::Fatal:   return RED;
    case DiagSeverity::Warning: return YELLOW;
    case DiagSeverity::Note:    return CYAN;
    }
    return "";
}

const char* TextDiagnosticConsumer::colorReset() const {
    return useColor_ ? RESET : "";
}

void TextDiagnosticConsumer::renderSeverity(DiagSeverity sev) const {
    const char* col  = colorFor(sev);
    const char* name = nullptr;
    switch (sev) {
    case DiagSeverity::Error:   name = "error";   break;
    case DiagSeverity::Fatal:   name = "error";   break;
    case DiagSeverity::Warning: name = "warning"; break;
    case DiagSeverity::Note:    name = "note";    break;
    }
    if (useColor_)
        fprintf(stderr, "%s%s%s%s%s",
                BOLD, col, name, colorReset(), BOLD);
    else
        fprintf(stderr, "%s", name);
}

void TextDiagnosticConsumer::renderLocation(const SourceLocation& loc) const {
    if (!loc.isValid()) return;
    std::string_view path = srcMgr_->getFilePath(loc);
    if (useColor_) fprintf(stderr, "%s", WHITE);
    if (!path.empty())
        fprintf(stderr, "%.*s:%u:%u",
                (int)path.size(), path.data(),
                loc.line(), loc.col());
    if (useColor_) fprintf(stderr, "%s", RESET);
}

void TextDiagnosticConsumer::renderSourceContext(const Diagnostic& diag) const {
    SourceLocation loc = diag.location();
    if (!loc.isValid()) return;

    std::string_view line = srcMgr_->getLineText(loc);
    if (line.empty()) return;

    uint32_t col = loc.col();

    // Print the source line with line number gutter
    fprintf(stderr, " %s%4u%s | %.*s\n",
            useColor_ ? BLUE : "", loc.line(), colorReset(),
            (int)line.size(), line.data());

    // Print the caret underline
    fprintf(stderr, "      | ");
    // Space to column
    for (uint32_t i = 1; i < col && i <= (uint32_t)line.size(); ++i)
        fputc(' ', stderr);

    if (useColor_) fprintf(stderr, "%s", RED);
    fputc('^', stderr);
    if (useColor_) fprintf(stderr, "%s", RESET);
    fputc('\n', stderr);
}

void TextDiagnosticConsumer::renderNote(const DiagnosticNote& note) const {
    if (useColor_) fprintf(stderr, "%s", CYAN);
    fprintf(stderr, "note");
    if (useColor_) fprintf(stderr, "%s", RESET);
    fprintf(stderr, ": %s", note.text.c_str());
    if (note.location.isValid()) {
        fprintf(stderr, "\n  --> ");
        renderLocation(note.location);
    }
    fputc('\n', stderr);
}

void TextDiagnosticConsumer::handleDiagnostic(const Diagnostic& diag) {
    // "file:line:col: severity: message"
    if (diag.location().isValid()) {
        renderLocation(diag.location());
        fprintf(stderr, ": ");
    }

    renderSeverity(diag.severity());
    if (useColor_) fprintf(stderr, "%s", BOLD);
    fprintf(stderr, ": %s%s\n", diag.message().c_str(), colorReset());

    renderSourceContext(diag);

    for (const auto& note : diag.notes())
        renderNote(note);

    if (!diag.hint().empty()) {
        if (useColor_) fprintf(stderr, "%s", CYAN);
        fprintf(stderr, "hint");
        if (useColor_) fprintf(stderr, "%s", RESET);
        fprintf(stderr, ": %s\n", diag.hint().c_str());
    }
}

void TextDiagnosticConsumer::finish() {
    if (errorCount_ > 0 || warningCount_ > 0) {
        fprintf(stderr, "\n");
        if (errorCount_)
            fprintf(stderr, "%serror%s: %s%u error%s%s generated\n",
                    useColor_ ? RED : "", colorReset(),
                    useColor_ ? BOLD : "",
                    errorCount_, errorCount_ == 1 ? "" : "s",
                    colorReset());
        if (warningCount_)
            fprintf(stderr, "%swarning%s: %u warning%s generated\n",
                    useColor_ ? YELLOW : "", colorReset(),
                    warningCount_, warningCount_ == 1 ? "" : "s");
    }
}

} // namespace vex
