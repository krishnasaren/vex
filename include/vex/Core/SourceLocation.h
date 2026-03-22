#pragma once
// ============================================================================
// vex/Core/SourceLocation.h
// Tracks file + line + column for every token and AST node.
// Used in all diagnostic messages.
// ============================================================================

#include <cstdint>
#include <string_view>

namespace vex {

// ── FileID ───────────────────────────────────────────────────────────────────
// Opaque identifier for a source file managed by SourceManager.
// 0 = invalid/unknown.

struct FileID {
    uint32_t id = 0;

    bool isValid() const { return id != 0; }
    bool operator==(FileID other) const { return id == other.id; }
    bool operator!=(FileID other) const { return id != other.id; }

    static FileID invalid() { return FileID{0}; }
};

// ── SourceLocation ────────────────────────────────────────────────────────────
// A compact (8-byte) location: file + line + column.
// Line and column are 1-based. 0 means unknown.

class SourceLocation {
public:
    SourceLocation() = default;

    SourceLocation(FileID file, uint32_t line, uint32_t col)
        : fileID_(file), line_(line), col_(col) {}

    FileID   fileID() const { return fileID_; }
    uint32_t line()   const { return line_; }
    uint32_t col()    const { return col_; }

    bool isValid() const {
        return fileID_.isValid() && line_ > 0;
    }

    bool operator==(const SourceLocation& o) const {
        return fileID_ == o.fileID_ && line_ == o.line_ && col_ == o.col_;
    }
    bool operator!=(const SourceLocation& o) const {
        return !(*this == o);
    }

    static SourceLocation invalid() { return SourceLocation{}; }

private:
    FileID   fileID_;
    uint32_t line_ = 0;
    uint32_t col_  = 0;
};

// ── SourceRange ───────────────────────────────────────────────────────────────
// A half-open range [begin, end) in source.
// Used for underline highlighting in diagnostics.

class SourceRange {
public:
    SourceRange() = default;
    SourceRange(SourceLocation begin, SourceLocation end)
        : begin_(begin), end_(end) {}

    // Single-location range (just one character/token)
    explicit SourceRange(SourceLocation loc)
        : begin_(loc), end_(loc) {}

    SourceLocation begin() const { return begin_; }
    SourceLocation end()   const { return end_; }

    bool isValid() const { return begin_.isValid(); }

    static SourceRange invalid() { return SourceRange{}; }

private:
    SourceLocation begin_;
    SourceLocation end_;
};

} // namespace vex