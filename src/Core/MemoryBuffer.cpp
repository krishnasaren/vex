// src/Core/MemoryBuffer.cpp
#include "vex/Core/MemoryBuffer.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdexcept>

namespace vex {

// ── HeapBuffer ───────────────────────────────────────────────────────────────

HeapBuffer::HeapBuffer(std::string content, std::string name)
    : MemoryBuffer(nullptr, 0, std::move(name))
    , storage_(std::move(content))
{
    data_ = storage_.data();
    size_ = storage_.size();
}

std::unique_ptr<HeapBuffer> HeapBuffer::create(std::string content, std::string name) {
    return std::unique_ptr<HeapBuffer>(
        new HeapBuffer(std::move(content), std::move(name)));
}

// ── MemoryBuffer factory ─────────────────────────────────────────────────────

std::unique_ptr<MemoryBuffer> MemoryBuffer::fromFile(std::string_view path) {
    std::ifstream file(std::string(path), std::ios::binary);
    if (!file.is_open()) return nullptr;

    std::ostringstream ss;
    ss << file.rdbuf();
    if (file.fail()) return nullptr;

    return HeapBuffer::create(ss.str(), std::string(path));
}

std::unique_ptr<MemoryBuffer> MemoryBuffer::fromString(
    std::string_view content, std::string_view name)
{
    return HeapBuffer::create(std::string(content), std::string(name));
}

std::unique_ptr<MemoryBuffer> MemoryBuffer::fromString(
    std::string&& content, std::string_view name)
{
    return HeapBuffer::create(std::move(content), std::string(name));
}

} // namespace vex
