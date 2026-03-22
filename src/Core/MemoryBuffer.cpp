// ============================================================================
// vex/Core/MemoryBuffer.cpp
// ============================================================================

#include "vex/Core/MemoryBuffer.h"
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>

namespace vex {

// ── MemoryBuffer factory ─────────────────────────────────────────────────────

std::unique_ptr<MemoryBuffer> MemoryBuffer::fromFile(std::string_view path) {
    std::ifstream file(std::string(path), std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return nullptr;

    std::streamsize size = file.tellg();
    if (size < 0)
        return nullptr;

    file.seekg(0, std::ios::beg);

    std::string content(static_cast<size_t>(size), '\0');
    if (size > 0 && !file.read(content.data(), size))
        return nullptr;

    return HeapBuffer::create(std::move(content), std::string(path));
}

std::unique_ptr<MemoryBuffer> MemoryBuffer::fromString(
    std::string_view content,
    std::string_view name)
{
    return HeapBuffer::create(std::string(content), std::string(name));
}

std::unique_ptr<MemoryBuffer> MemoryBuffer::fromString(
    std::string&&    content,
    std::string_view name)
{
    return HeapBuffer::create(std::move(content), std::string(name));
}

// ── HeapBuffer ────────────────────────────────────────────────────────────────

HeapBuffer::HeapBuffer(std::string content, std::string name)
    : MemoryBuffer(nullptr, 0, std::move(name))
    , storage_(std::move(content))
{
    data_ = storage_.data();
    size_ = storage_.size();
}

std::unique_ptr<HeapBuffer> HeapBuffer::create(
    std::string content,
    std::string name)
{
    return std::unique_ptr<HeapBuffer>(
        new HeapBuffer(std::move(content), std::move(name)));
}

} // namespace vex