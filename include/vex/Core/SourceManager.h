#pragma once
// ============================================================================
// vex/Core/SourceManager.h
// Owns all source files loaded during a compilation.
// Maps FileID ↔ MemoryBuffer and SourceLocation ↔ line/column.
// ============================================================================

#include "vex/Core/MemoryBuffer.h"
#include "vex/Core/SourceLocation.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace vex {

// ── FileEntry ─────────────────────────────────────────────────────────────────
// Internal record for one loaded source file.

struct FileEntry {
    FileID                        id;
    std::string                   path;       // absolute path
    std::unique_ptr<MemoryBuffer> buffer;

    // Lazily computed line-start offsets for O(log n) line lookup.
    mutable std::vector<uint32_t> lineOffsets; // lineOffsets[i] = byte offset of line i+1
    mutable bool                  lineOffsetsBuilt = false;

    void buildLineOffsets() const;
};

// ── SourceManager ─────────────────────────────────────────────────────────────

class SourceManager {
public:
    SourceManager() = default;
    ~SourceManager() = default;

    // Non-copyable, moveable
    SourceManager(const SourceManager&) = delete;
    SourceManager& operator=(const SourceManager&) = delete;

    // ── File loading ─────────────────────────────────────────────────────────

    // Load a file from disk. Returns invalid FileID on failure.
    FileID loadFile(std::string_view path);

    // Register an in-memory buffer (for tests, REPL, generated code).
    FileID addBuffer(
        std::unique_ptr<MemoryBuffer> buffer,
        std::string_view              virtualPath = "<inline>"
    );

    // ── File access ──────────────────────────────────────────────────────────

    // Returns nullptr if id is invalid or not loaded.
    const MemoryBuffer* getBuffer(FileID id) const;

    // File path for a given FileID.
    std::string_view getPath(FileID id) const;

    // ── SourceLocation construction ───────────────────────────────────────────

    // Build a SourceLocation from a raw byte pointer inside a buffer.
    // The pointer must be within the buffer for the given FileID.
    SourceLocation getLocation(FileID id, const char* ptr) const;

    // Build a SourceLocation from a byte offset.
    SourceLocation getLocation(FileID id, uint32_t byteOffset) const;

    // ── SourceLocation queries ────────────────────────────────────────────────

    // Get the source line text for a given location (without trailing newline).
    std::string_view getLineText(SourceLocation loc) const;

    // Get the file path for a location.
    std::string_view getFilePath(SourceLocation loc) const;

    // Number of files loaded so far.
    size_t fileCount() const { return files_.size(); }

private:
    std::vector<std::unique_ptr<FileEntry>>  files_;
    std::unordered_map<std::string, FileID>  pathToID_;
    uint32_t                                 nextID_ = 1;

    FileEntry* getEntry(FileID id) const;
    FileEntry* getOrCreateEntry(std::string path);
};

} // namespace vex