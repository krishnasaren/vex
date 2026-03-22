#pragma once
// ============================================================================
// vex/Core/MemoryBuffer.h
// Read-only view of source file contents.
// Backed by memory-mapped file or heap-allocated string.
// ============================================================================

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace vex {

// ── MemoryBuffer ─────────────────────────────────────────────────────────────
// Immutable buffer holding the raw bytes of one source file.
// The Lexer reads directly from this buffer — zero-copy.

class MemoryBuffer {
public:
    virtual ~MemoryBuffer() = default;

    // The raw bytes of the file. Not null-terminated.
    const char* data() const { return data_; }

    // Byte count (not character count — UTF-8 aware code handles that).
    size_t size() const { return size_; }

    // Name of the file this buffer came from (for diagnostics).
    std::string_view name() const { return name_; }

    // Convenience: full content as string_view
    std::string_view content() const { return {data_, size_}; }

    // True if the buffer is empty
    bool empty() const { return size_ == 0; }

    // ── Factory methods ───────────────────────────────────────────────────────

    // Read a file from disk into a heap-allocated buffer.
    // Returns nullptr on failure (file not found, permission denied, etc.)
    static std::unique_ptr<MemoryBuffer> fromFile(std::string_view path);

    // Create a buffer from an existing string (copies the data).
    // Used for inline source in tests and REPL.
    static std::unique_ptr<MemoryBuffer> fromString(
        std::string_view content,
        std::string_view name = "<inline>"
    );

    // Create a buffer from an existing string (moves the data, no copy).
    static std::unique_ptr<MemoryBuffer> fromString(
        std::string&& content,
        std::string_view name = "<inline>"
    );

protected:
    MemoryBuffer(const char* data, size_t size, std::string name)
        : data_(data), size_(size), name_(std::move(name)) {}

    const char* data_;
    size_t      size_;
    std::string name_;
};

// ── HeapBuffer ───────────────────────────────────────────────────────────────
// MemoryBuffer backed by a heap-allocated std::string.

class HeapBuffer final : public MemoryBuffer {
public:
    static std::unique_ptr<HeapBuffer> create(
        std::string content,
        std::string name
    );

private:
    HeapBuffer(std::string content, std::string name);
    std::string storage_; // owns the data
};

} // namespace vex